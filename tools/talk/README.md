# Haut-parleur par le LAN (talk_inject)

`anyka_ipc` sait déjà jouer de l'audio « talk » (interphone Tuya) via `ht_audio_codec_talk_open` /
`ht_audio_codec_play_talk_data` / `ht_audio_codec_talk_close`, mais seuls les callbacks P2P du cloud les appellent.
`talk_inject.so` est chargée dans `anyka_ipc` par `LD_PRELOAD` et ouvre un port UDP qui alimente ce chemin.
Le micro et la vidéo ne sont pas touchés. Testé sur la caméra IPC-XC-C323 (AK3918EV330, firmware 3.10.78.1269).

- **Format** : G.711 µ-law, 8 kHz, mono, brut, en paquets UDP de 160 à 1024 octets (20 ms = 160 octets).
- **Port** : `TALK_PORT` (5004 par défaut). **Filtre** : `TALK_ALLOW=<IPv4>` n'accepte que cette source.
  Sans `TALK_ALLOW`, toute machine du LAN peut faire parler la caméra : à toujours renseigner.
- `TALK_GAIN` : volume en pour-cent (1 à 100).
- La bibliothèque attend 40 s après le démarrage, et referme la voie audio après 1,5 s sans paquet.

## Compiler

```
./build.sh        # nécessite arm-linux-gnueabi-gcc ; produit talk_inject.so (sans glibc, résolu contre uClibc)
```

## Installer sur la caméra

Avec l'installeur : section `speaker:` de `install.yaml` (`hack.conf` : `TALK`, `TALK_ALLOW`, `TALK_GAIN`, `TALK_PORT`) ; la bibliothèque compilée est dans `sd_card/custom/bin/`. À la main :

1. Copier `talk_inject.so` dans `custom/bin/` de la carte SD.
2. Dans `custom/scripts/anyka_ipc.sh`, lancer le binaire avec :
   `LD_PRELOAD=/tmp/sd/custom/bin/talk_inject.so TALK_ALLOW=<ip du PC> anyka_ipc ...`
3. Redémarrer la caméra (`reboot`), attendre environ 1 minute.

## Envoyer un son

```
./send_audio.py <ip_caméra> son.wav [--port 5004]     # ffmpeg convertit en µ-law 8 kHz
```

Commencer avec un volume faible.
