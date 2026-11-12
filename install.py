#!/usr/bin/env python3
"""Installeur de la carte SD du toolkit (caméra Tuya/Anyka, modèle 3007325 / IPC-XC-C323).

Copie `sd_card/` sur la carte, puis la personnalise (Wi-Fi, IP fixe, heure, syslog, ONVIF...)
à partir d'un fichier YAML ou de questions interactives. Python 3 standard, sans dépendance
(PyYAML est utilisé s'il est installé, sinon un lecteur YAML minimal intégré).

  python3 install.py                              # assistant interactif
  python3 install.py --config install.yaml        # tout depuis le YAML (les manques sont demandés)
  python3 install.py --config install.yaml --yes  # sans aucune question (échoue si un champ manque)
  python3 install.py --target /dev/sdb1          # carte ou point de montage donné en ligne de commande
  python3 install.py --dry-run                    # montre ce qui serait fait, n'écrit rien
"""
import argparse
import getpass
import glob
import hashlib
import ipaddress
import json
import os
import re
import secrets
import shutil
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(ROOT, "sd_card")
TZ_PRESETS = {
    "1": ("Europe/Paris, Bruxelles, Amsterdam, Berlin", "CET-1CEST,M3.5.0,M10.5.0/3"),
    "2": ("Europe/Londres, Lisbonne", "GMT0BST,M3.5.0/1,M10.5.0"),
    "3": ("UTC", "UTC0"),
    "4": ("Autre (saisie d'une chaîne POSIX)", None),
}
DEFAULTS = {
    "target": "",
    "network": {"ssid": "", "password": "", "security": 3, "dhcp": False,
                "ip": "", "netmask": "255.255.255.0", "gateway": ""},
    "time": {"tz": "CET-1CEST,M3.5.0,M10.5.0/3", "ntp_server": ""},
    "syslog": {"server": ""},
    "camera": {"hostname": "", "ptz_home_on_boot": True, "mute_factory_prompt": True, "logs_keep": 20,
              "privacy_preset": 0},
    "onvif": {"enabled": True, "user": "admin", "password": "auto", "ptz": True},
    "services": {"telnet": "open", "ftp": False, "offline": True},
    "security": {"root_password": "auto"},
    "video": {"main_codec": 2, "main_width": 1920, "main_height": 1080,
              "sub_codec": 2, "sub_width": 640, "sub_height": 360},
    "speaker": {"enabled": False, "allow": "", "gain": 50, "port": 5004},
    "firmware": {"dump_dir": ""},
}
CODECS = {0: "H264", 1: "MJPEG", 2: "H265"}
CODEC_NAMES = {"h264": 0, "mjpeg": 1, "mjpg": 1, "jpeg": 1, "h265": 2, "hevc": 2}
ONVIF_ENC = {0: "H264", 1: "JPEG", 2: "H265"}
SIZES = {"1": (1920, 1080), "2": (1280, 720), "3": (640, 360), "4": (None, None)}


# ---------- YAML (PyYAML si présent, sinon sous-ensemble : sections + « clé: valeur ») ----------
def _scalar(v):
    v = v.strip()
    if v.startswith('"'):
        return json.JSONDecoder().raw_decode(v)[0]
    if v.startswith("'"):
        m = re.match(r"'((?:[^']|'')*)'", v)
        return m.group(1).replace("''", "'")
    v = re.sub(r"\s+#.*$", "", v)
    if v.lower() in ("true", "yes"):
        return True
    if v.lower() in ("false", "no"):
        return False
    if v in ("", "~", "null"):
        return ""
    if re.fullmatch(r"-?\d+", v):
        return int(v)
    return v


def parse_yaml(text):
    try:
        import yaml
        return yaml.safe_load(text) or {}
    except ImportError:
        pass
    data, section = {}, None
    for n, raw in enumerate(text.splitlines(), 1):
        if not raw.strip() or raw.lstrip().startswith("#"):
            continue
        m = re.match(r"^(\s*)([A-Za-z0-9_]+):\s*(.*)$", raw)
        if not m:
            sys.exit(f"YAML illisible ligne {n}: {raw!r}")
        indent, key, val = len(m.group(1)), m.group(2), m.group(3)
        if indent == 0:
            if val.strip() == "" or val.strip().startswith("#"):
                data[key] = section = {}
            else:
                data[key], section = _scalar(val), None
        elif section is not None:
            section[key] = _scalar(val)
        else:
            sys.exit(f"YAML: indentation inattendue ligne {n}")
    return data


