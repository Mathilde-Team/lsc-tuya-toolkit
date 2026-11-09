#!/usr/bin/env python3
"""Patch de isp_sc2336_mipi_2lane_h3b.conf : capteur SC2336 à N fps (défaut 20) au lieu de 14.

Le fichier d'origine plafonne le capteur à 14 fps (`frame_rate=14`, `max_exp_time=2385`).
Les paramètres sont présents en deux exemplaires dans le fichier (0x8cb4 et 0x1b75c) ; 5 mots
de 32 bits sont modifiés dans chacun. Chaque valeur d'origine est contrôlée avant écriture.

max_exp_time suit la loi fps * max_exp_time ~= constante : 2385 pour 14 fps, 1688 pour 20 fps
(valeur testée sur la caméra). Pour un autre fps la valeur est extrapolée, NON TESTÉE.

Fichier testé : md5 e5cd9fc1f2e88d2f99aa4041cd3bb646
(capteur SC2336, caméra IPC-XC-C323, firmware 3.10.78.1269).
"""
import argparse
import hashlib
import struct
import sys

EXPECTED_MD5 = "e5cd9fc1f2e88d2f99aa4041cd3bb646"
COPIES = (0x8CB4, 0x1B75C)
OLD_FPS, OLD_EXP = 14, 2385
# (décalage depuis la copie, valeur d'origine, rôle)
FIELDS = ((-4, OLD_FPS, "fps"), (0, OLD_EXP, "exp"), (8, OLD_FPS, "fps"), (12, OLD_EXP, "exp"), (20, OLD_EXP, "exp"))
TESTED = {20: 1688}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("src", help="isp_sc2336_mipi_2lane_h3b.conf d'origine (/usr/local/ ou /etc/config/ sur la caméra)")
    ap.add_argument("dst", help="fichier à écrire (ex. sd_card/custom/configs/isp_sc2336_fps20.conf)")
    ap.add_argument("--fps", type=int, default=20)
    ap.add_argument("--force", action="store_true", help="ignorer le contrôle MD5 (valeurs attendues toujours vérifiées)")
    a = ap.parse_args()

    data = bytearray(open(a.src, "rb").read())
    md5 = hashlib.md5(data).hexdigest()
    if md5 != EXPECTED_MD5 and not a.force:
        sys.exit(f"MD5 inattendu {md5} (attendu {EXPECTED_MD5}) : autre capteur ou version ? --force pour tenter.")
    exp = TESTED.get(a.fps) or round(33760 / a.fps)
    if a.fps not in TESTED:
        print(f"ATTENTION : {a.fps} fps jamais testé (max_exp_time extrapolé à {exp}).", file=sys.stderr)

    for base in COPIES:
        for rel, old, _ in FIELDS:
            val = struct.unpack_from("<I", data, base + rel)[0]
            if val != old:
                sys.exit(f"Valeur inattendue à 0x{base + rel:x} : {val} (attendu {old}). Rien écrit.")
    for base in COPIES:
        for rel, _, role in FIELDS:
            struct.pack_into("<I", data, base + rel, a.fps if role == "fps" else exp)

    open(a.dst, "wb").write(data)
    print(f"écrit {a.dst}  md5={hashlib.md5(data).hexdigest()}  ({a.fps} fps, max_exp_time={exp})")


if __name__ == "__main__":
    main()
