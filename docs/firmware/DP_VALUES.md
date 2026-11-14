# Valeurs et signatures des DP (lots de décompilation)

Rédigé **à la main** par les lots de décompilation (#54 à #58), un tableau par lot. Contrairement aux autres fichiers de ce dossier, il n'est pas généré : c'est la **source des confirmations** lue par `tools/gateway/gen_gateway.py`.

Règle (epic #51) : chaque ligne s'appuie sur le désassemblage (adresse ou fonction citée dans « Preuve »). Une ligne sans adresse ne peut pas être exposée : le générateur refuse.

## Format d'une ligne (lu par le générateur)

`| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |`

| Colonne | Contenu attendu |
|---|---|
| DP | numéro (ex. `103`) |
| Fonction | nom du topic, comme dans `docs/TOPICS.md` (informatif) |
| Type | `bool`, `enum`, `int`, `action`, `sensor` ou `json` |
| Valeurs | `bool` : texte libre (charges `ON`/`OFF`) ; `enum` : **chaque charge utile admise entre accents graves** (ex. `` `0` `1` `2` ``) ; `int` : **deux nombres entre accents graves**, min puis max (ex. `` `0` `100` ``) ; autres : texte libre |
| Écriture | passage de la valeur à `IPC_APP_set_*` (registre r0) : `int`, `str` (pointeur de chaîne) ou `aucun` |
| Lecture | retour de `IPC_APP_get_*` (r0) : `int`, `str` (pointeur de chaîne), `out` (la fonction écrit un entier via un pointeur passé en r0) ou `aucun` |
| Effets de bord / persistance | clé de configuration écrite (`ht_tuya_config_set_*`), redémarrage, etc. |
| Preuve | adresse(s) / fonction(s) lues (ex. `0x74a78 IPC_APP_set_flip_onoff : ldr ... bl ht_tuya_config_set_int`) |
| Exposé | `oui` = la passerelle pilote ce DP ; `non` = documenté seulement (matériel absent, effet dangereux, non testé) |

Après modification : `python3 tools/gateway/gen_gateway.py` régénère `tools/gateway/dp_table.h` et `docs/TOPICS.md`, puis `tools/motion/build.sh`.

Conversion faite par la passerelle (`tools/gateway/gateway.c`) :

- `bool` : `ON`/`on` → 1, `OFF`/`off` → 0 ; passé en entier (`int`) ou en chaîne `"1"`/`"0"` (`str`). État publié : `ON`/`OFF`.
- `enum` : la charge utile doit figurer dans la liste ; passée telle quelle (`str`) ou convertie en entier (`int`, valeurs numériques uniquement).
- `int` : entier décimal dans [min, max] ; passé en entier ou en chaîne.
- `action` : `aucun` = appel sans argument ; sinon comme `int`/`str`. Pas d'état publié.
- `json` : texte libre (≤ 159 octets) transmis tel quel à la fonction (`str`) ; le firmware valide le JSON. Lecture renvoyée telle quelle.
- `action` : si la colonne Valeurs liste des valeurs entre accents graves, seules celles-ci sont acceptées.
- `sensor` : lu au démarrage et toutes les 5 minutes, publié retained.

## Lot 1 : image et système (#54)

Lu dans le pseudo-C (`tools/firmware/Decompile.java`), conversions par `docs/firmware/DECOMPILE.md`.

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 108 | night_mode | enum | `0` `1` `2` | str | str | config `enum_night_mode` puis `ht_tuya_func_set_night_mode` → `ht_night_mode_set_irled_mode` (0 auto, 1 jour, 2 nuit ; alias `night`) | 0x74bf0 IPC_APP_set_night_mode; 0x74ccc IPC_APP_get_night_mode : atoi, refus hors 0 à 2, défaut 0 | oui |
| 124 | night_vision_mode | enum | `0` `1` | str | str | config `enum_night_vision_mode` (stockée 1 ou 2) puis `ht_tuya_func_set_night_vision_mode` → `ht_night_mode_set` ; lecture renvoie 0 ou 1 | 0x74dc0 IPC_APP_set_night_vision_mode; 0x74eac IPC_APP_get_night_vision_mode | oui |
| 103 | flip_onoff | bool | ON / OFF | int | int | config `bool_rotate180` puis `ht_tuya_func_set_flip_mirror(v)` | 0x74a78 IPC_APP_set_flip_onoff; 0x74b08 IPC_APP_get_flip_onoff : set écrit la clé puis applique le retournement si l'écriture réussit | oui |
| 104 | watermark_onoff | bool | ON / OFF | int | int | config `?` puis `ht_tuya_func_set_osd(v)` | 0x74b34 IPC_APP_set_watermark_onoff; 0x74bc4 IPC_APP_get_watermark_onoff : set écrit la clé puis applique l'OSD; get défaut 1 | oui |
| 188 | basic_anti_flicker | enum | `0` `1` `2` | str | str | config `?` puis `ht_tuya_func_set_basic_anti_flicker` : 2 → 60 Hz, 0 et 1 → 50 Hz | 0x78390 IPC_APP_set_basic_anti_flicker; 0x78488 IPC_APP_get_basic_anti_flicker : set : `atoi(chaîne)`, refus hors 0/1/2; get renvoie un pointeur de chaîne, défaut 2 | oui |
| 198 | object_outline_onoff | bool | ON / OFF | int | int | config `bool_object_outline_switch` seulement (aucun appel d'effet dans cette fonction) | 0x79d20 IPC_APP_set_object_outline_onoff; 0x79d50 IPC_APP_get_object_outline_onoff : effet réel non trouvé : lu ailleurs ? à confirmer | oui |
| 162 | system_reboot | action | charge utile ignorée (envoyer `1`) | int | aucun | `system("reboot -f")` si la valeur est non nulle : **redémarrage immédiat sans arrêt propre** | 0x754c8 IPC_APP_set_system_reboot : set n'appelle rien si param=0 | oui |

## Lot 2 : détection et alarmes (#55)

Lu dans le pseudo-C. Ces DP agissent sur la **détection du firmware**, qui n'est pas notre `md_inject` : laissés non exposés tant qu'un test caméra n'a pas montré l'effet.

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 168 | alarm_zone_onoff | bool | ON / OFF | int | int | `IPC_APP_set_md_area_onoff(v)` : zone de détection du firmware | 0x75620 IPC_APP_set_alarm_zone_onoff; 0x75680 IPC_APP_get_alarm_zone_onoff | oui |
| 169 | alarm_zone_draw | json | JSON `{"num":1,"region0":{"x":0,"y":0,"xlen":100,"ylen":100}}` | str | str | `cJSON_Parse` puis `IPC_APP_set_md_area_rect(x,y,xlen,ylen)` ; lecture renvoie le même JSON | 0x75864 IPC_APP_set_alarm_zone_draw; 0x75d00 IPC_APP_get_alarm_zone_draw : num > 1 refusé | oui |
| 159 | siren_onoff | bool | ON / OFF | int | int | ON lance un thread qui boucle `ht_audio_codec_play_audio_file("/usr/share/8k16_siren.mp3")` ; OFF arrête la lecture. **Bruyant** | 0x7817c IPC_APP_set_siren_onoff; 0x78248 IPC_APP_get_siren_onoff; 0x780e8 thread de sirène | oui |
| 247 | audible_alarm_onoff | bool | ON / OFF | int | int | config `bool_audible_alarm_enable` seulement | 0x7945c IPC_APP_set_audible_alarm_onoff; 0x7948c IPC_APP_get_audible_alarm_onoff | oui |
| 172 | flight_warn_onoff | bool | ON / OFF | int | int | l'écriture ne fait que journaliser (`bool_flight_warn_switch`) | 0x79bdc IPC_APP_set_flight_warn_onoff; 0x79c48 IPC_APP_get_flight_warn_onoff | oui |
| 173 | flight_warn_time | int | `0` `3600` | int | int | config `int_flight_warn_time` (aucun contrôle de plage dans le firmware ; plage choisie par la passerelle) | 0x79ca0 IPC_APP_set_flight_warn_time; 0x79c74 IPC_APP_get_flight_warn_time | oui |
| 106 | alarm_sensitivity | enum | `0` `1` `2` | str | str | config `enum_alarm_sensitivity` puis `ht_tuya_func_set_montion_detect_sensitivity(v)` | 0x752ec IPC_APP_set_alarm_sensitivity; 0x7537c IPC_APP_get_alarm_sensitivity : atoi puis refus hors 0/1/2; défaut 1. Agit sur la détection du firmware, pas sur `md_inject` (non câblé) | oui |
| 134 | alarm_function_onoff | bool | ON / OFF | int | int | config `bool_alarm_enable` puis relit et réapplique `IPC_APP_set_human_filter` | 0x751d0 IPC_APP_set_alarm_function_onoff; 0x75268 IPC_APP_get_alarm_function_onoff : get défaut dépend de la clé `pir_support` | oui |
| 170 | human_filter | bool | ON / OFF | int | int | config `bool_human_filter_enable` puis `ht_tuya_func_set_human_filter(v)` | 0x769c8 IPC_APP_set_human_filter; 0x76a50 IPC_APP_get_human_filter : filtre du firmware, pas celui de `md_inject` | oui |
| 120 | alarm_siren_onoff | bool | ON / OFF | int | int | config `bool_alarm_siren_enable` seulement | 0x7541c IPC_APP_set_alarm_siren_onoff; 0x7549c IPC_APP_get_alarm_siren_onoff : aucun effet direct trouvé | oui |
| 139 | sound_detect_onoff | bool | ON / OFF | int | int | config `bool_sound_detect_enable` puis `ht_tuya_func_set_sound_detect(v)` | 0x74fe4 IPC_APP_set_sound_detect_onoff; 0x75078 IPC_APP_get_sound_detect_onoff | oui |
| 140 | sound_sensitivity | enum | `0` `1` | str | str | config `enum_sound_sensitivity` puis `ht_tuya_func_set_sound_detect_sensitivity(v)` | 0x750a4 IPC_APP_set_sound_sensitivity; 0x7513c IPC_APP_get_sound_sensitivity : atoi, refus hors 0/1; défaut 0 | oui |
| 167 | cry_detect_onoff | bool | ON / OFF | int | int | config `bool_cry_detect_enable` puis `ht_tuya_func_set_cry_detect(v)` | 0x7988c IPC_APP_set_cry_detect_onoff; 0x798d0 IPC_APP_get_cry_detect_onoff : défaut selon la clé `support_cry_detect` | oui |

## Lot 3 : carte SD (#56)

À compléter (DP 109, 110, 111, 112, 117, 150, 151).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 109 | sd_storage | sensor | capacité de la carte SD | aucun | out3 | lecture : `IPC_APP_get_sd_storage(total, used, free)` publiée en JSON `{"total":…,"used":…,"free":…}` | 0x75e00 IPC_APP_get_sd_storage : `capacity(&a,&b)` puis `total=a<<10`, `free=b<<10`, `used=total-free` | oui |
| 112 | unmount_sd_card | action | aucun | aucun | aucun | démontage de la carte SD | 0x76038 IPC_APP_unmount_sd_card | non |
| 117 | sd_format_status | sensor | progression du formatage | aucun | int | lecture du statut de formatage de la SD | 0x76514 IPC_APP_get_sd_format_status | oui |
| 150 | sd_record_onoff | bool | ON / OFF | int | int | config `bool_sd_record_enable` puis `ht_tuya_stream_storage_set_record_onoff(v)` | 0x75e80 IPC_APP_set_sd_record_onoff; 0x75f08 IPC_APP_get_sd_record_onoff : enregistrement du stockage Tuya, pas celui du NVR (#5) ; non exposé : active l'enregistrement sur la SD système | non |
| 151 | sd_record_mode | enum | `0` `1` | int | int | config `enum_sd_record_mode` puis force `record_onoff(1)` | 0x75f34 IPC_APP_set_sd_record_mode; 0x75fdc IPC_APP_get_sd_record_mode : refus hors 0/1; défaut 0 ; non exposé : active l'enregistrement sur la SD système | non |
| 110 | sd_status | sensor | code d'état (entier) | aucun | int | lecture seule : `ht_tuya_stream_storage_tf_status()` | 0x75de8 IPC_APP_get_sd_status : codes non décodés | oui |
| 111 | format_sd_card | action | aucun | aucun | aucun | lance un thread de formatage de la carte SD : **destructif** | 0x76420 IPC_APP_format_sd_card : statut via DP 117 (`get_sd_format_status`) | non |

## Lot 4 : PTZ natif (#57)

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 176 | patrol_tmode | int | `0` `255` | int | aucun | `printf` seulement | 0x77114 IPC_APP_set_patrol_tmode | oui |
| 177 | patrol_time | int | `0` `86400` | int | aucun | renvoie l'argument sans l'enregistrer | 0x77170 IPC_APP_set_patrol_time | oui |
| 178 | preset | json | JSON `{"type":1,"data":{"name":"p1"}}` | str | aucun | enregistre la position courante, une vignette et la déclare au cloud (rapport Tuya inopérant sans cloud) | 0x77190 IPC_APP_set_preset | oui |
| 179 | patrol_state | sensor | état de patrouille (entier) | aucun | out | `IPC_APP_patrol_state(&v)` | 0x78000 IPC_APP_patrol_state | oui |
| 190 | link_move | int | `0` `255` | int | aucun | `printf` seulement | 0x780b4 IPC_APP_set_link_move | oui |
| 199 | link_pos | int | `0` `255` | int | aucun | `printf` seulement | 0x78080 IPC_APP_set_link_pos | oui |
| 119 | ptz_start_move | action | chaîne `0` `2` `4` `6` (0 haut, 2 droite, 4 bas, 6 gauche ; inversées si flip actif) | str | aucun | `ht_ptz_set_cmd` ; directions inversées si `bool_rotate180` | 0x76558 IPC_APP_ptz_start_move : valeurs impaires ignorées | oui |
| 116 | ptz_stop_move | action | aucun | aucun | aucun | `ht_ptz_set_cmd(0,-1)` puis `IPC_APP_ptz_save_pos` | 0x76858 IPC_APP_ptz_stop_move | oui |
| 132 | ptz_check | action | aucun | aucun | aucun | `ht_ptz_calibration()` : recalibrage mécanique | 0x768b0 IPC_APP_ptz_check | oui |
| 161 | track_enable | bool | ON / OFF | int | int | config `bool_track_enable` | 0x76900 IPC_APP_track_enable; 0x7699c IPC_APP_get_track_enable : effet réel non vérifié | oui |
| 175 | patrol_mode | enum | `0` `1` | str | str | config `enum_patrol_mode` | 0x76a7c IPC_APP_set_patrol_mode; 0x76b88 IPC_APP_get_patrol_mode | oui |
| 174 | patrol_switch | bool | ON / OFF | int | aucun | `ht_ptz_set_patrol_mode(0 à 2)` | 0x76c24 IPC_APP_set_patrol_switch | oui |

## Lot 5 : berceuse, température, lumière, PIR, porte (#58)

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 138 | blub_onoff | bool | ON / OFF | int | int | config `bool_bulb_enable` puis `ht_pwm_set_light_led(0 ou luminosité)` : PWM, ampoule probablement absente | 0x78270 IPC_APP_set_blub_onoff; 0x78324 IPC_APP_get_blub_onoff | oui |
| 158 | floodlight_lightness | int | `1` `100` | int | int | config `int_floodlight_lightness` puis `ht_pwm_set_light_led` si l'ampoule est allumée ; défaut 50 | 0x78590 IPC_APP_set_floodlight_lightness; 0x78530 IPC_APP_get_floodlight_lightness | oui |
| 152 | pir_detect_switch | enum | `0` `1` `2` `3` | str | str | config `enum_pir_switch` puis `ht_tuya_func_set_pir_detect(0 ou 1)` et sensibilité : capteur PIR probablement absent | 0x7992c IPC_APP_set_pir_detect_switch; 0x79a38 IPC_APP_get_pir_detect_switch | oui |
| 248 | door_control | enum | `0` `1` `2` `3` `4` `5` `6` | str | str | config `enum_door_ctrl` puis `ht_cardoor_ctrl(v)` : porte absente probable | 0x794b8 IPC_APP_set_door_control; 0x79640 IPC_APP_get_door_control | oui |
| 236 | ipc_lullaby_command | enum | `0` `1` | str | aucun | `ht_tuya_set_ipc_lullaby_cmd` : 0 suivant, 1 précédent (modulo 4) | 0x78b2c IPC_APP_set_ipc_lullaby_command | oui |
| 145 | battery_percent | sensor | pourcentage (constante 80) | aucun | int | retourne la constante 0x50 | 0x78350 IPC_APP_get_battery_percent | oui |
| 146 | power_mode | sensor | `1` constant | aucun | str | retourne la chaîne "1" | 0x7836c IPC_APP_get_power_mode | oui |
| 238 | temp_report_c | sensor | °C × 10 ? (0 si pas de capteur) | aucun | out | `ht_tuya_get_temperature(&v)` ; 0 en cas d'erreur | 0x78b90 IPC_APP_get_temp_report_c | oui |
| 239 | temp_report_f | sensor | °F (320 si pas de capteur) | aucun | out | `ht_tuya_get_temperature(&v)` converti ; 0x140 par défaut | 0x78bd4 IPC_APP_get_temp_report_f | oui |
| 240 | temp_unit_select | enum | `0` `1` | str | str | config `int_temp_unit_select` | 0x78cb0 IPC_APP_set_temp_unit_select; 0x78d80 IPC_APP_get_temp_unit_select | oui |
| 241 | temp_max_onoff | bool | ON / OFF | int | int | config d'alerte de température maximale | 0x78e1c IPC_APP_set_temp_max_onoff; 0x78e4c IPC_APP_get_temp_max_onoff | oui |
| 242 | temp_min_onoff | bool | ON / OFF | int | int | config d'alerte de température minimale | 0x78e78 IPC_APP_set_temp_min_onoff; 0x78ea8 IPC_APP_get_temp_min_onoff | oui |
| 243 | temp_max_cvalue | int | `0` `500` | int | int | config `int_temp_max_cvalue` + recalcul °F | 0x78f34 IPC_APP_set_temp_max_cvalue; 0x78ed4 IPC_APP_get_temp_max_cvalue | oui |
| 244 | temp_min_cvalue | int | `0` `500` | int | int | config `int_temp_min_cvalue` + recalcul °F | 0x79094 IPC_APP_set_temp_min_cvalue; 0x79034 IPC_APP_get_temp_min_cvalue | oui |
| 245 | temp_max_fvalue | int | `320` `1220` | int | int | config `int_temp_max_fvalue` + recalcul °C | 0x791fc IPC_APP_set_temp_max_fvalue; 0x79194 IPC_APP_get_temp_max_fvalue | oui |
| 246 | temp_min_fvalue | int | `320` `1220` | int | int | config `int_temp_min_fvalue` + recalcul °C | 0x79360 IPC_APP_set_temp_min_fvalue; 0x792f8 IPC_APP_get_temp_min_fvalue | oui |
| 101 | light_onoff | bool | ON / OFF | int | int | config `bool_led_enable` puis `ht_tuya_func_set_indicator_light` : LED témoin de la caméra | 0x7499c IPC_APP_set_light_onoff; 0x74a4c IPC_APP_get_light_onoff : défaut 1 | oui |
| 105 | sleep_mode | bool | ON / OFF | int | int | config `bool_sleep_mode` ; coupe le service média Tuya, `ht_ptz_set_cmd(9/10)` | 0x748e0 IPC_APP_set_sleep_mode; 0x74970 IPC_APP_get_sleep_mode : remplacé par l'alias `privacy` ; `sleep_mode/set` est routé vers `privacy` par la passerelle ; redémarre `anyka_ipc` (constaté sur la .188)| oui |
| 232 | ipc_lullaby_mode | enum | `0` `1` `2` | str | str | config `enum_ipc_lullaby_mode` | 0x78604 IPC_APP_set_ipc_lullaby_mode; 0x786e4 IPC_APP_get_ipc_lullaby_mode : lecteur de berceuses non vérifié | oui |
| 160 | ipc_lullaby_volume | int | `0` `10` | int | int | config `enum_ipc_lullaby_volume` | 0x787ec IPC_APP_set_ipc_lullaby_volume; 0x7878c IPC_APP_get_ipc_lullaby_volume | oui |
| 234 | ipc_lullaby_control | enum | `0` `1` | str | str | config `enum_ipc_lullaby_control` | 0x78850 IPC_APP_set_ipc_lullaby_control; 0x78924 IPC_APP_get_ipc_lullaby_control | oui |
| 235 | ipc_lullaby_list | enum | `0` `1` `2` `3` `4` | str | str | config `enum_ipc_lullaby_list` | 0x789c0 IPC_APP_set_ipc_lullaby_list; 0x78a98 IPC_APP_get_ipc_lullaby_list | oui |