def dump_yaml(cfg):
    out = ["# Configuration de l'installeur (contient des mots de passe : ne pas publier, chmod 600)."]
    for k, v in cfg.items():
        if isinstance(v, dict):
            out.append(f"{k}:")
            out += [f"  {kk}: {_fmt(vv)}" for kk, vv in v.items()]
        else:
            out.append(f"{k}: {_fmt(v)}")
    return "\n".join(out) + "\n"


def _fmt(v):
    if isinstance(v, bool):
        return "true" if v else "false"
    if isinstance(v, int):
        return str(v)
    return json.dumps(v, ensure_ascii=False)


def merge(base, over):
    res = {k: (dict(v) if isinstance(v, dict) else v) for k, v in base.items()}
    for k, v in (over or {}).items():
        if isinstance(v, dict) and isinstance(res.get(k), dict):
            res[k].update(v)
        else:
            res[k] = v
    return res


# ---------- questions ----------
def ask(label, default="", secret=False, required=False):
    suffix = f" [{default}]" if default not in ("", None) and not secret else ""
    while True:
        v = (getpass.getpass if secret else input)(f"{label}{suffix} : ").strip()
        v = v or (default if default is not None else "")
        if v or not required:
            return v
        print("  Valeur obligatoire.")


def ask_bool(label, default):
    d = "O/n" if default else "o/N"
    while True:
        v = input(f"{label} ({d}) : ").strip().lower()
        if not v:
            return default
        if v in ("o", "oui", "y", "yes"):
            return True
        if v in ("n", "non", "no"):
            return False


