# Comparatif des modèles de caméra

Relevé en lecture seule sur sept caméras Tuya/Anyka AK3918EV330, dont le nom interne `IPC-XC-C323` est confirmé sur les cam181 et cam187. Seules les valeurs vérifiées sont indiquées ; `?` = non relevé. Le flux est lu avec `ffprobe` sur `rtsp://<ip>:554/main_ch`.

| | cam181 | cam184 | cam185 | cam186 | cam187, cam188 | cam189 |
|---|---|---|---|---|---|---|
| Type | fixe extérieure | `?` | PTZ | PTZ intérieure | PTZ intérieure (référence du toolkit) | PTZ |
| **Firmware** (`/usr/share/VERSION`) | **2.10.33.993** | `?` (accès telnet impossible) | **3.167.94.14** | **3.10.72.1254** | **3.10.78.1269** | **3.286.81.13** |
| `anyka_ipc` (MD5) | `972a3c93…` | `?` | `fd726ad2…` | `09ba8e90…` | `27ee718f…` (binaire patché 20 i/s) | `1adee3a4…` |
| Capteur (`isp_*.conf`) | MIS2006 | `?` | MIS2006 | MIS2006 | SC2336 | SC2336 |
| `ptz_support` | non (clé absente) | `?` | oui | oui | oui | oui |
| Réglages matériels actifs (`sensor_type_def` / `ircut_type`) | 4 / 2 (d'origine) | `?` | 17 / 1 (*) | 17 / 1 (*) | 17 / 1 | 17 / 1 (non vérifié : le hack n'a pas tourné) |
| Flux `main_ch` | H.264 1080p, 15 i/s | H.265 1080p, ≈14 i/s | H.265 1080p, ≈14 i/s | H.265 1080p, ≈14 i/s | H.265 720p, 20 i/s | H.265 1080p (cadence annoncée 40 i/s, à vérifier) |
| Audio | AAC-LC 8 kHz mono | idem | idem | idem | idem | idem |
| Sous-flux (`sub_ch`) | absent | absent | absent | absent | absent | absent |
| `/usr/local/etc` (conf ONVIF) | absent | `?` | absent | absent | absent | absent |
| Port 24 (telnet du toolkit) | fermé | fermé | fermé | fermé | cam187 ouvert, cam188 fermé | fermé |
| Heure | juste | `?` | juste | juste | décalée (+7 à +8 h constatés : version sans la correction NTP/TZ) | **fausse (an 2000)** |

(*) Ces caméras ont un capteur MIS2006 mais le fichier actif annonce `sensor_type_def = 17` et `ircut_type = 1` : c'est le `_ht_hw_settings.ini` PTZ de référence, copié par une ancienne version de `hack.sh`. Les valeurs d'origine de la cam186 étaient `sensor_type_def = 4`, `ircut_type = 2`, H.264. La version actuelle de `hack.sh` garde les réglages d'origine sur ces variantes.

Écarts à noter :
- La cam185 (3.167.94.14) et la cam189 (3.286.81.13) sont deux firmwares encore non traités par le toolkit ; leurs réglages d'origine restent à relever.
- La cam189 n'a ni `/tmp/hacklogs` ni `/tmp/hw_ptz` : `hack.sh` ne s'y est pas exécuté complètement, d'où l'heure de l'an 2000.
- Le MD5 `3d8cad80…` précédemment cité est celui du binaire de référence non patché ; les cam187 et cam188 exécutent la version patchée.

## Conséquences pour le toolkit

- Le fichier `_ht_hw_settings.ini` du dépôt est celui d'une caméra PTZ de référence : le copier tel quel sur une autre variante écrase l'IR-cut, le capteur et les paramètres moteur. `hack.sh` compare au démarrage le firmware et le capteur au profil testé (voir `docs/INSTALL.md`, §4 quater) : sur une autre variante, les réglages d'origine sont gardés ; sans `ptz_support`, ONVIF n'annonce pas de PTZ et le recalage PTZ est ignoré. Les caméras marquées (*) doivent recevoir cette version de `hack.sh` (redémarrage nécessaire).
- Le patch 20 i/s (`patch_fw.sh`) cible le firmware 3.10.78.1269 : il ne s'applique pas à un autre binaire.
- Sur les variantes à `/usr/local/etc` absent, la conf ONVIF de la SD n'est pas montée : à traiter (état à vérifier caméra par caméra).
- Identifier un modèle : lire `/usr/share/VERSION`, `ptz_support` dans `/etc/config/_ht_hw_settings.ini` et le fichier `isp_*.conf` de `/etc/config`.
- Les fonctions liées au PTZ (presets, positions absolues, mode privé par position) n'existent pas sur une caméra fixe.
