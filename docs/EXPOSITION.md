# Tableau d'exposition : fonctions, ONVIF, MQTT

Légende : ✅ fonctionne ; `*` pourrait l'être (service ONVIF ou topic possible, non fait) ; `—` non applicable ou aucun service ONVIF standard ; `exclu` volontairement (SD).
¹ = exposé mais **non testé sur caméra** (action risquée ou matériel absent). Source : `TOPICS.md`, `ONVIF_MAPPING.md`, `CHANGES.md`.

## A. Fonctions livrées avant le désassemblage global

| Fonction | Définition | ONVIF | MQTT |
|---|---|---|---|
| Flux vidéo RTSP (H.265, 720p/1080p, 20 i/s) | Flux principal local, sans cloud | ✅ Media | —  (RTSP direct) |
| PTZ continu (haut/bas/gauche/droite/stop) | Déplacement par pas fixe via `ptz_move` | ✅ PTZ | ✅¹ DP 119/116 (natif firmware) |
| PTZ absolu normalisé (-1..1) et position | Aller à une position, lire la position suivie par calcul | ✅ PTZ | ✅¹ `ptz_absolute`, `ptz_position` |
| Préréglages PTZ (créer, aller à, supprimer) et position maison | Positions mémorisées du toolkit (`ptz_presets.sh`) | ✅ PTZ | ✅¹ `ptz_preset_*`, `ptz_presets` |
| Recentrage au démarrage / calibrage PTZ | `PTZ_HOME_ON_BOOT`, `ptz_calibrate.sh` | — | ✅¹ `ptz_home`, `ptz_calibrate` |
| Mode jour/nuit/auto (alias `night`) | Filtre IR et LED infrarouges | ✅ Imaging IrCut | ✅ `night` |
| Mode privé (alias `privacy`) | Envoie la caméra sur le preset privé (`privacy.sh`) | ✅ DeviceIO relais 0 | ✅ `privacy` |
| Détection de mouvement locale | `md_inject.so` publie le mouvement | * Events MotionAlarm (`onvif_notify_server` absent) | ✅ `motion` |
| Redémarrage de la caméra | `reboot` propre | ✅ Device SystemReboot | ✅¹ `system_reboot` |
| Haut-parleur par le LAN / annonce / sirène | Audio G.711 µ-law par UDP (`talk_inject.so`, `send_audio.py`) | — (pas de sortie audio ONVIF) | ✅¹ `speaker_siren`, `speaker_play` |
| Micro (audio AAC dans le RTSP) | Son entrant | ✅ Media (audio) | — |
| LED d'état (état_led) | Voyant de la caméra (`state_led.sh`) | — | ✅ `light_onoff` (DP 101) |
| Heure / NTP / fuseau | Horloge locale, anti-falsification | Device SetSystemDateAndTime (*) | — |
| Syslog distant, rotation des journaux | Journaux envoyés au LAN, purge au démarrage | — | — |
| Wi-Fi / IP fixe / installeur | `install.py`, `WIFI_ET_IP.md` | — | — |

## B. DP du firmware (`anyka_ipc`, 67 identifiés, 61 avec fonction)

