#!/usr/bin/env python3
"""Patch de /usr/bin/anyka_ipc : sortie vidéo à N fps (défaut 20) au lieu de 15.

Le binaire du fabricant n'est pas distribué ici : ce script l'applique à VOTRE copie
(extraite de votre caméra, voir docs/CHANGES.md) et refuse de rien écrire si les octets
attendus ne sont pas exactement ceux de la version testée.

Trois modifications, toutes vérifiées avant écriture :
  1. ak_vi_set_chn_attr, canal principal : `mov r2,#15` -> `mov r2,#N`   (0xb54e0)
  2. ak_vi_set_chn_attr, sous-flux       : `mov r2,#15` -> `mov r2,#N`   (0xb55c0)
  3. app_video_vi_stream_do_venc : `bgt drop` -> `nop` (0xca9f4)
     Ce saut jette toute image arrivant moins de 66 ms (1000/15) après la précédente,
     ce qui ramenait la sortie à 15 fps même avec un capteur à 20 fps.

Adresses = adresses virtuelles ARM ; offset fichier = adresse - 0x10000.
Binaire testé : anyka_ipc md5 3d8cad8080cae1f8bd6bca7331e2c651
(caméra IPC-XC-C323, firmware 3.10.78.1269 ; le dépôt amont cible 6.2863.123, binaire différent).
"""
import argparse
import hashlib
import struct
import sys

BASE = 0x10000
EXPECTED_MD5 = "3d8cad8080cae1f8bd6bca7331e2c651"
MOV_R2_15 = bytes.fromhex("0f20a0e3")   # mov r2, #0xf
BGT_DROP = bytes.fromhex("7a0000ca")    # bgt #0xcabe4 (chemin « jeter l'image »)
NOP = struct.pack("<I", 0xE1A00000)     # mov r0, r0


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("src", help="anyka_ipc d'origine (extrait de la caméra)")
    ap.add_argument("dst", help="fichier patché à écrire (ex. sd_card/custom/bin/anyka_ipc_fps20b)")
    ap.add_argument("--fps", type=int, default=20, help="fps de sortie (défaut 20, seule valeur testée)")
    ap.add_argument("--keep-drop", action="store_true", help="ne pas neutraliser le filtre d'images (patchs 1 et 2 seulement)")
    ap.add_argument("--force", action="store_true", help="ignorer le contrôle MD5 (octets attendus toujours vérifiés)")
    a = ap.parse_args()

    if not 1 <= a.fps <= 255:
        sys.exit("--fps doit être entre 1 et 255")
    data = bytearray(open(a.src, "rb").read())
    md5 = hashlib.md5(data).hexdigest()
    if md5 != EXPECTED_MD5 and not a.force:
        sys.exit(f"MD5 inattendu {md5} (attendu {EXPECTED_MD5}) : autre version du firmware ? --force pour tenter.")

    patches = [(0xB54E0, MOV_R2_15, bytes([a.fps]) + MOV_R2_15[1:]),
               (0xB55C0, MOV_R2_15, bytes([a.fps]) + MOV_R2_15[1:])]
    if not a.keep_drop:
        patches.append((0xCA9F4, BGT_DROP, NOP))

    for addr, old, _ in patches:
        off = addr - BASE
        if data[off:off + 4] != old:
            sys.exit(f"Octets inattendus à 0x{addr:x} : {data[off:off + 4].hex()} (attendu {old.hex()}). Rien écrit.")
    for addr, _, new in patches:
        off = addr - BASE
        data[off:off + 4] = new
        print(f"0x{addr:x}: -> {new.hex()}")

    open(a.dst, "wb").write(data)
    print(f"écrit {a.dst}  md5={hashlib.md5(data).hexdigest()}")


if __name__ == "__main__":
    main()
