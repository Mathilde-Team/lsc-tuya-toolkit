# Comparatif des modèles de caméra

Relevé sur sept caméras Tuya/Anyka AK3918EV330 (nom interne `IPC-XC-C323` confirmé sur cam181 et cam187), toutes pilotées par le même toolkit. Utile si vous cherchez à **reprendre la main sur votre caméra** : ce tableau dit ce qui varie d'un modèle à l'autre et ce que le toolkit adapte tout seul. `?` = non relevé. Le flux est lu avec `ffprobe` sur `rtsp://<ip>:554/main_ch`.

| | cam181 | cam184 | cam185 | cam186 | cam187, cam188 | cam189 |
|---|---|---|---|---|---|---|
| Type | fixe extérieure | PTZ intérieure | PTZ | PTZ intérieure | PTZ intérieure (référence du toolkit) | PTZ |
| **Firmware** (`/usr/share/VERSION`) | 2.10.33.993 | 2.10.34.981 | 3.167.94.14 | 3.10.72.1254 | 3.10.78.1269 | 3.286.81.13 |
| Capteur (`isp_*.conf`) | MIS2006 | MIS2006 | MIS2006 | MIS2006 | SC2336 | SC2336 |
| PTZ | non | oui (`pan/tilt_total_steps`, `ak_motor.ko`) | oui | oui | oui | oui |
| `sensor_type_def` / `ircut_type` | 4 / 2 | 4 / 2 | 17 / 1 | 17 / 1 | 17 / 1 | 17 / 1 |
| **Flux `main_ch` (actuel)** | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s |
| Où est lue la résolution / le codec | `/etc/config/_ht_hw_settings.ini` (2.10.x) | idem | `/usr/local/_ht_hw_settings.ini` | idem | idem | idem |
| Audio | AAC-LC 8 kHz mono | idem | idem | idem | idem | idem |
| Sous-flux (`sub_ch`) | absent | absent | absent | absent | absent | absent |
| LED (`/sys/class/leds`) | `state_led` + `fill_light_led` | `state_led` seule | `state_led` seule | les deux | les deux | les deux |
| ONVIF (8080) et découverte WS-Discovery | oui | oui | oui | oui | oui | oui |
| Heure (NTP via la passerelle) | juste | juste | juste | juste | juste | juste |
| Port 24 (telnet du toolkit) | fermé | fermé | fermé | fermé | cam187 ouvert, cam188 fermé | fermé |

Notes :
- Le critère « PTZ » ne repose pas sur `ptz_support` seul : `hack.sh` regarde aussi `pan_total_steps` / `tilt_total_steps`.
- Les firmwares 3.167.94.14 (cam185) et 3.286.81.13 (cam189) sont gérés en gardant leurs réglages d'origine.
- Cadence réelle mesurée le soir : 10 à 14 i/s (l'exposition automatique du capteur peut la réduire) ; à surveiller de jour.
- Les caméras joignent encore Internet : pour un flux 100 % LAN, bloquer leur accès au routeur.

## Ce qu'il faut encore « patcher »

**Aucun fichier du firmware n'est modifié** : la mémoire flash n'est jamais écrite. Tout passe par la carte SD au démarrage :

- des montages `mount --bind` en RAM (réglages matériels, `anyka_ipc.sh`, `state_led.sh`, conf ONVIF) ;
- des bibliothèques injectées par `LD_PRELOAD` dans `anyka_ipc` (`md_inject.so` : MQTT, horloge, haut-parleur) ;
- un serveur ONVIF et un démon de découverte (`wsd`) lancés depuis la SD.

Le seul vrai patch binaire est le **20 i/s** (`anyka_ipc` + fichier ISP, `VIDEO_FPS20=1`) : optionnel, désactivé par défaut, limité au firmware 3.10.78.1269 (contrôle MD5). Avec 15 i/s sur toutes les caméras, plus aucun patch n'est nécessaire.

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
