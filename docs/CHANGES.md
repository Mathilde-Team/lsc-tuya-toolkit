# Journal des modifications et des actions

Ce document décrit **tout ce qui a été changé** par rapport au dépôt amont `tasarren/lsc-tuya-toolkit`, **pourquoi**, comment le
déployer, comment revenir en arrière, et la liste chronologique des actions faites sur la caméra.

- Caméra et différences avec le dépôt amont : [`CAMERA.md`](CAMERA.md)
- Matériel : [`HARDWARE.md`](HARDWARE.md) · Sécurité : [`SECURITY.md`](SECURITY.md) · Wi-Fi/IP : [`WIFI_ET_IP.md`](WIFI_ET_IP.md)

## 1. Objectif

Comprendre les failles d'une caméra, couper le cloud Tuya et obtenir un flux vidéo limité au réseau local, fluide en 1920×1080.

## 2. Diagnostic : pourquoi `anyka_ipc` redémarrait en boucle

La caméra ne redémarrait pas : le système restait allumé (un seul `hack.sh` dans les journaux) mais le processus
`anyka_ipc` plantait puis était relancé toutes les ~14 s (239 relances dans `logs/`, environ 2 900 dans `logs1/`).
Chaque cycle contenait :

```
Encoder failed : Channel creation failed, processing power of the available cores insufficient,
perhaps the fps or resolution is oversize, please reduce the fps or resolution
ak_venc_open: AKV_Encoder_Open failed!   ->   dev0 open venc failed
```

