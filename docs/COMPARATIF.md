# Comparatif des modèles de caméra

Relevé en lecture seule sur deux familles de caméras Tuya/Anyka AK3918EV330 portant le même nom interne. Seules les valeurs vérifiées sont indiquées ; `?` = non relevé.

| | Caméra PTZ intérieure (référence du toolkit) | Caméra fixe extérieure |
|---|---|---|
| Nom interne (`dev_name`) | `IPC-XC-C323` | `IPC-XC-C323` (identique : il ne permet pas de les distinguer) |
| `soft_version` | 3007 | 3007 |
| **Firmware applicatif** (`/usr/share/VERSION`) | **3.10.78.1269** | **2.10.33.993** |
| SDK, noyau | V1.02.04, Linux 4.4.192 V2.1 | identiques |
| `anyka_ipc` (MD5) | `3d8cad80…` | `972a3c93…` (autre binaire) |
| Capteur | SmartSens SC2336 | MIS2006 (id `0x2006`, `isp_mis2006_mipi_2lane_h3b.conf`) |
| `sensor_type_def` d'origine | 17 | 4 |
| Wi-Fi | RTL8188FTV (USB) | RTL8188FU (USB) |
| PTZ | oui (`ptz_support = 1`, moteurs pan/tilt) | **non** (clé absente) |
| IR-cut d'origine | `ircut_type = 1`, jour A=0 / nuit A=1 | `ircut_type = 2`, jour A=1 / nuit A=0 (inversé) |
| Seuils jour/nuit d'origine | 2000 / 1000 | 2048 / 1024 |
| Vidéo d'origine | H.265, 15 i/s | H.264, 15 i/s, 768 kb/s (max 1 024) |
| Vidéo avec le toolkit | H.265 20 i/s (firmware patché) | H.264 15 i/s (réglages d'origine conservés) |
| RTSP | `/main_ch` seul | `/main_ch` seul |
| Port 23 (login), 8080 (ONVIF), 554 | oui | oui |

## Conséquences pour le toolkit

- Le fichier `_ht_hw_settings.ini` du dépôt est celui d'une caméra PTZ : le copier tel quel sur une caméra fixe écraserait l'IR-cut (niveaux inversés), le capteur et les paramètres moteur. `hack.sh` détecte désormais le profil au démarrage (voir `docs/INSTALL.md`, §4 quater) : sans `ptz_support`, les réglages d'origine sont gardés, ONVIF n'annonce pas de PTZ et le recalage PTZ est ignoré.
- Le patch 20 i/s (`patch_fw.sh`) cible le firmware 3.10.78 : il ne s'applique pas à un autre binaire.
- Identifier un modèle : lire `/usr/share/VERSION`, `ptz_support` dans `/etc/config/_ht_hw_settings.ini` et le fichier `isp_*.conf` de `/etc/config`.
- Les fonctions liées au PTZ (presets, positions absolues, mode privé par position) n'existent pas sur une caméra fixe.
