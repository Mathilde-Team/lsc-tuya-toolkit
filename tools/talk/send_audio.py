#!/usr/bin/env python3
"""Envoie un fichier audio au haut-parleur de la caméra (talk_inject.so).

Usage : send_audio.py <ip_caméra> <fichier> [--port 5004] [--loop] [--max SECONDES]
Convertit avec ffmpeg en G.711 µ-law 8 kHz mono et envoie en UDP au rythme réel.
"""
import argparse
import socket
import subprocess
import time

ap = argparse.ArgumentParser()
ap.add_argument("ip")
ap.add_argument("file")
ap.add_argument("--port", type=int, default=5004)
ap.add_argument("--loop", action="store_true", help="rejouer en boucle")
ap.add_argument("--max", type=float, default=30, help="durée maximale en secondes (défaut 30)")
a = ap.parse_args()

pcm = subprocess.run(
    ["ffmpeg", "-v", "error", "-i", a.file, "-ac", "1", "-ar", "8000", "-f", "mulaw", "-"],
    capture_output=True, check=True).stdout
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CH = 160  # 20 ms
t0 = time.time()
sent = 0
while True:
    for i in range(0, len(pcm), CH):
        if time.time() - t0 >= a.max:
            raise SystemExit(0)
        s.sendto(pcm[i:i + CH], (a.ip, a.port))
        sent += CH
        time.sleep(max(0, t0 + sent / 8000.0 - time.time()))
    if not a.loop:
        break
