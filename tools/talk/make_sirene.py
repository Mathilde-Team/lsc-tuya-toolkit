#!/usr/bin/env python3
"""Génère la sirène d'alarme (sirene.wav + sirene.ulaw) — libre de droits.

Le son est entièrement synthétisé ici (sinusoïde à balayage de fréquence,
« wail ») : aucun fichier du fabricant n'est utilisé, la provenance est le
script lui-même. Sorties reproductibles octet pour octet (aucun aléa) :

- sirene.wav  : WAV PCM 16 bits, mono, 8 kHz (écoute sur PC) ;
- sirene.ulaw : G.711 µ-law brut, 8 kHz (format attendu par send_audio.py,
                envoyé tel quel au port UDP de la caméra, sans ffmpeg).

Usage : make_sirene.py [--duree 4] [--fmin 700] [--fmax 2500] [--gain 0.6]
        make_sirene.py --check   # vérifie que les fichiers livrés sont
                                 # exactement ceux que le script produit
"""
import argparse
import array
import io
import math
import os
import sys
import wave

HERE = os.path.dirname(os.path.abspath(__file__))
RATE = 8000  # Hz — le chemin audio de la caméra est figé à 8 kHz

# ---------- G.711 µ-law (ITU-T) ----------
_BIAS = 0x84
_CLIP = 32635


def lin2ulaw(sample: int) -> int:
    """Encode un échantillon PCM 16 bits en octet µ-law (algorithme ITU-T G.711)."""
    sign = 0x80 if sample < 0 else 0x00
    if sample < 0:
        sample = -sample
    if sample > _CLIP:
        sample = _CLIP
    sample += _BIAS
    exponent = 7
    seuil = 0x4000
    while exponent > 0 and sample < seuil:
        exponent -= 1
        seuil >>= 1
    mantissa = (sample >> (exponent + 3)) & 0x0F
    return ~(sign | (exponent << 4) | mantissa) & 0xFF


# ---------- synthèse ----------
def synthese(duree: float, fmin: float, fmax: float, gain: float) -> array.array:
    """Balayages montants répétés (2 s par montée), fondu de 10 ms par bord."""
    n = int(duree * RATE)
    montee = 2.0  # secondes par montée
    out = array.array("h")
    phase = 0.0
    for i in range(n):
        t = i / RATE
        pos = (t % montee) / montee           # 0 -> 1 dans chaque montée
        f = fmin * (fmax / fmin) ** pos       # balayage exponentiel (perception linéaire)
        phase += 2 * math.pi * f / RATE
        env = min(1.0, t / 0.01, (duree - t) / 0.01)  # anti-clic
        out.append(int(gain * env * 32767 * math.sin(phase)))
    return out


def octets_wav(echantillons: array.array) -> bytes:
    tampon = io.BytesIO()
    with wave.open(tampon, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(echantillons.tobytes())
    return tampon.getvalue()


def octets_ulaw(echantillons: array.array) -> bytes:
    return bytes(lin2ulaw(s) for s in echantillons)


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--duree", type=float, default=4.0, help="durée en secondes (défaut 4)")
    ap.add_argument("--fmin", type=float, default=700.0, help="fréquence basse du balayage (défaut 700)")
    ap.add_argument("--fmax", type=float, default=2500.0, help="fréquence haute du balayage (défaut 2500)")
    ap.add_argument("--gain", type=float, default=0.6, help="amplitude 0-1 (défaut 0.6)")
    ap.add_argument("--check", action="store_true",
                    help="vérifier que les fichiers du dépôt sont identiques à la génération")
    a = ap.parse_args()

    echant = synthese(a.duree, a.fmin, a.fmax, a.gain)
    wav = os.path.join(HERE, "sirene.wav")
    ulaw = os.path.join(HERE, "sirene.ulaw")

    if a.check:
        ok = True
        for chemin, octets in ((wav, octets_wav(echant)), (ulaw, octets_ulaw(echant))):
            actuel = open(chemin, "rb").read() if os.path.exists(chemin) else b""
            if octets == actuel:
                print(f"identique : {chemin}")
            else:
                print(f"DIFFÉRENT : {chemin}", file=sys.stderr)
                ok = False
        sys.exit(0 if ok else 1)

    with open(wav, "wb") as f:
        f.write(octets_wav(echant))
    with open(ulaw, "wb") as f:
        f.write(octets_ulaw(echant))
    print(f"{wav} et {ulaw} générés ({a.duree} s, balayage {a.fmin:.0f}-{a.fmax:.0f} Hz)")


if __name__ == "__main__":
    main()
