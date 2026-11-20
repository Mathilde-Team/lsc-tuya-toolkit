# Comparatif des modèles de caméra

Relevé en lecture seule sur trois variantes de caméras Tuya/Anyka AK3918EV330 portant le même nom interne. Seules les valeurs vérifiées sont indiquées ; `?` = non relevé.

| | PTZ intérieure (référence du toolkit) | PTZ intérieure, autre variante | Fixe extérieure |
|---|---|---|---|
| Nom interne (`dev_name`) | `IPC-XC-C323` | `?` | `IPC-XC-C323` (identique : il ne permet pas de distinguer) |
| **Firmware applicatif** (`/usr/share/VERSION`) | **3.10.78.1269** | **3.10.72.1254** | **2.10.33.993** |
| SDK | V1.02.04 | V1.02.04 | V1.02.04 |
| `anyka_ipc` (MD5) | `3d8cad80…` | `?` | `972a3c93…` |
| Capteur | SC2336 | MIS2006 (`isp_mis2006_mipi_2lane_h3b.conf`) | MIS2006 (id `0x2006`) |
| `sensor_type_def` d'origine | 17 | 4 | 4 |
| PTZ (`ptz_support`) | oui | oui | non (clé absente) |
| IR-cut d'origine | `ircut_type = 1` | `ircut_type = 2` | `ircut_type = 2` (niveaux jour/nuit inversés par rapport au type 1) |
| Vidéo d'origine | H.265, 15 i/s | H.264, 15 i/s | H.264, 15 i/s, 768 kb/s |
| Vidéo avec le toolkit | H.265 20 i/s (firmware patché) | réglages d'origine | réglages d'origine |
| Script Wi-Fi du firmware | `wifi_driver.sh` | `wifi_driver_new.sh` | `wifi_driver.sh` |
| `/usr/local/etc` (conf ONVIF) | présent | absent (montage de la conf ONVIF en échec) | absent |
| RTSP | `/main_ch` seul | `?` | `/main_ch` seul |

## Conséquences pour le toolkit

- Le fichier `_ht_hw_settings.ini` du dépôt est celui d'une caméra PTZ : le copier tel quel sur une caméra fixe écraserait l'IR-cut (niveaux inversés), le capteur et les paramètres moteur. `hack.sh` compare au démarrage le firmware et le capteur au profil testé (voir `docs/INSTALL.md`, §4 quater) : sur une autre variante, les réglages d'origine sont gardés ; sans `ptz_support`, ONVIF n'annonce pas de PTZ et le recalage PTZ est ignoré.
- Le patch 20 i/s (`patch_fw.sh`) cible le firmware 3.10.78.1269 : il ne s'applique pas à un autre binaire.
- Sur les variantes à `/usr/local/etc` absent, la conf ONVIF de la SD n'est pas montée : à traiter (état à vérifier caméra par caméra).
- Identifier un modèle : lire `/usr/share/VERSION`, `ptz_support` dans `/etc/config/_ht_hw_settings.ini` et le fichier `isp_*.conf` de `/etc/config`.
- Les fonctions liées au PTZ (presets, positions absolues, mode privé par position) n'existent pas sur une caméra fixe.
