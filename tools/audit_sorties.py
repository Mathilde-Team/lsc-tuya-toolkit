#!/usr/bin/env python3
"""Analyse une capture (pcap) des connexions sortantes d'une caméra — issue #1.

Usage : audit_sorties.py capture.pcap [--ip 192.0.2.10] [--masque]

Produit un rapport markdown : noms DNS demandés et adresses IP contactées
(avec ports), hors réseau local (RFC 1918 et lien-local). \`--ip\` ne retient
que les paquets émis par cette adresse (la caméra) ; sans \`--ip\`, tout le
fichier est analysé. \`--masque\` remplace le dernier octet de chaque IP par
« x » (le rapport est destiné à \`docs/SECURITY.md\`, sans IP réelle).

Format d'entrée : pcap classique (tcpdump/wireshark par défaut), Ethernet,
IPv4/IPv6, UDP/TCP. La capture se fait côté routeur ou PC (miroir de port) :

    tcpdump -i eth0 -w camera.pcap host <ip_caméra>          # >= 24 h

Aucune dépendance hors la bibliothèque standard.
"""
import argparse
import ipaddress
import socket
import struct
import sys

# ---------- pcap (format classique) ----------
MAGICS = {
    b"\xd4\xc3\xb2\xa1": "<",   # little-endian, µs
    b"\xa1\xb2\xc3\xd4": ">",   # big-endian, µs
    b"\x4d\x3c\xb2\xa1": "<",   # little-endian, ns
    b"\xa1\xb2\x3c\x4d": ">",   # big-endian, ns
}


def iter_paquets(donnees):
    """Rend (horodatage, octets du paquet) pour chaque enregistrement pcap."""
    if len(donnees) < 24 or donnees[:4] not in MAGICS:
        raise SystemExit("pas un pcap classique (attendu : sortie tcpdump/wireshark "
                         "par défaut ; le format pcapng n'est pas géré)")
    boutisme = MAGICS[donnees[:4]]
    i = 24
    while i + 16 <= len(donnees):
        _, _, taille, taille_reelle = struct.unpack_from(boutisme + "IIII", donnees, i)
        i += 16
        if taille_reelle == 0 or taille_reelle > 262144:  # tronqué/corrompu
            break
        paquet = donnees[i:i + taille_reelle]
        i += taille_reelle
        yield paquet


# ---------- trames ----------
def ip_de_trame(paquet):
    """Rend (src, dst, proto, payload) de la couche réseau, ou None."""
    if len(paquet) < 14:
        return None
    ethertype = struct.unpack_from(">H", paquet, 12)[0]
    depart = 14
    while ethertype == 0x8100 and len(paquet) >= depart + 4:  # VLAN 802.1Q
        ethertype = struct.unpack_from(">H", paquet, depart + 2)[0]
        depart += 4
    if ethertype == 0x0800:  # IPv4
        if len(paquet) < depart + 20:
            return None
        ihl = (paquet[depart] & 0x0F) * 4
        proto = paquet[depart + 9]
        src = socket.inet_ntop(socket.AF_INET, paquet[depart + 12:depart + 16])
        dst = socket.inet_ntop(socket.AF_INET, paquet[depart + 16:depart + 20])
        return src, dst, proto, paquet[depart + ihl:]
    if ethertype == 0x86DD:  # IPv6
        if len(paquet) < depart + 40:
            return None
        proto = paquet[depart + 6]
        src = socket.inet_ntop(socket.AF_INET6, paquet[depart + 8:depart + 24])
        dst = socket.inet_ntop(socket.AF_INET6, paquet[depart + 24:depart + 40])
        return src, dst, proto, paquet[depart + 40:]
    return None


def ports_udp(payload):
    if len(payload) < 4:
        return None
    src, dst = struct.unpack_from(">HH", payload, 0)
    return src, dst


def noms_dns(payload):
    """Noms interrogés dans un message DNS (questions, en-tête 12 octets)."""
    noms = []
    if len(payload) < 14:
        return noms
    qdcount = struct.unpack_from(">H", payload, 4)[0]
    i = 12
    for _ in range(min(qdcount, 16)):
        bouts = []
        while i < len(payload):
            n = payload[i]
            i += 1
            if n == 0:
                break
            if n & 0xC0:  # compression : inutilisé dans une question
                i += 1
                break
            bouts.append(payload[i:i + n].decode("ascii", "replace"))
            i += n
        if bouts:
            noms.append(".".join(bouts))
        i += 4  # type + classe
    return noms


# ---------- rapport ----------
def est_local(ip):
    a = ipaddress.ip_address(ip)
    return a.is_private or a.is_link_local or a.is_loopback or a.is_multicast


def masque_ip(ip):
    if ":" in ip:  # IPv6 : garde le préfixe /64
        return ":".join(ip.split(":")[:4]) + "::x"
    return ".".join(ip.split(".")[:3]) + ".x"


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("pcap", help="fichier pcap (tcpdump)")
    ap.add_argument("--ip", default="", help="n'analyser que les paquets émis par cette IP")
    ap.add_argument("--masque", action="store_true", help="masquer le dernier octet des IP du rapport")
    a = ap.parse_args()

    dns, ips, ports_par_ip = {}, {}, {}
    paquets_lus = 0
    try:
        with open(a.pcap, "rb") as f:
            donnees = f.read()
    except OSError as e:
        raise SystemExit(f"lecture impossible : {e}")

    for paquet in iter_paquets(donnees):
        paquets_lus += 1
        couche = ip_de_trame(paquet)
        if couche is None:
            continue
        src, dst, proto, payload = couche
        if a.ip and src != a.ip:
            continue
        if est_local(dst):
            continue
        if proto == 17:  # UDP
            ports = ports_udp(payload)
            if ports and ports[1] == 53:
                for nom in noms_dns(payload[8:]):
                    dns[nom] = dns.get(nom, 0) + 1
                continue
        if dst not in ips:
            ips[dst] = 0
            ports_par_ip[dst] = set()
        ips[dst] += 1
        if proto == 17 and len(payload) >= 4:
            ports_par_ip[dst].add(f"udp/{struct.unpack_from('>H', payload, 2)[0]}")
        elif proto == 6 and len(payload) >= 4:
            ports_par_ip[dst].add(f"tcp/{struct.unpack_from('>H', payload, 2)[0]}")

    if paquets_lus == 0:
        raise SystemExit("aucun paquet lu")

    fmt = masque_ip if a.masque else (lambda ip: ip)
    print(f"# Connexions sortantes — {paquets_lus} paquets analysés"
          + (f", émetteur {a.ip}" if a.ip else "") + "\n")
    print("## Noms DNS demandés\n")
    if dns:
        print("| Nom | Demandes |")
        print("|---|---|")
        for nom, n in sorted(dns.items(), key=lambda x: (-x[1], x[0])):
            print(f"| {nom} | {n} |")
    else:
        print("Aucune requête DNS (hors réseau local) dans la capture.")
    print("\n## Destinations contactées (hors réseau local)\n")
    if ips:
        print("| Destination | Paquets | Ports |")
        print("|---|---|---|")
        for ip, n in sorted(ips.items(), key=lambda x: (-x[1], x[0])):
            ports = ", ".join(sorted(ports_par_ip[ip])) or "—"
            print(f"| {fmt(ip)} | {n} | {ports} |")
    else:
        print("Aucune destination hors réseau local contactée.")


if __name__ == "__main__":
    main()