def wizard(cfg):
    n, t, c, o, s = cfg["network"], cfg["time"], cfg["camera"], cfg["onvif"], cfg["services"]
    print("\n== Réseau Wi-Fi (la caméra ne gère que le 2,4 GHz) ==")
    n["ssid"] = ask("Nom du Wi-Fi (SSID)", n["ssid"], required=True)
    n["security"] = int(ask("Sécurité : 1=ouvert 2=WEP 3=WPA/WPA2", n["security"]))
    if n["security"] != 1:
        n["password"] = ask("Mot de passe Wi-Fi", n["password"], secret=True, required=True) if not n["password"] else n["password"]
    print("\n== Adresse IP (une IP fixe hors de la plage DHCP du routeur est recommandée) ==")
    n["dhcp"] = ask_bool("Utiliser le DHCP (déconseillé : instable sur ce firmware)", n["dhcp"])
    if not n["dhcp"]:
        n["ip"] = ask("Adresse IP de la caméra", n["ip"], required=True)
        n["netmask"] = ask("Masque", n["netmask"], required=True)
        n["gateway"] = ask("Passerelle (routeur)", n["gateway"], required=True)
    c["hostname"] = ask("Nom d'hôte de la caméra (vide = inchangé)", c["hostname"])
    print("\n== Heure ==")
    for k, (lbl, _) in TZ_PRESETS.items():
        print(f"  {k}. {lbl}")
    ch = ask("Fuseau", "1")
    t["tz"] = TZ_PRESETS.get(ch, (None, None))[1] or ask("Chaîne POSIX du fuseau", t["tz"], required=True)
    if ask_bool("Synchroniser l'heure sur un serveur NTP du LAN (sinon l'heure dérive)", bool(t["ntp_server"])):
        t["ntp_server"] = ask("Adresse du serveur NTP", t["ntp_server"], required=True)
    else:
        t["ntp_server"] = ""
    print("\n== Journaux distants ==")
    if ask_bool("Envoyer le syslog vers un serveur (UDP)", bool(cfg["syslog"]["server"])):
        cfg["syslog"]["server"] = ask("Serveur syslog hôte:port", cfg["syslog"]["server"] or ":5514", required=True)
    else:
        cfg["syslog"]["server"] = ""
    print("\n== ONVIF / Home Assistant ==")
    o["enabled"] = ask_bool("Activer ONVIF", o["enabled"])
    if o["enabled"]:
        o["user"] = ask("Utilisateur ONVIF", o["user"])
        pw = ask("Mot de passe ONVIF ('auto' = généré ; 'aucun' = SANS authentification)", "auto", secret=True)
        o["password"] = "" if pw.lower() == "aucun" else pw
        o["ptz"] = ask_bool("Activer le pilotage PTZ via ONVIF", o["ptz"])
    print("\n== Services ==")
    s["telnet"] = ask("Telnet : open (sans mot de passe, pour régler) / login (root + mot de passe) / off", telnet_mode(s["telnet"]))
    rp = cfg["security"]
    rp["root_password"] = ask("Mot de passe root ('auto' = généré, vide = inchangé, déconseillé)", rp["root_password"] or "auto", secret=True) or ""
    s["ftp"] = ask_bool("Serveur FTP", s["ftp"])
    c["ptz_home_on_boot"] = ask_bool("Recentrer la caméra à chaque démarrage (positions absolues fiables)", c["ptz_home_on_boot"])
    c["logs_keep"] = int(ask("Journaux de démarrage conservés sur la SD (0 = tout garder)", c["logs_keep"]))
    c["privacy_preset"] = int(ask("Numéro du preset « position privée » (0 = désactivé ; voir docs/PRIVACY.md)", c["privacy_preset"]))
    v = cfg["video"]
    print("\n== Flux vidéo (codec : 0=H.264, 1=MJPEG, 2=H.265) ==")
    print("  Constaté ici : H.265 1920x1080 à 20 i/s est fluide ; H.264 ne donnait que ~11 i/s ; plus de 1920x1080 fait boucler la caméra.")
    for pre, lbl, sizes in (("main", "principal", "1. 1920x1080  2. 1280x720  3. 640x360  4. autre"),
                            ("sub", "secondaire", "1. 1920x1080  2. 1280x720  3. 640x360  4. autre")):
        v[pre + "_codec"] = ask(f"Codec du flux {lbl} (0/1/2)", v[pre + "_codec"])
        print(f"  Taille du flux {lbl} : {sizes}")
        dflt = "1" if pre == "main" else "3"
        ch = ask("  Choix", dflt)
        w, h = SIZES.get(ch, (None, None))
        if w is None:
            w = ask("  Largeur (px)", v[pre + "_width"], required=True)
            h = ask("  Hauteur (px)", v[pre + "_height"], required=True)
        v[pre + "_width"], v[pre + "_height"] = w, h
    sp = cfg["speaker"]
    print("\n== Haut-parleur par le LAN (sirène, annonces) ==")
    print("  Port UDP sans authentification : seule l'IP indiquée (ex. votre Home Assistant) pourra envoyer du son.")
    sp["enabled"] = ask_bool("Activer le haut-parleur par le LAN", sp["enabled"])
    if sp["enabled"]:
        sp["allow"] = ask("IP autorisées à envoyer du son (séparées par des virgules)", sp["allow"], required=True)
        sp["gain"] = ask("Volume (1 à 100)", sp["gain"])
    print("\n== Dump du firmware d'origine (pour la vidéo 20 images/s) ==")
    print("  Inutile avec le firmware 3.10.78 : le 20 i/s est fabriqué par la caméra au démarrage. Voir docs/INSTALL.md (étape 4).")
    cfg["firmware"]["dump_dir"] = ask("Dossier contenant anyka_ipc et l'ISP extraits de VOTRE caméra", cfg["firmware"]["dump_dir"])
    return cfg


# ---------- mot de passe root (hash MD5-crypt "$1$", format du fichier shadow du firmware) ----------
_B64 = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"


def _to64(v, n):
    out = ""
    for _ in range(n):
        out += _B64[v & 0x3F]
        v >>= 6
    return out


