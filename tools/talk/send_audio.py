#!/usr/bin/env python3
"""Envoie un fichier audio au haut-parleur de la caméra (talk_inject.so).

Usage : send_audio.py <ip_caméra> <fichier> [--port 5004]
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
a = ap.parse_args()

pcm = subprocess.run(
    ["ffmpeg", "-v", "error", "-i", a.file, "-ac", "1", "-ar", "8000", "-f", "mulaw", "-"],
    capture_output=True, check=True).stdout
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
CH = 160  # 20 ms
t0 = time.time()
for i in range(0, len(pcm), CH):
    s.sendto(pcm[i:i + CH], (a.ip, a.port))
    time.sleep(max(0, t0 + (i + CH) / 8000.0 - time.time()))
