# Comparatif des modèles de caméra

Relevé sur sept caméras Tuya/Anyka AK3918EV330 (nom interne `IPC-XC-C323` confirmé sur cam181 et cam187) ; versions de firmware relevées le 22/11 après mise à jour des sept caméras, toutes pilotées par le même toolkit. Utile si vous cherchez à **reprendre la main sur votre caméra** : ce tableau dit ce qui varie d'un modèle à l'autre et ce que le toolkit adapte tout seul. `?` = non relevé. Le flux est lu avec `ffprobe` sur `rtsp://<ip>:554/main_ch`.

| | cam181 | cam184 | cam185 | cam186 | cam187, cam188 | cam189 |
|---|---|---|---|---|---|---|
| Type | fixe extérieure | PTZ intérieure | PTZ | PTZ intérieure | PTZ intérieure (référence du toolkit) | PTZ |
| **Firmware** (`/usr/share/VERSION`) | 3.178.61.9 | 3.167.94.14 | 3.167.94.14 | 3.286.74.13 | 3.286.81.13 | 3.286.81.13 |
| Capteur (`isp_*.conf`) | MIS2006 | MIS2006 | MIS2006 | MIS2006 | SC2336 | SC2336 |
| PTZ | non | oui (`pan/tilt_total_steps`, `ak_motor.ko`) | oui | oui | oui | oui |
| `sensor_type_def` / `ircut_type` | 4 / 2 | 4 / 2 | 17 / 1 | 17 / 1 | 17 / 1 | 17 / 1 |
| **Flux `main_ch` (actuel)** | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s | H.265 720p, 15 i/s |
| Où est lue la résolution / le codec | `/usr/local/_ht_hw_settings.ini` (3.x) ; ancien chemin `/etc/config` à revérifier | idem | idem | idem | idem | idem |
| Audio | AAC-LC 8 kHz mono | idem | idem | idem | idem | idem |
| Sous-flux (`sub_ch`) | absent (non exploitable, voir ci-dessous) | absent | absent | absent | absent (essais cam187 : échec) | absent |
| LED (`/sys/class/leds`) | `state_led` + `fill_light_led` | `state_led` seule | `state_led` seule | les deux | les deux | les deux |
| ONVIF (8080) et découverte WS-Discovery | oui | oui | oui | oui | oui | oui |
| Heure (NTP via la passerelle) | juste | juste | juste | juste | juste | juste |
| Port 24 (telnet du toolkit) | fermé | fermé | fermé | fermé | cam187 ouvert, cam188 fermé | fermé |