def md5crypt(password, salt=None):
    pw = password.encode()
    salt = (salt or "".join(secrets.choice(_B64) for _ in range(8)))[:8]
    s = salt.encode()
    alt = hashlib.md5(pw + s + pw).digest()
    ctx = pw + b"$1$" + s + (alt * (len(pw) // 16 + 1))[:len(pw)]
    i = len(pw)
    while i:
        ctx += b"\0" if i & 1 else pw[:1]
        i >>= 1
    h = hashlib.md5(ctx).digest()
    for i in range(1000):
        c = pw if i & 1 else h
        if i % 3:
            c += s
        if i % 7:
            c += pw
        c += h if i & 1 else pw
        h = hashlib.md5(c).digest()
    out = ""
    for a, b, c in ((0, 6, 12), (1, 7, 13), (2, 8, 14), (3, 9, 15), (4, 10, 5)):
        out += _to64((h[a] << 16) | (h[b] << 8) | h[c], 4)
    return f"$1${salt}$" + out + _to64(h[11], 2)


def telnet_mode(v):
    if v is True:
        return "open"
    if v is False:
        return "off"
    return str(v).lower()


# ---------- validation ----------
BAD = re.compile(r"[\r\n\x00]")
SHELL_BAD = re.compile(r'[\r\n\x00"`$\\]')


def validate(cfg):
    err, warn = [], []
    n, t, o, c = cfg["network"], cfg["time"], cfg["onvif"], cfg["camera"]
    for k in ("ssid", "password"):
        if BAD.search(str(n[k])):
            err.append(f"network.{k}: retour à la ligne interdit")
    if not n["ssid"]:
        err.append("network.ssid obligatoire")
    if re.search(r"CHANGE_?ME|<.*>", f"{n['ssid']}{n['password']}", re.I):
        err.append("network: valeurs d'exemple non remplacées")
    if n["security"] not in (1, 2, 3):
        err.append("network.security doit valoir 1, 2 ou 3")
    if n["security"] == 3 and not 8 <= len(n["password"]) <= 63:
        err.append("network.password: 8 à 63 caractères en WPA")
    if n["security"] == 1:
        n["password"] = ""
    if n["dhcp"]:
        warn.append("DHCP : constaté instable sur ce firmware (docs/WIFI_ET_IP.md §3), préférez l'IP fixe")
    else:
        try:
            net = ipaddress.ip_interface(f"{n['ip']}/{n['netmask']}")
            gw = ipaddress.ip_address(n["gateway"])
            if gw not in net.network:
                err.append("network.gateway n'est pas dans le même sous-réseau que l'IP")
            if net.ip in (net.network.network_address, net.network.broadcast_address) or net.ip == gw:
                err.append("network.ip invalide (réseau, diffusion ou passerelle)")
        except ValueError as e:
            err.append(f"network ip/netmask/gateway: {e}")
    if SHELL_BAD.search(t["tz"]) or not t["tz"]:
        err.append("time.tz invalide")
    if t["ntp_server"] and not re.fullmatch(r"[A-Za-z0-9.\-]+", str(t["ntp_server"])):
        err.append("time.ntp_server invalide")
    srv = cfg["syslog"]["server"]
    if srv and not re.fullmatch(r"[A-Za-z0-9.\-]+:\d{1,5}", srv):
        err.append("syslog.server doit être de la forme hote:port")
    if not isinstance(c["privacy_preset"], int) or c["privacy_preset"] < 0:
        err.append("camera.privacy_preset doit être un entier positif ou nul")
    if not isinstance(c["logs_keep"], int) or c["logs_keep"] < 0:
        err.append("camera.logs_keep doit être un entier positif ou nul")
    if c["hostname"] and not re.fullmatch(r"[A-Za-z0-9\-]{1,32}", c["hostname"]):
        err.append("camera.hostname: lettres, chiffres et tirets (32 max)")
    if o["enabled"]:
        if not re.fullmatch(r"[A-Za-z0-9_.\-]{1,32}", str(o["user"])):
            err.append("onvif.user invalide")
        pw = str(o["password"])
        if pw and pw != "auto" and (len(pw) < 8 or re.search(r"[\r\n\x00=]", pw)):
            err.append("onvif.password: 8 caractères minimum, sans '=' ni retour à la ligne")
        if not pw:
            warn.append("ONVIF sans authentification : toute machine du LAN peut piloter la caméra")
    v = cfg["video"]
    for pre in ("main", "sub"):
        c_ = v[pre + "_codec"]
        c_ = CODEC_NAMES.get(str(c_).lower(), c_) if not isinstance(c_, int) else c_
        try:
            c_ = int(c_)
        except (TypeError, ValueError):
            c_ = None
        if c_ not in CODECS:
            err.append(f"video.{pre}_codec: 0 (h264), 1 (mjpeg) ou 2 (h265)")
            continue
        v[pre + "_codec"] = c_
        try:
            w, h = int(v[pre + "_width"]), int(v[pre + "_height"])
        except (TypeError, ValueError):
            err.append(f"video.{pre}_width/height doivent être des entiers")
            continue
        v[pre + "_width"], v[pre + "_height"] = w, h
        if not (160 <= w <= 3840 and 120 <= h <= 2160) or w % 8 or h % 8:
            err.append(f"video.{pre}: taille {w}x{h} invalide (multiples de 8, 160x120 à 3840x2160)")
        if c_ == 1:
            warn.append(f"video.{pre}_codec = MJPEG : non testé sur ce firmware")
        if c_ == 0 and pre == "main":
            warn.append("H.264 en flux principal : ~11 images/s mesurées sur ce modèle (H.265 : 20)")
    if not err:
        if v["main_width"] * v["main_height"] > 1920 * 1080:
            warn.append("flux principal au-dessus de 1920x1080 : une caméra a bouclé au redémarrage à 2304x1296")
        if v["sub_width"] * v["sub_height"] > v["main_width"] * v["main_height"]:
            err.append("video: le flux secondaire ne peut pas être plus grand que le principal")
    sp = cfg["speaker"]
    if sp["enabled"]:
        ips = [x for x in re.split(r"[,\s]+", str(sp["allow"])) if x]
        try:
            if not ips or len(ips) > 8:
                raise ValueError
            for x in ips:
                ipaddress.IPv4Address(x)
        except ValueError:
            err.append("speaker.allow: 1 à 8 adresses IPv4 séparées par des virgules (seules ces machines pourront envoyer du son)")
        sp["allow"] = ",".join(ips)
        try:
            sp["gain"], sp["port"] = int(sp["gain"]), int(sp["port"])
        except (TypeError, ValueError):
            err.append("speaker.gain et speaker.port doivent être des entiers")
        else:
            if not 1 <= sp["gain"] <= 100:
                err.append("speaker.gain: 1 à 100")
            if not 1024 <= sp["port"] <= 65535:
                err.append("speaker.port: 1024 à 65535")
    tm = telnet_mode(cfg["services"]["telnet"])
    cfg["services"]["telnet"] = tm
    if tm not in ("open", "login", "off"):
        err.append("services.telnet: open (sans mot de passe), login (root + mot de passe) ou off")
    rp = str(cfg["security"]["root_password"])
    if rp and rp != "auto" and (len(rp) < 8 or re.search(r"[\r\n\x00:]", rp)):
        err.append("security.root_password: 8 caractères minimum, sans ':' ni retour à la ligne")
    if tm == "login" and not rp:
        err.append("services.telnet=login exige security.root_password (sinon le mot de passe est celui, public, du dépôt)")
    if tm == "open":
        warn.append("Telnet 'open' : shell root SANS mot de passe pour tout le LAN ; passez à 'login' puis 'off' une fois la caméra réglée")
    if not rp:
        warn.append("security.root_password vide : le mot de passe root du port 23 reste celui, public, du dépôt")
    return err, warn


# ---------- cible : périphérique ou dossier ----------
def run(*a, check=True):
    return subprocess.run(a, capture_output=True, text=True, check=check)


def lsblk():
    r = run("lsblk", "-J", "-o", "NAME,PATH,SIZE,RM,TRAN,FSTYPE,LABEL,MOUNTPOINT,MODEL,TYPE", check=False)
    try:
        return json.loads(r.stdout)["blockdevices"]
    except (ValueError, KeyError):
        return []


def flat(devs):
    for d in devs:
        yield d
        yield from flat(d.get("children", []))


def pick_target(arg, yes):
    if arg and os.path.isdir(arg):
        return arg, None
    parts = [d for d in flat(lsblk()) if d.get("type") == "part" and d.get("fstype") in ("vfat", "exfat")]
    removable = [d for d in parts if str(d.get("rm")) in ("1", "True", "true") or d.get("tran") == "usb"]
    if not arg:
        if not removable:
            sys.exit("Aucune carte SD / clé USB FAT détectée. Donnez --target /dev/sdX1 ou un point de montage.")
        print("\nCartes détectées (FAT, amovibles) :")
        for i, d in enumerate(removable, 1):
            print(f"  {i}. {d['path']}  {d['size']}  {d.get('label') or '-'}  {d.get('model') or ''}  monté sur {d.get('mountpoint') or '-'}")
        k = ask("Numéro de la carte ou chemin (/dev/sdX1 ou point de montage)", "1")
        arg = removable[int(k) - 1]["path"] if k.isdigit() and 1 <= int(k) <= len(removable) else k
        if os.path.isdir(arg):
            return arg, None
    dev = next((d for d in parts if d["path"] == arg), None)
    if dev is None:
        sys.exit(f"{arg}: pas une partition FAT connue (lsblk). Le firmware lit la carte en FAT32.")
    if dev not in removable:
        sys.exit(f"{arg}: n'est pas amovible, refus (protection du disque système).")
    if dev.get("mountpoint"):
        return dev["mountpoint"], None
    tmp = tempfile.mkdtemp(prefix="lsc-sd-")
    r = run("mount", "-o", "uid=%d,gid=%d" % (os.getuid(), os.getgid()), arg, tmp, check=False)
    if r.returncode != 0:
        os.rmdir(tmp)
        sys.exit(f"mount {arg} a échoué ({r.stderr.strip()}). Relancez avec sudo ou montez la carte vous-même.")
    return tmp, tmp


# ---------- rendu des fichiers ----------
def sh_quote(v):
    return "'" + str(v).replace("'", "'\\''") + "'"


def set_kv(text, key, value, sep="="):
    line = f"{key}{sep}{value}"
    pat = re.compile(rf"^[# \t]*{re.escape(key)}[ \t]*{re.escape(sep.strip())}.*$", re.M)
    return pat.sub(lambda _: line, text, count=1) if pat.search(text) else text.rstrip("\n") + "\n" + line + "\n"


def rw(path, fn):
    with open(path, encoding="utf-8", errors="surrogateescape") as f:
        t = f.read()
    with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="\n") as f:
        f.write(fn(t))


def render(dst, cfg):
    n, t, c, o, s = cfg["network"], cfg["time"], cfg["camera"], cfg["onvif"], cfg["services"]
    conf = os.path.join(dst, "custom", "configs")
    ip, mask, gw = ("", "", "") if n["dhcp"] else (n["ip"], n["netmask"], n["gateway"])

    def factory(txt):
        for k, v in (("ssid", n["ssid"]), ("passphrase", n["password"]), ("ip", ip), ("netmask", mask), ("gateway", gw)):
            txt = set_kv(txt, k, v)
        return txt
    rw(os.path.join(dst, "_ak39_factory.ini"), factory)

    rp = cfg["security"]["root_password"]
    if rp:
        with open(os.path.join(dst, "shadow"), "w", newline="\n") as f:
            f.write(f"root:{md5crypt(rp)}:18943:0:99999:7:::\n")

    with open(os.path.join(conf, "wifi.conf"), "w", newline="\n") as f:
        f.write("# Genere par install.py. Lu par wifi_apply.sh (shell : valeurs entre apostrophes).\n"
                f"MODE=sta\nSSID={sh_quote(n['ssid'])}\nPASS={sh_quote(n['password'])}\n"
                f"SECURITY={n['security']}\nDHCP={1 if n['dhcp'] else 0}\n")

    def b(v):
        return "1" if v else "0"
    tm = telnet_mode(s["telnet"])

    def hack(txt):
        for k, v in (("TELNET", "0" if tm == "off" else "1"), ("TELNET_LOGIN", "1" if tm == "login" else "0"), ("FTP", b(s["ftp"])), ("OFFLINE", b(s["offline"])),
                     ("ONVIF", b(o["enabled"])), ("ONVIF_PTZ", b(o["ptz"])),
                     ("MUTE_FACTORY_PROMPT", b(c["mute_factory_prompt"])),
                     ("PTZ_HOME_ON_BOOT", b(c["ptz_home_on_boot"])),
                     ("LOGS_KEEP", str(c["logs_keep"])),
                     ("PRIVACY_PRESET", str(c["privacy_preset"])),
                     ("TZ_STRING", f'"{t["tz"]}"'), ("NTP_SERVER", f'"{t["ntp_server"]}"'),
                     ("SYSLOG_SERVER", f'"{cfg["syslog"]["server"]}"'), ("CAM_HOSTNAME", f'"{c["hostname"]}"'),
                     ("TALK", b(cfg["speaker"]["enabled"])), ("TALK_ALLOW", f'"{cfg["speaker"]["allow"] if cfg["speaker"]["enabled"] else ""}"'),
                     ("TALK_GAIN", cfg["speaker"]["gain"]), ("TALK_PORT", cfg["speaker"]["port"])):
            txt = set_kv(txt, k, v)
        return txt
    rw(os.path.join(conf, "hack.conf"), hack)

    v = cfg["video"]

    def hw(txt):
        for k, val in (("main_width", v["main_width"]), ("main_height", v["main_height"]),
                       ("main_enc_out_type", v["main_codec"]),
                       ("sub_width", v["sub_width"]), ("sub_height", v["sub_height"]),
                       ("sub_enc_out_type", v["sub_codec"])):
            txt = set_kv(txt, k, val, " = ")
        return txt
    rw(os.path.join(conf, "_ht_hw_settings.ini"), hw)

    def onvif(txt):
        txt = set_kv(txt, "width", v["main_width"])
        txt = set_kv(txt, "height", v["main_height"])
        txt = set_kv(txt, "type", ONVIF_ENC[v["main_codec"]])
        if o["enabled"] and o["password"]:
            txt = set_kv(txt, "user", o["user"])
            txt = set_kv(txt, "password", o["password"])
        else:
            txt = re.sub(r"^(user|password)=", r"#\1=", txt, flags=re.M)
        return txt
    rw(os.path.join(conf, "onvif_simple_server.conf"), onvif)


def copy_sd(dst, dry):
    skip_dirs = {"logs", "state", "dump"}
    count = 0
    for root, dirs, files in os.walk(SRC):
        rel = os.path.relpath(root, SRC)
        dirs[:] = [d for d in dirs if not (rel == "." and d in skip_dirs)]
        for d in dirs:
            if not dry:
                os.makedirs(os.path.join(dst, rel, d) if rel != "." else os.path.join(dst, d), exist_ok=True)
        for f in files:
            if f == ".gitkeep":
                continue
            target = os.path.join(dst, rel, f) if rel != "." else os.path.join(dst, f)
            count += 1
            if not dry:
                os.makedirs(os.path.dirname(target), exist_ok=True)
                shutil.copyfile(os.path.join(root, f), target)
                try:
                    shutil.copymode(os.path.join(root, f), target)
                except OSError:
                    pass
    if not dry:
        for d in ("logs", os.path.join("custom", "state")):
            os.makedirs(os.path.join(dst, d), exist_ok=True)
    return count


def find_file(root, name):
    for r, _, fs in os.walk(root):
        if name in fs:
            return os.path.join(r, name)
    return None


def patch_firmware(dst, dump):
    """Génère anyka_ipc_fps20b et isp_sc2336_fps20.conf depuis le dump de VOTRE caméra."""
    jobs = (("anyka_ipc", "patch_anyka_ipc.py", os.path.join(dst, "custom", "bin", "anyka_ipc_fps20b")),
            ("isp_sc2336_mipi_2lane_h3b.conf", "patch_isp_conf.py", os.path.join(dst, "custom", "configs", "isp_sc2336_fps20.conf")))
    ok = True
    for name, tool, out in jobs:
        src = find_file(dump, name)
        if not src:
            print(f"  ! {name} introuvable dans {dump} : étape ignorée")
            ok = False
            continue
        r = run(sys.executable, os.path.join(ROOT, "tools", tool), src, out, check=False)
        print(f"  {'OK ' if r.returncode == 0 else '!! '}{name} -> {os.path.basename(out)}" +
              ("" if r.returncode == 0 else "\n" + (r.stderr or r.stdout).strip()))
        ok &= r.returncode == 0
    return ok


def patch_from_card(arg):
    """Second passage : après un premier démarrage, la caméra a copié son firmware dans <carte>/dump/."""
    target, mounted = pick_target(arg or "", False)
    try:
        dumps = sorted(glob.glob(os.path.join(target, "dump", "*")), key=os.path.getmtime, reverse=True)
        if not dumps:
            sys.exit("Aucun dossier dump/ sur la carte : démarrez d'abord la caméra avec cette carte (une fois), puis rééteignez-la.")
        print(f"Dump utilisé : {dumps[0]}")
        if not patch_firmware(target, dumps[0]):
            sys.exit("Patch incomplet : voir les messages ci-dessus (firmware différent de la version testée ?).")
        os.sync()
        print("Terminé. Remettez la carte dans la caméra et redémarrez-la : 20 images/s.")
    finally:
        if mounted:
            run("umount", mounted, check=False)
            os.rmdir(mounted)


# ---------- programme ----------
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--config", help="fichier YAML de configuration")
    ap.add_argument("--target", help="/dev/sdX1 ou point de montage de la carte SD")
    ap.add_argument("--patch-firmware", action="store_true",
                    help="applique le patch 20 images/s depuis le dump que la caméra a écrit sur la carte (dump/), puis quitte")
    ap.add_argument("--yes", action="store_true", help="non interactif : aucune question, aucune confirmation")
    ap.add_argument("--dry-run", action="store_true", help="n'écrit rien")
    ap.add_argument("--save-config", help="enregistre la configuration finale (défaut : proposé en fin d'assistant)")
    a = ap.parse_args()

    if a.patch_firmware:
        return patch_from_card(a.target)

    cfg = DEFAULTS
    if a.config:
        with open(a.config, encoding="utf-8") as f:
            cfg = merge(DEFAULTS, parse_yaml(f.read()))
    if a.target:
        cfg["target"] = a.target
    interactive = not a.yes
    if interactive:
        cfg = wizard(cfg) if not a.config else _fill(cfg)
    err, warn = validate(cfg)
    for w in warn:
        print(f"  ATTENTION : {w}")
    if err:
        sys.exit("Configuration invalide :\n  - " + "\n  - ".join(err))

    o = cfg["onvif"]
    generated = None
    if o["enabled"] and o["password"] == "auto":
        generated = o["password"] = secrets.token_urlsafe(12)

    sec = cfg["security"]
    root_generated = None
    if sec["root_password"] == "auto":
        root_generated = sec["root_password"] = secrets.token_urlsafe(12)

    target, mounted = ((cfg["target"], None) if (a.dry_run and not cfg["target"]) else pick_target(cfg["target"], a.yes))
    print(f"\nCarte cible : {target}")
    if interactive and not a.dry_run:
        if ask("Copier et personnaliser ici ? Tapez OUI pour confirmer", "").upper() != "OUI":
            sys.exit("Annulé.")
    try:
        n = copy_sd(target, a.dry_run)
        print(f"{n} fichiers {'à copier' if a.dry_run else 'copiés'}.")
        if not a.dry_run:
            render(target, cfg)
            print("Configuration écrite (_ak39_factory.ini, wifi.conf, hack.conf, _ht_hw_settings.ini, onvif_simple_server.conf).")
            if cfg["firmware"]["dump_dir"]:
                print("Patch 20 images/s depuis le dump :")
                patch_firmware(target, cfg["firmware"]["dump_dir"])
            os.sync()
    finally:
        if mounted:
            run("umount", mounted, check=False)
            os.rmdir(mounted)

    save = a.save_config
    if not save and interactive and not a.config and not a.dry_run:
        if ask_bool("Enregistrer ces réglages dans install.yaml (contient les mots de passe, ignoré par git)", True):
            save = os.path.join(ROOT, "install.yaml")
    if save and not a.dry_run:
        with open(save, "w", encoding="utf-8") as f:
            f.write(dump_yaml(cfg))
        os.chmod(save, 0o600)
        print(f"Réglages enregistrés dans {save} (droits 600).")
    if generated:
        print(f"\nMot de passe ONVIF généré (à noter, affiché une seule fois) : {generated}")
    if root_generated:
        print(f"Mot de passe root généré (telnet 'login' et port 23) : {root_generated}")
    print("\nTerminé. Éteignez la caméra, insérez la carte, rallumez : voir docs/INSTALL.md (vérifications).")


def _fill(cfg):
    """Mode --config : ne demande que les champs obligatoires manquants."""
    n = cfg["network"]
    if not n["ssid"]:
        n["ssid"] = ask("SSID", required=True)
    if n["security"] != 1 and not n["password"]:
        n["password"] = ask("Mot de passe Wi-Fi", secret=True, required=True)
    return cfg


if __name__ == "__main__":
    main()