| Fonction (DP) | Définition | ONVIF | MQTT |
|---|---|---|---|
| `light_onoff` (101) | Voyant (LED témoin) allumé/éteint | *  DeviceIO relais | ✅ |
| `flip_onoff` (103) | Retourne l'image à 180° (flip et miroir inséparables) | — | ✅ |
| `watermark_onoff` (104) | Affiche/masque l'horodatage incrusté (OSD) | — | ✅ |
| `sleep_mode` (105) | Mode veille (routé vers `privacy` : le natif plante sans cloud) | *  DeviceIO relais | ✅ |
| `alarm_sensitivity` (106) | Sensibilité de l'alarme de mouvement (0 bas, 1 moyen, 2 haut) | — | ✅ |
| `night_mode` (108) | Mode jour/nuit (0 auto, 1 jour, 2 nuit) | ✅ Imaging IrCut (alias `night`) | ✅ |
| `sd_storage` (109) | Capacité, utilisé et libre de la carte SD | — | ✅ JSON en Kio |
| `sd_status` (110) | Code d'état de la carte SD | — | ✅ |
| `format_sd_card` (111) | Formate la SD (destructif) | — | exclu |
| `unmount_sd_card` (112) | Démonte la SD | — | exclu |
| `ptz_stop_move` (116) | Arrête le mouvement PTZ | ✅ PTZ Stop | ✅ |
| `sd_format_status` (117) | Progression du formatage SD | — | ✅ |
| `ptz_start_move` (119) | Démarre un mouvement (0 haut, 2 droite, 4 bas, 6 gauche) | ✅ PTZ ContinuousMove | ✅ |
| `alarm_siren_onoff` (120) | Sirène d'alarme activée/désactivée | *  DeviceIO relais | ✅ |
| `night_vision_mode` (124) | Type de vision nocturne (0/1) | *  Imaging | ✅ |
| `ptz_check` (132) | Auto-calibrage du PTZ (butées) | — | ✅¹ |
| `alarm_function_onoff` (134) | Alarme générale activée/désactivée | *  DeviceIO relais | ✅ |
| `(sans fonction)` (136) | Gestionnaire sans `IPC_APP_*` (adresse 0x7b588) | — | — |
| `blub_onoff` (138) | Ampoule/projecteur on/off (matériel probablement absent) | *  DeviceIO relais | ✅¹ |
| `sound_detect_onoff` (139) | Détection de son activée/désactivée | * Events (réglage : —) | ✅ |
| `sound_sensitivity` (140) | Sensibilité de détection de son (0/1) | — | ✅ |
| `(sans fonction)` (141) | Gestionnaire partagé 141–143 (0x7cb40) | — | — |
| `(sans fonction)` (142) | Idem | — | — |
| `(sans fonction)` (143) | Idem | — | — |
| `battery_percent` (145) | Pourcentage de batterie (constante 80 ici) | — | ✅ |
| `power_mode` (146) | Mode d'alimentation (constante 1) | — | ✅ |
| `(sans fonction)` (147) | Gestionnaire sans `IPC_APP_*` (0x7b72c) | — | — |
| `sd_record_onoff` (150) | Enregistrement SD du firmware | — | exclu (SD système) |
| `sd_record_mode` (151) | Mode d'enregistrement SD (0/1) | — | exclu (SD système) |
| `pir_detect_switch` (152) | Détecteur PIR (0 à 3, capteur absent) | — | ✅ |
| `floodlight_lightness` (158) | Luminosité du projecteur (1–100, matériel absent) | — | ✅ |
| `siren_onoff` (159) | Sirène (MP3 en boucle du firmware) | *  DeviceIO relais | ✅¹ |
| `ipc_lullaby_volume` (160) | Volume de la berceuse (0–10) | — | ❌ (remplacé par `lullaby/set`) |
| `track_enable` (161) | Suivi automatique du sujet activé/désactivé | — | ✅ |
| `system_reboot` (162) | Redémarre la caméra | (Device SystemReboot : voie interne, ne passe pas par ce DP) | ✅¹ |
| `cry_detect_onoff` (167) | Détection de pleurs | * Events (réglage : —) | ✅ |
| `alarm_zone_onoff` (168) | Zone d'alarme activée | — | ✅ |
| `alarm_zone_draw` (169) | Dessin de la zone d'alarme (JSON x, y, xlen, ylen) | — | ✅ |
| `human_filter` (170) | Filtre « humain » de la détection | * Events (réglage : —) | ✅ |
| `flight_warn_onoff` (172) | Avertissement de vol (réglage seulement, ne fait que journaliser) | — | ✅ |
| `flight_warn_time` (173) | Durée de l'avertissement (0–3600) | — | ✅ |
| `patrol_switch` (174) | Patrouille activée/désactivée | * PTZ PresetTour (non implémenté) | ✅¹ |
| `patrol_mode` (175) | Mode de patrouille (0/1) | — | ✅ |
| `patrol_tmode` (176) | Mode de temps de patrouille (journalise seulement) | — | ✅ |
| `patrol_time` (177) | Durée de patrouille (renvoie l'argument) | — | ✅ |
| `preset` (178) | Création/gestion de préréglage firmware (JSON) | ✅ PTZ presets (versions toolkit) | ✅¹ |
| `patrol_state` (179) | État de la patrouille (lecture) | — | ✅ |
| `basic_anti_flicker` (188) | Anti-scintillement 50/60 Hz (0/1/2) | — | ✅ |
| `link_move` (190) | Mouvement lié (journalise seulement) | — | ✅ |
| `object_outline_onoff` (198) | Contour des objets détectés | — | ✅ |
| `link_pos` (199) | Position liée (journalise seulement) | — | ✅ |
| `ipc_lullaby_mode` (232) | Mode de la berceuse (0/1/2) | — | ❌ (remplacé par `lullaby/set`) |
| `(sans fonction)` (233) | Gestionnaire sans `IPC_APP_*` (0x7c940) | — | — |
| `ipc_lullaby_control` (234) | Lecture/pause de la berceuse | — | ❌ (remplacé par `lullaby/set`) |
| `ipc_lullaby_list` (235) | Choix du morceau de la berceuse | — | ❌ (remplacé par `lullaby/set`) |
| `ipc_lullaby_command` (236) | Commande de la berceuse | — | ❌ (remplacé par `lullaby/set`) |
| `temp_report_c` (238) | Température en °C (0 sans capteur) | — | ✅ |
| `temp_report_f` (239) | Température en °F (320 sans capteur) | — | ✅ |
| `temp_unit_select` (240) | Unité de température (0/1) | — | ✅ |
| `temp_max_onoff` (241) | Alerte température max. | — | ✅ |
| `temp_min_onoff` (242) | Alerte température min. | — | ✅ |
| `temp_max_cvalue` (243) | Seuil max. en °C | — | ✅ |
| `temp_min_cvalue` (244) | Seuil min. en °C | — | ✅ |
| `temp_max_fvalue` (245) | Seuil max. en °F | — | ✅ |
| `temp_min_fvalue` (246) | Seuil min. en °F | — | ✅ |
| `audible_alarm_onoff` (247) | Alarme sonore | *  DeviceIO relais | ✅ |
| `door_control` (248) | Commande de porte (0–6 ; 6 appelle `ht_cardoor_ctrl`) | — | ✅¹ |

**Bilan MQTT** : 56 DP sur 61 ✅ (dont 109) ; 111, 112, 150, 151 exclus (SD). Côté ONVIF, aujourd'hui : PTZ, Imaging IrCut, relais privé, reboot et profil Media. Le canal `/tmp/md_cmd` peut porter tous les DP, mais ONVIF n'a pas de service standard pour la plupart des réglages (`—`).