Notes :
- Le critère « PTZ » ne repose pas sur `ptz_support` seul : `hack.sh` regarde aussi `pan_total_steps` / `tilt_total_steps`.
- Mise à jour des firmwares : toutes les caméras sont en 3.x (plus de 2.10.x). cam187, cam188 et cam189 partagent 3.286.81.13 ; cam184 et cam185 partagent 3.167.94.14 ; cam186 est en 3.286.74.13 et cam181 en 3.178.61.9. `hack.sh` ne reconnaît comme profil testé que 3.10.78.1269 : les autres versions gardent leurs réglages d'origine, à revalider après cette mise à jour.
- Cadence réelle mesurée le soir : 10 à 14 i/s (l'exposition automatique du capteur peut la réduire) ; à surveiller de jour.
- Les caméras joignent encore Internet : pour un flux 100 % LAN, bloquer leur accès au routeur.

## Bilan matériel complet

Relevé en lecture seule (`uname`, `/proc/cpuinfo`, `/proc/meminfo`, `/proc/mtd`, `lsmod`, `/sys/class/leds`) sur les sept caméras. Le détail d'une caméra de référence (GPIO, DTB, moteurs, UART) est dans [`HARDWARE.md`](HARDWARE.md).

**Identique sur les 7 caméras**

| Élément | Valeur |
|---|---|
| SoC / CPU | Anyka AK3918EV330, ARM926EJ-S rev 5 (ARMv5TEJL), mono-cœur, soft-float |
| Noyau | Linux 4.4.192 (`armv5tejl`) |
| RAM | 64 Mo déclarés, 35 720 ko visibles dans `/proc/meminfo` (le reste est réservé à la vidéo) |
| Flash | SPI NOR 8 Mo : UBOOT 200 Ko, ENV 4 Ko, ENVBK 4 Ko, DTB 48 Ko, KERNEL 1,5 Mo, ROOTFS 1 Mo, CONFIG 256 Ko (jffs2, `/etc/config`), APP 5 Mo |
| Wi-Fi | Realtek RTL8188FTV, USB (`rtl8188ftv`, `ak_hcd`) ; le fichier `ssv6x5x-wifi.cfg` est présent mais inutilisé |
| Stockage | microSD FAT32 sur `mmc0` (cartes de 7 à 15 Go utilisées) |
| Audio | codec intégré (`ak_pcm`), AAC-LC 8 kHz mono |
| Pilotes communs | `ak_isp`, `ak_i2c`, `ak_gpio_keys`, `ak_leds`, `ak_saradc` (capteur de lumière), `ak_rtc`, `ak_ion`, `ak_uio` |

**Ce qui change d'un modèle à l'autre**

| | cam181 | cam184 | cam185 | cam186 | cam187, cam188 | cam189 |
|---|---|---|---|---|---|---|
| Capteur d'image | MIS2006 (`sensor_mis2006`) | MIS2006 | MIS2006 | MIS2006 | SC2336 (`sensor_sc2336`) | SC2336 |
| Fichier ISP | `isp_mis2006_mipi_2lane_h3b.conf` | idem | idem | idem | `isp_sc2336_mipi_2lane_h3b.conf` | idem |
| Moteurs PTZ (`ak_motor`) | **absent** | présent | présent | présent | présent | présent |
| LED disponibles | `state_led`, `fill_light_led`, `ir_led` | `state_led`, `ir_led` | `state_led`, `ir_led` | les trois | les trois | les trois |
| Script Wi-Fi | `wifi_driver.sh` | ? | ? | ? | `wifi_driver_new.sh` | ? |
| Firmware | 3.178.61.9 | 3.167.94.14 | 3.167.94.14 | 3.286.74.13 | 3.286.81.13 | 3.286.81.13 |

À retenir : le matériel de base est le même partout ; seuls changent le **capteur** (MIS2006 ou SC2336), la présence du **PTZ** et les **LED** câblées. Les firmwares 2.10.x lisent leurs réglages dans `/etc/config`, les 3.x dans `/usr/local`.

## Ce qu'il faut encore « patcher »

**Aucun fichier du firmware n'est modifié** : la mémoire flash n'est jamais écrite. Tout passe par la carte SD au démarrage :

- des montages `mount --bind` en RAM (réglages matériels, `anyka_ipc.sh`, `state_led.sh`, conf ONVIF) ;
- des bibliothèques injectées par `LD_PRELOAD` dans `anyka_ipc` (`md_inject.so` : MQTT, horloge, haut-parleur) ;
- un serveur ONVIF et un démon de découverte (`wsd`) lancés depuis la SD.

Le seul vrai patch binaire est le **20 i/s** (`anyka_ipc` + fichier ISP, `VIDEO_FPS20=1`) : optionnel, désactivé par défaut, limité à l'ancien firmware 3.10.78.1269 (contrôle MD5) : ne s'applique plus aux versions actuelles. Avec 15 i/s sur toutes les caméras, plus aucun patch n'est nécessaire.

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
- Firmwares : la résolution est lue dans `/usr/local/_ht_hw_settings.ini` (3.x) ou `/etc/config/_ht_hw_settings.ini`, surchargés par le profil effectif ; à revérifier sur 181 et 184 depuis leur passage en 3.x.
- LED d'état éteinte sur tous les modèles (`state_led.sh` gère `fill_light_led` et `state_led`).

## Sous-flux : essais et conclusion

Le firmware ne publie aucun `sub_ch` sur 554. Le profil matériel contient pourtant des clés `sub_*` (`sub_fps`, `sub_enc_out_type`, résolution secondaire), et l'encodeur secondaire existe dans `anyka_ipc`. Tentative faite sur cam187 (alors en firmware 3.10.78.1269, avant mise à jour) : profil secondaire 640×360 à 5 i/s plus une extension `LD_PRELOAD` (`substream.so`) qui sert l'encodeur secondaire en RTSP sur le port 553 (`/sub_ch`), le flux principal restant strictement identique.

| Essai | Résultat |
|---|---|
| Sous-flux H.265 | pertes UDP, puis caméra indisponible |
| Sous-flux H.264, 5 i/s, UDP | port 553 jamais ouvert ; après un reboot complet, la caméra n'a pas redémarré (cause non identifiée : aucun journal `anyka` n'a été conservé) |