puis `camera start times N read failed` (compteur jusqu'à 224) et `Timeout waiting for RTSP` côté toolkit.

**Cause** : le `_ht_hw_settings.ini` du toolkit, monté par-dessus celui du firmware, demandait **2304×1296 en H.265** (réglages d'un
autre modèle, capteur SC3332). Cette caméra a un capteur SC2336 en **1920×1080** : l'encodeur refusait le canal.
**Correction** : `main_width = 1920`, `main_height = 1080`. Ce changement a suffi à stabiliser `anyka_ipc` ; le codec H.265 a été conservé.
Voir [`CAMERA.md`](CAMERA.md).

## 3. Modifications du dépôt

| Fichier | Changement | Raison |
|---|---|---|
| `sd_card/custom/configs/_ht_hw_settings.ini` | `main_width/height` 2304×1296 → **1920×1080** ; `main_fps` 15 → **20** ; `main_goplen` 30 → **40** ; `sub_fps` 15 → **20** | corrige la boucle ; fps voulus (GOP = 2 s à 20 fps) |
| `sd_card/custom/configs/_ht_sw_settings.ini` | `bool_led_enable` 1 → **0** | réglage officiel de la LED (insuffisant seul, voir §5) |
| `sd_card/custom/scripts/state_led.sh` | **nouveau** : force `trigger=none` et `brightness=0` sur `fill_light_led` | `anyka_ipc` appelle `/usr/sbin/state_led.sh on\|off\|blink` ; ce script le remplace (§5) |
| `sd_card/hack.sh` | **+6 lignes** dans `apply_overrides()` : montage de `state_led.sh`, de `isp_sc2336_fps20.conf` et de `anyka_ipc_fps20b` | active les trois correctifs ; chaque montage est **conditionné à l'existence du fichier** |
| `tools/patch_anyka_ipc.py` | **nouveau** : patch du binaire `anyka_ipc` | §6 |
| `tools/patch_isp_conf.py` | **nouveau** : patch du fichier ISP du capteur | §6 |
| `docs/*` | **nouveau** : ce journal et les documents liés | documentation |
| `README.md` | encart « Notes sur ce fork » | renvoi vers `docs/` |
| `.gitignore` | ignore les fichiers dérivés du firmware | ne pas publier de code du fabricant |

**Non modifiés** : `wifi.conf`, `_ak39_factory.ini`, `hack.conf`, `hosts`, `ptz.conf`, `onvif_simple_server.conf`, et les scripts autres que `hack.sh`.
`hack.sh` garde ses commentaires d'origine ; ses 6 lignes ajoutées sont en français.

**Non publiés** (volontairement) : le firmware du fabricant sous toute forme (`anyka_ipc` original ou patché, fichier ISP), les dumps de flash,
et toute donnée personnelle (SSID, mot de passe, adresses IP/MAC, UUID Tuya).

## 4. Problème de fluidité : ce qui a été trouvé

Mesures faites avec `ffprobe`/`ffmpeg` sur le flux `rtsp://<IP_CAMERA>:554/main_ch`, une seule connexion à la fois, 20 s par mesure.

| Étape | fps réels | Écart-type | Trous | Débit |
|---|---|---|---|---|
| Config amont (2304×1296 H.265) | — | — | — | boucle de redémarrage |
| 1920×1080 H.265, 15 fps demandés (UDP) | 14,29 | 2 ms | 0 > 120 ms | 936 kbit/s |
| Capteur à 20 fps, sortie bridée (2 mesures) | 15,14 / 15,18 | **24 ms** | **42 / 51** > 100 ms | 923 / 599 kbit/s |
| **Capteur 20 fps + filtre retiré (UDP)** | **20,01** | **3 ms** | **0** > 80 ms | 1 165 kbit/s |
| Même chose en TCP | 12,6 | 417 ms | 4, dont un de 6,6 s | 686 kbit/s |

Trois causes se cumulaient :

1. **Le capteur plafonnait à 14 fps** : `frame_rate=14` et `max_exp_time=2385` dans `isp_sc2336_mipi_2lane_h3b.conf`.
2. **Le canal d'entrée vidéo était figé à 15 fps** : `ak_vi_set_chn_attr` reçoit `frame_rate=15` écrit en dur dans `anyka_ipc` pour les deux
   flux (ni `main_fps` ni `sub_fps` ne le changent).
3. **Un filtre jetait les images en trop** : dans `app_video_vi_stream_do_venc`, un saut `bgt` renvoie vers le chemin « jeter l'image »
   (`Venc frame rate if over the limitation… should drop!`) quand une image arrive moins de `int` ms après la précédente. `int = 1000 × hi16 / lo16`
   vaut 66 ms (= 15 fps) et ne vient pas du `.ini`. Avec un capteur à 20 fps, une image sur quatre était jetée : cadence irrégulière
   (σ 24 ms, trous). Le compteur `total_frame` du journal le confirmait : 1000 images en 66 s avant, **en 50 s après**.

Les erreurs de décodage `Error constructing the frame RPS` n'apparaissent qu'**à la connexion** (toutes dans la même fraction de seconde,
aucune après 5 s) : le décodeur attend la première image clé.

**Utilisez UDP** (`-rtsp_transport udp`). En TCP la caméra est irrégulière, avec des trous de plusieurs secondes.
Un seul client RTSP à la fois : la caméra n'a qu'un cœur et ~3 Mo de RAM libre ; une lecture en parallèle fausse les mesures.

## 5. LED qui clignote

- Le clignotement (`gpio_led_set value=1/0`, ~4 par seconde, dans `dmesg`) vient du pilote noyau `ak_leds`, piloté par `state_led.sh blink` appelé par `anyka_ipc`.
- `bool_led_enable = 0` ne l'arrête pas : le firmware signale ainsi l'absence de cloud.
- Test **non persistant** : `rmmod ak_leds` (code retour 0) a stoppé le clignotement sans planter `anyka_ipc`. Retour : `insmod /usr/modules/ak_leds.ko` ou redémarrage.
- Solution retenue, **persistante** : `hack.sh` monte `custom/scripts/state_led.sh` sur `/usr/sbin/state_led.sh` puis l'exécute avec `off`.
- `main.sh` appelle `state_led.sh on` au démarrage, avant `hack.sh` : la LED peut donc rester allumée quelques secondes (non vérifié).
- Il existe aussi un chemin PWM pour la LED (`ht_pwm_led_ctrl`) : non observé ici.

## 6. Les deux patchs binaires (`tools/`)

Les fichiers du fabricant ne sont pas distribués. Les scripts s'appliquent à **votre** copie, **refusent** de s'exécuter si le MD5 ou les octets
attendus diffèrent, et n'écrivent qu'un fichier de sortie.

| Script | Fichier d'entrée (MD5 attendu) | Sortie sur la SD |
|---|---|---|
| `tools/patch_isp_conf.py` | `isp_sc2336_mipi_2lane_h3b.conf` (`e5cd9fc1f2e88d2f99aa4041cd3bb646`) | `sd_card/custom/configs/isp_sc2336_fps20.conf` (`754d46cf9a3abbfca980c367d1268145`) |
| `tools/patch_anyka_ipc.py` | `anyka_ipc` (`3d8cad8080cae1f8bd6bca7331e2c651`) | `sd_card/custom/bin/anyka_ipc_fps20b` (`27ee718fccc520b9989dddeb9ffa124a`) |

- **ISP** : 10 mots de 32 bits modifiés (5 dans chacune des deux copies du bloc, `0x8cb4` et `0x1b75c`) : `frame_rate` 14 → 20 et `max_exp_time` 2385 → 1688.
- **`anyka_ipc`** : 3 instructions ARM (adresses virtuelles ; offset fichier = adresse − `0x10000`) :
  `0xb54e0` et `0xb55c0` : `mov r2,#15` → `mov r2,#20` ; `0xca9f4` : `bgt #0xcabe4` → `nop`.
- Les MD5 de sortie ci-dessus sont ceux des fichiers déployés et testés : les scripts les reproduisent **octet pour octet**.
- Seule la valeur **20 fps** est testée. Un autre `--fps` extrapole `max_exp_time` (avertissement affiché) et reste à valider.

## 7. Déploiement sur cette caméra

1. Avoir la config du fork sur la SD (`sd_card/` à la racine de la carte) et la caméra stable en 1920×1080.
2. Récupérer les **fichiers d'origine** depuis la partition APP (et non `/usr/bin/anyka_ipc` : une fois le patch actif, il serait déjà patché) :
   ```
   # PC          : nc -l -p 9000 > mtd7_APP.bin
   # caméra      : nc <IP_DU_PC> 9000 < /dev/mtd7ro
   unsquashfs -d app mtd7_APP.bin        # puis : app/bin/anyka_ipc  et  app/local/isp_sc2336_mipi_2lane_h3b.conf
   md5sum app/bin/anyka_ipc app/local/isp_sc2336_mipi_2lane_h3b.conf
   ```
3. Générer les deux fichiers :
   ```
   python3 tools/patch_isp_conf.py  app/local/isp_sc2336_mipi_2lane_h3b.conf  sd_card/custom/configs/isp_sc2336_fps20.conf
   python3 tools/patch_anyka_ipc.py app/bin/anyka_ipc                         sd_card/custom/bin/anyka_ipc_fps20b
   ```
4. Copier sur la SD, **sans écraser** votre `wifi.conf` ni votre `_ak39_factory.ini` renseignés.
5. **Redémarrer** la caméra avec `reboot`.
6. Vérifier (shell du port 24) :
   ```
   mount | grep -E 'state_led|isp_sc|usr/bin/anyka_ipc'
   md5sum /usr/bin/anyka_ipc                                   # 27ee718f…
   grep -E 'chn\[[01]\] frame_rate|params: w=' /tmp/sd/logs/anyka.*.log | tail -4     # frame_rate[20]
   grep 'int\[' /var/log/messages | tail -2                    # cur_int[50]
   ```
   Si `cur_int` vaut 66 et le compteur avance de 1000 images en ~66 s, le patch `anyka_ipc` n'est pas actif.

**Ne pas tuer `anyka_ipc` pour recharger** (`kill -TERM`) : cela a provoqué un redémarrage complet de la caméra, et les montages `mount --bind` faits à la main ont été perdus.
Utilisez `reboot`.

## 8. Retour arrière

- Retirer de la SD `custom/bin/anyka_ipc_fps20b`, `custom/configs/isp_sc2336_fps20.conf` et (si voulu) `custom/scripts/state_led.sh`, puis redémarrer :
  les montages étant conditionnés à l'existence des fichiers, `hack.sh` n'a pas besoin d'être modifié.
- Si la caméra boucle : le shell du port 24 reste disponible (`telnetd` est lancé par `hack.sh`, indépendamment d'`anyka_ipc`), ou sortir la SD et
  supprimer ces fichiers depuis un PC (FAT).
- Pour revenir aux 15 fps : remettre `main_fps = 15`, `main_goplen = 30`, `sub_fps = 15` dans `_ht_hw_settings.ini` **et** retirer les deux fichiers patchés
  (sinon capteur et sortie ne sont plus cohérents).
- Sauvegardes faites avant intervention, **hors dépôt** : `_ht_hw_settings.ini`, `_ht_sw_settings.ini`, `hack.sh`, fichier ISP d'origine ; sur la SD : `*.bak`, `hack.sh.v2`.

## 9. Journal chronologique des actions (2026-11-09)

Légende : **L** = lecture seule · **E** = écriture sur la caméra ou la SD.

| # | Action | Type |
|---|---|---|
| 1 | Analyse des journaux de la SD (`logs/`, `logs1/`, `hack.sh`, configs) : cause de la boucle trouvée (§2). | L |
| 2 | `nmap` + telnet : ports 23 (mot de passe), 24 (sans login, toolkit), 554 (RTSP), 8090. | L |
| 3 | Audit de sécurité interne par le shell du port 24 ([`SECURITY.md`](SECURITY.md)). | L |
| 4 | LED : lecture de `/sys/class/leds`, `dmesg` ; puis `rmmod ak_leds` en direct (test non persistant, accord préalable). | L puis E (non persistant) |
| 5 | Récupération de `anyka_ipc` par `nc` et analyse statique (désassemblage local, chaînes, imports). | L |
| 6 | Vérification de l'absence de SSH. | L |
| 7 | Audit matériel et **dump de la flash** par `nc` depuis `/dev/mtd*ro` ([`HARDWARE.md`](HARDWARE.md)). | L |
| 8 | Sauvegardes : `_ht_hw_settings.ini`, `_ht_sw_settings.ini`, `hack.sh`, ISP d'origine, `*.bak` sur la SD. | E (copies) |
| 9 | Écriture sur la SD : `isp_sc2336_fps20.conf`, `main_fps`/`main_goplen`/`sub_fps`, `state_led.sh`, `hack.sh`. | E |
| 10 | Redémarrages : **1 × `kill -TERM anyka_ipc`** (a provoqué un redémarrage complet d'environ 2 min), puis **4 × `reboot`**. | E |
| 11 | Mesures du flux (§4) et analyse de `app_video_vi_stream_do_venc` ; génération et déploiement du binaire patché. | L puis E |
| 12 | Fork du dépôt, ajout de `tools/` et `docs/`. | — |
| 13 | Le 2026-11-10 : `ip`/`netmask`/`gateway` vidés dans `_ak39_factory.ini` (sauvegarde `.bak`) pour passer en DHCP, puis `reboot`. | E |
| 14 | Lecture des journaux de la SD : avec `ip` vide, la caméra ne se connecte pas (voir [`WIFI_ET_IP.md`](WIFI_ET_IP.md) §3). | L |
| 15 | Option retenue : `_ak39_factory.ini` rétabli à l'identique de l'original (comparé octet pour octet) et `DHCP=0` dans `wifi.conf` (sauvegarde `.bak`). | E (SD) |
| 16 | Redémarrage de la caméra par l'utilisateur, puis vérification en lecture seule : connectée à l'IP fixe, pas de `udhcpc`, patchs vidéo et LED toujours actifs (`cur_int[50]`, binaire `27ee718f…`). | L |
| 17 | PTZ : lecture du pilote `ak_motor.ko` (local), essais de `ptzctl` en RAM puis mesure du déplacement sur l'image (corrélation de phase). | L puis E (moteurs, non persistant) |
| 18 | PTZ : `ptzctl` recompilé (ARM statique) et déployé sur la SD avec `ptz.conf` ; sauvegardes `ptzctl.orig` et `ptz.conf.orig` sur la SD. | E (SD) |
| 19 | PTZ : tilt validé (seuil 489 pas par ordre), valeurs `TILT_*` ajoutées dans `ptz.conf` (sauvegarde `ptz.conf.pan_only` sur la SD). | E (SD) |
| 20 | ONVIF : `onvif_simple_server` et `mini_cgi_httpd` compilés (ARM statique), déployés sur la SD avec sauvegardes `.orig` de `onvif_simple_server.conf` et `onvif.sh` ; redémarrage pour vérifier. | E (SD) |

## 10. Erreurs et leçons

- Plusieurs écritures sur la caméra (fichier ISP, réglages fps, premier redémarrage) ont été faites **avant d'avoir été expliquées** ; elles avaient été sauvegardées au préalable.
- Le conseil initial « utilisez TCP » était **faux** : sur cette caméra le TCP est le mode irrégulier.
- Un `kill -TERM` pour recharger a causé un redémarrage complet et la perte des montages manuels.
- Des mesures ont été faussées par un second client RTSP (lecteur ouvert en parallèle) ; elles ont été refaites seul.
- La première analyse du mot de passe Wi-Fi dans le dépôt était invalide (valeur vide cherchée) ; elle a été refaite correctement avant publication.
- L'adresse IP de la caméra a changé entre deux démarrages : voir [`WIFI_ET_IP.md`](WIFI_ET_IP.md).
- La première version de `WIFI_ET_IP.md` **conseillait** de vider `ip`/`netmask`/`gateway` (DHCP partout) en précisant que ce n'était pas retesté :
  l'essai a montré que la caméra ne se connectait pas ainsi. Le conseil a été remplacé par IP fixe + `DHCP=0`.
- Mes comptes sur les journaux étaient faux : au démarrage l'horloge repart de l'an 2000 et plusieurs démarrages **s'ajoutent au même fichier**
  (voir `WIFI_ET_IP.md` §5). J'ai d'abord pris un ancien fichier pour le démarrage en cours ; refait en lisant la fin du bon fichier.

## 12. PTZ (pan et tilt validés)

**Constat.** `ptzctl` d'origine (échelle 4096/2230 pas, vitesse 640–800, 8 répétitions) est accepté par le pilote (`rc=0`) mais la caméra ne bouge pas.

**Causes trouvées.**
1. **Vitesse** : `anyka_ipc` envoie `set motor speed:60` (journal `ht_ptz.c`). Les valeurs 160–640 que nous envoyions n'ont produit aucun mouvement (le pilote déclare `speed-step:200`, `total-steps:2000`, `boundary-steps:50`).
2. **Fermeture trop rapide** : `ak_motor_release()` arrête le moteur et `ptzctl` ferme le descripteur juste après l'ordre MOVE, ce qui coupe le déplacement après quelques pas (hypothèse cohérente avec le code du pilote et avec les mesures).

**Changements.**
- `ptzlib/src/ptz_motor.c` : après un ordre MOVE réussi, `ptz_issue_motor()` attend `|pas| × rép × 4 ms` avant de fermer (variable d'environnement `PTZ_HOLD_US_PER_STEP`, 4000 par défaut).
- `ptz.conf` : `PAN_TOTAL_STEPS=2000`, `PAN_STEP_REPEAT=1`, `PAN_SPEED_STEP=75` (valeur réellement envoyée : 60).
- `sd_card/custom/bin/ptzctl` : recompilé avec `arm-linux-gnueabi-gcc -O2 -static`, puis `strip` (md5 `83af89f6…`).

**Mesure (décalage de l'image, sur 960 px de large).**

| Ordre | Décalage |
|---|---|
| 2 × 400 pas à droite (essai en RAM) | −195 px |
| 2 × 400 pas à gauche | +194 px |
| 1 ordre pan droite / gauche (config de la SD) | −82 px / +82 px |

**Tilt : validé par balayage mesuré** (10 ordres haut, 10 bas, une photo par ordre, déplacement mesuré par corrélation de phase).

- Chaque ordre doit faire **au moins 489 pas** : en dessous (100 à 204), le jeu mécanique absorbe tout et rien ne bouge. Le seuil de 489 est celui qu'utilise `anyka_ipc` (`run down step:489`).
- Jeu mort d'environ 1 à 2 ordres à chaque changement de sens (ordre 1 : ~0 à 15 px ; ordre 2 : partiel), puis ~130 px par ordre, symétrique haut/bas.
- Valeurs retenues dans `ptz.conf` : `TILT_*_STEP_MULT=3`, `TILT_*_STEP_REPEAT=1`, `TILT_STEP_ABS_MAX=489`, `TILT_SPEED_STEP=75`.
- Pan : ~100 à 110 px par ordre dans les deux sens, jeu mort d'un ordre au changement de sens. Les valeurs aberrantes d'un premier balayage venaient de la mesure (image floue), pas du moteur.
- Un seul ordre ne reprend pas le jeu : pour changer de sens, envoyer un ordre de plus ou deux.

**Incident.** En envoyant plusieurs « bas » de suite avec de petits pas, l'objectif s'est retrouvé tourné vers la base (image grise) ; la tête a été remise à la main. Toujours mesurer après chaque ordre.


**Limites de course (mesure interrompue, marges abandonnées).** Butée droite atteinte vers 27 ordres de ~400 pas depuis le zéro (reboot). Les mesures gauche, haut et bas du protocole automatique ont donné 0 : elles ne sont pas fiables (capture ou départ déjà en butée) et ne sont pas retenues. Les marges seront gérées plus tard via ONVIF.

**À faire (demande de l'utilisateur, non appliqué).** Pan : ordres de ~335 pas (≈ 5°), 36 ordres à droite et 36 à gauche (≈ 180° de chaque côté), moteurs plus rapides (vitesse « 8 », à définir et à tester : seule la valeur 60 avait un effet sur le pilote).

## 13. ONVIF (pile légère)

Le firmware n'a pas `lighttpd`, `wsd_simple_server` ni `onvif_notify_server`. Solution : `onvif_simple_server` (compilé en ARM statique) lancé par un mini serveur HTTP/CGI écrit pour ce dépôt (`mini_cgi_httpd`, port 8080), démarré par `onvif.sh`. Détails, limites et retour arrière : [ONVIF.md](ONVIF.md).

- Authentification ONVIF obligatoire (identifiants à définir par l'utilisateur, non publiés) ; sans identifiants ou avec un mauvais mot de passe, la caméra refuse.
- Un `ContinuousMove` = un déplacement de taille fixe ; pas de position absolue fiable.
- Vérifié après redémarrage : service relancé seul, flux 1080p H.265 20 fps inchangé.
- Limites de course et marges : abandonnées pour l'instant (butée droite vers 27 ordres ; mesures gauche/haut/bas non fiables), à gérer via ONVIF.

## 11. Non fait, non testé

- **PTZ** : pan et tilt validés (§12). ONVIF : voir §13.
- **Débit** : toujours `main_target_kbps = 800` / `main_max_kbps = 1600` ; augmentation envisagée, non réalisée.
- `sensor_type_def` reste à 17 (usine : 20) ; H.265 conservé (usine : H.264) ; fps autres que 20.
- Aucune vérification que la caméra ne contacte plus le cloud (filtrage Internet à faire sur le routeur).
- Configuration Wi-Fi « IP fixe + `DHCP=0` » : testée sur un démarrage (~4 min stable), pas sur la durée.
- Pas de recompilation de firmware complet : les sources du noyau ne sont pas disponibles.
