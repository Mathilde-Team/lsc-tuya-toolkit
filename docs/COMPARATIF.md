# Comparatif des modèles de caméra

Relevé en lecture seule sur sept caméras Tuya/Anyka AK3918EV330, dont le nom interne `IPC-XC-C323` est confirmé sur les cam181 et cam187. Seules les valeurs vérifiées sont indiquées ; `?` = non relevé. Le flux est lu avec `ffprobe` sur `rtsp://<ip>:554/main_ch`.

| | cam181 | cam184 | cam185 | cam186 | cam187, cam188 | cam189 |
|---|---|---|---|---|---|---|
| Type | fixe extérieure | PTZ intérieure | PTZ | PTZ intérieure | PTZ intérieure (référence du toolkit) | PTZ |
| **Firmware** (`/usr/share/VERSION`) | **2.10.33.993** | **2.10.34.981** (lu sur sa carte SD) | **3.167.94.14** | **3.10.72.1254** | **3.10.78.1269** | **3.286.81.13** |
| `anyka_ipc` (MD5) | `972a3c93…` | `?` (non relevé) | `fd726ad2…` | `09ba8e90…` | `27ee718f…` (binaire patché 20 i/s) | `1adee3a4…` |
| Capteur (`isp_*.conf`) | MIS2006 | MIS2006 | MIS2006 | MIS2006 | SC2336 | SC2336 |
| `ptz_support` | non (clé absente) | oui d'après `pan/tilt_total_steps` et `ak_motor.ko` (clé `ptz_support` absente) | oui | oui | oui | oui |
| Réglages matériels actifs (`sensor_type_def` / `ircut_type`) | 4 / 2 (d'origine) | 4 / 2 (d'origine, carte) | 17 / 1 (d'origine) | 17 / 1 (d'origine) | 17 / 1 | 17 / 1 (d'origine) |
| Flux `main_ch` | H.264 720p cible, 15 i/s | H.265 1080p, ≈14 i/s (à confirmer : caméra injoignable avant réinstallation) | H.265 1080p, ≈14 i/s | H.265 1080p, ≈14 i/s | H.265 720p, 20 i/s | H.265 1080p (cadence annoncée 40 i/s, à vérifier) |
| Audio | AAC-LC 8 kHz mono | idem | idem | idem | idem | idem |
| Sous-flux (`sub_ch`) | absent | absent | absent | absent | absent | absent |
| `/usr/local/etc` (conf ONVIF) | absent | `?` | absent | absent | absent | absent |
| Port 24 (telnet du toolkit) | fermé | fermé | fermé | fermé | cam187 ouvert, cam188 fermé | fermé |
| Heure | juste | `?` | juste | juste | décalée (+7 à +8 h constatés : version sans la correction NTP/TZ) | **fausse (an 2000)** |


Écarts à noter :
- La cam184 a deux identités dans nos relevés : un firmware 2.10.34.981 (MIS2006, `ircut_type = 2`, `sensor_type_def = 4`) lu sur sa carte, et un flux H.265 relevé en RTSP à son adresse avant la correction du DHCP, qui était probablement celui d'une autre caméra. Le flux et l'heure de la cam184 sont à relever après son démarrage avec la nouvelle carte.
- Le critère « PTZ » ne doit pas reposer sur `ptz_support` seul : `hack.sh` regarde aussi `pan_total_steps` / `tilt_total_steps`.
- La cam185 (3.167.94.14) et la cam189 (3.286.81.13) sont deux firmwares encore non traités par le toolkit ; leurs réglages d'origine restent à relever.
- La cam189 avait un `hack.sh` incomplet (heure de l'an 2000) ; corrigé par le déploiement de la version actuelle.
- Le MD5 `3d8cad80…` précédemment cité est celui du binaire de référence non patché ; les cam187 et cam188 exécutent la version patchée.

## Conséquences pour le toolkit

- Le fichier `_ht_hw_settings.ini` du dépôt est celui d'une caméra PTZ de référence : le copier tel quel sur une autre variante écrase l'IR-cut, le capteur et les paramètres moteur. `hack.sh` compare au démarrage le firmware et le capteur au profil testé (voir `docs/INSTALL.md`, §4 quater) : sur une autre variante, les réglages d'origine sont gardés ; sans `ptz_support`, ONVIF n'annonce pas de PTZ et le recalage PTZ est ignoré. 
- Cadence unifiée à 15 i/s d'origine (`VIDEO_FPS20=0`) sur toutes les caméras. Le patch 20 i/s (`patch_fw.sh`, optionnel) cible le firmware 3.10.78.1269 : il ne s'applique pas à un autre binaire.
- Sur les variantes à `/usr/local/etc` absent, la conf ONVIF de la SD n'est pas montée : à traiter (état à vérifier caméra par caméra).
- Identifier un modèle : lire `/usr/share/VERSION`, `ptz_support` dans `/etc/config/_ht_hw_settings.ini` et le fichier `isp_*.conf` de `/etc/config`.
- Les fonctions liées au PTZ (presets, positions absolues, mode privé par position) n'existent pas sur une caméra fixe.

## Passe d'harmonisation (15 i/s, version actuelle du toolkit)

- `VIDEO_FPS20=0` par défaut : plus de binaire patché ; cam187/cam188 repassées au `anyka_ipc` d'origine. Sur les autres variantes, le fichier de réglages d'origine est gardé avec `main_fps = 15`.
- Déployé et redémarré : cam181, 185, 186, 187, 188, 189 (PTZ détecté sur toutes sauf la 181, heure juste, `md_inject` chargé, ONVIF 8080 ouvert). La cam184 reçoit cette version avec sa nouvelle carte.
- Cadence mesurée sur 12 s : 10 à 14 i/s selon la caméra et la luminosité (l'exposition automatique du capteur peut la réduire) ; à surveiller de jour.

## Résolution unifiée : 720p

Tous les flux `main_ch` sont réglés en 1280×720 (légèreté : débit et charge Home Assistant/Frigate). Le dépôt (`_ht_hw_settings.ini`, conf ONVIF, `install.py`) est en 720p par défaut ; sur les variantes autres que la référence, `hack.sh` reporte seulement les résolutions de la carte dans le profil d'origine. Le tableau ci-dessus indique les flux relevés avant ce changement (1080p sauf cam187/188) ; à relever à nouveau après redéploiement.

## Codec et LED (mise à jour)

- Toutes les caméras : **H.265 720p 15 i/s** (`VIDEO_H265=1` dans `hack.conf` ; la 181 était en H.264, la clé est `main_enc_out_type` du profil matériel).
- Firmwares 2.10.x (181, 184) : la résolution est lue dans `/etc/config/_ht_hw_settings.ini`, désormais surchargé par le profil effectif.
- LED d'état éteinte sur tous les modèles (`state_led.sh` gère `fill_light_led` et `state_led`).