Conclusion : **sous-flux non exploitable** avec le firmware d'origine. Retour arrière effectué (`SUBSTREAM` retiré, fichiers d'origine restaurés). La référence CPU `anyka_ipc` sans sous-flux est d'environ 24 à 29 % selon la mesure ; l'impact d'un sous-flux n'a pas pu être mesuré.

## MQTT et Home Assistant : indépendance vis-à-vis du firmware

Relevé le 22/11 sur le broker (messages `homeassistant/+/+/+/config`, lecture seule) après la mise à jour des firmwares.

- **La découverte ne dépend pas de la version.** La liste des entités vient de `dp_table.h` / `DP_VALUES.md` (générée par `gen_ha_discovery.py`), pas d'une lecture de la caméra : cam181, 184, 185, 186, 187 et 189 annoncent chacune les mêmes **66 entités**, quel que soit le firmware, le capteur ou la présence du PTZ (la cam181, sans moteur, expose donc aussi les entités PTZ). Le champ `model` annoncé (`anyka_ipc 3.10.78`) est un texte fixe, il ne reflète pas le firmware réel.
- **Ce qui dépend du firmware est l'exécution.** Les fonctions `IPC_APP_*` sont des symboles faibles résolus dans `anyka_ipc` : un symbole absent vaut NULL et n'est jamais appelé (la commande est alors sans effet). Les preuves (adresses) de `DP_VALUES.md` viennent du firmware 3.10.78.1269. Sur 3.167.94.14, 3.178.61.9, 3.286.74.13 et 3.286.81.13, chaque commande MQTT reste à valider caméra par caméra.

### Pourquoi la cam188 a 72 entités au lieu de 66

Écart exact (comparaison des topics retained de camera187 et camera188, et de la sortie actuelle du générateur) :

| Entité en plus sur camera188 | Composant |
|---|---|
| `ipc_lullaby_volume` | `number` |
| `ipc_lullaby_command`, `ipc_lullaby_control`, `ipc_lullaby_list`, `ipc_lullaby_mode` | `select` |
| `system_reboot` | `text` |

Ces 6 entités **ne sont plus produites par le générateur actuel** (sa sortie fait 66 entités, comme les autres caméras ; la berceuse est maintenant le `select` `lullaby` et le redémarrage le `button` `reboot`). Ce sont des **restes d'anciennes annonces retained** : la cam188 a été la caméra de développement, elle a reçu une ancienne version de `md_inject.so` qui annonçait ces DP, et les messages retained du broker n'ont jamais été effacés. Ce n'est pas une différence de matériel ni de firmware (la cam188 est identique aux cam187 et cam189 : même capteur et même firmware 3.286.81.13).

Pour les retirer de Home Assistant, effacer ces six messages retained (charge vide), par exemple :
`for t in number/camera188/ipc_lullaby_volume select/camera188/ipc_lullaby_command select/camera188/ipc_lullaby_control select/camera188/ipc_lullaby_list select/camera188/ipc_lullaby_mode text/camera188/system_reboot; do mosquitto_pub -r -t "homeassistant/$t/config" -n; done`
(avec l'hôte et les identifiants du broker). Non exécuté à ce jour.
