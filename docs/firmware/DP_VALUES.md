# Valeurs et signatures des DP (lots de décompilation)

Rédigé **à la main** par les lots de décompilation (#54 à #58), un tableau par lot. Contrairement aux autres fichiers de ce dossier, il n'est pas généré : c'est la **source des confirmations** lue par `tools/gateway/gen_gateway.py`.

Règle (epic #51) : chaque ligne s'appuie sur le désassemblage (adresse ou fonction citée dans « Preuve »). Une ligne sans adresse ne peut pas être exposée : le générateur refuse.

## Format d'une ligne (lu par le générateur)

`| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |`

| Colonne | Contenu attendu |
|---|---|
| DP | numéro (ex. `103`) |
| Fonction | nom du topic, comme dans `docs/TOPICS.md` (informatif) |
| Type | `bool`, `enum`, `int`, `action` ou `sensor` |
| Valeurs | `bool` : texte libre (charges `ON`/`OFF`) ; `enum` : **chaque charge utile admise entre accents graves** (ex. `` `0` `1` `2` ``) ; `int` : **deux nombres entre accents graves**, min puis max (ex. `` `0` `100` ``) ; autres : texte libre |
| Écriture | passage de la valeur à `IPC_APP_set_*` (registre r0) : `int`, `str` (pointeur de chaîne) ou `aucun` |
| Lecture | retour de `IPC_APP_get_*` (r0) : `int`, `str` (pointeur de chaîne) ou `aucun` |
| Effets de bord / persistance | clé de configuration écrite (`ht_tuya_config_set_*`), redémarrage, etc. |
| Preuve | adresse(s) / fonction(s) lues (ex. `0x74a78 IPC_APP_set_flip_onoff : ldr ... bl ht_tuya_config_set_int`) |
| Exposé | `oui` = la passerelle pilote ce DP ; `non` = documenté seulement (matériel absent, effet dangereux, non testé) |

Après modification : `python3 tools/gateway/gen_gateway.py` régénère `tools/gateway/dp_table.h` et `docs/TOPICS.md`, puis `tools/motion/build.sh`.

Conversion faite par la passerelle (`tools/gateway/gateway.c`) :

- `bool` : `ON`/`on` → 1, `OFF`/`off` → 0 ; passé en entier (`int`) ou en chaîne `"1"`/`"0"` (`str`). État publié : `ON`/`OFF`.
- `enum` : la charge utile doit figurer dans la liste ; passée telle quelle (`str`) ou convertie en entier (`int`, valeurs numériques uniquement).
- `int` : entier décimal dans [min, max] ; passé en entier ou en chaîne.
- `action` : `aucun` = appel sans argument ; sinon comme `int`/`str`. Pas d'état publié.
- `sensor` : lu au démarrage et toutes les 5 minutes, publié retained.

## Lot 1 : image et système (#54)

Lu dans le pseudo-C (`tools/firmware/Decompile.java`), conversions par `docs/firmware/DECOMPILE.md`.

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 103 | flip_onoff | bool | ON / OFF | int | int | config `bool_rotate180` puis `ht_tuya_func_set_flip_mirror(v)` | 0x74a78 IPC_APP_set_flip_onoff; 0x74b08 IPC_APP_get_flip_onoff : set écrit la clé puis applique le retournement si l'écriture réussit | oui |
| 104 | watermark_onoff | bool | ON / OFF | int | int | config `?` puis `ht_tuya_func_set_osd(v)` | 0x74b34 IPC_APP_set_watermark_onoff; 0x74bc4 IPC_APP_get_watermark_onoff : set écrit la clé puis applique l'OSD; get défaut 1 | oui |
| 188 | basic_anti_flicker | enum | `0` `1` `2` | str | str | config `?` puis `ht_tuya_func_set_basic_anti_flicker` : 2 → 60 Hz, 0 et 1 → 50 Hz | 0x78390 IPC_APP_set_basic_anti_flicker; 0x78488 IPC_APP_get_basic_anti_flicker : set : `atoi(chaîne)`, refus hors 0/1/2; get renvoie un pointeur de chaîne, défaut 2 | oui |
| 198 | object_outline_onoff | bool | ON / OFF | int | int | config `bool_object_outline_switch` seulement (aucun appel d'effet dans cette fonction) | 0x79d20 IPC_APP_set_object_outline_onoff; 0x79d50 IPC_APP_get_object_outline_onoff : effet réel non trouvé : lu ailleurs ? à confirmer | non |
| 162 | system_reboot | action | charge utile ignorée (envoyer `1`) | int | aucun | `system("reboot -f")` si la valeur est non nulle : **redémarrage immédiat sans arrêt propre** | 0x754c8 IPC_APP_set_system_reboot : set n'appelle rien si param=0 | oui |

## Lot 2 : détection et alarmes (#55)

Lu dans le pseudo-C. Ces DP agissent sur la **détection du firmware**, qui n'est pas notre `md_inject` : laissés non exposés tant qu'un test caméra n'a pas montré l'effet.

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 106 | alarm_sensitivity | enum | `0` `1` `2` | str | str | config `enum_alarm_sensitivity` puis `ht_tuya_func_set_montion_detect_sensitivity(v)` | 0x752ec IPC_APP_set_alarm_sensitivity; 0x7537c IPC_APP_get_alarm_sensitivity : atoi puis refus hors 0/1/2; défaut 1. Agit sur la détection du firmware, pas sur `md_inject` (non câblé) | non |
| 134 | alarm_function_onoff | bool | ON / OFF | int | int | config `bool_alarm_enable` puis relit et réapplique `IPC_APP_set_human_filter` | 0x751d0 IPC_APP_set_alarm_function_onoff; 0x75268 IPC_APP_get_alarm_function_onoff : get défaut dépend de la clé `pir_support` | non |
| 170 | human_filter | bool | ON / OFF | int | int | config `bool_human_filter_enable` puis `ht_tuya_func_set_human_filter(v)` | 0x769c8 IPC_APP_set_human_filter; 0x76a50 IPC_APP_get_human_filter : filtre du firmware, pas celui de `md_inject` | non |
| 120 | alarm_siren_onoff | bool | ON / OFF | int | int | config `bool_alarm_siren_enable` seulement | 0x7541c IPC_APP_set_alarm_siren_onoff; 0x7549c IPC_APP_get_alarm_siren_onoff : aucun effet direct trouvé | non |
| 139 | sound_detect_onoff | bool | ON / OFF | int | int | config `bool_sound_detect_enable` puis `ht_tuya_func_set_sound_detect(v)` | 0x74fe4 IPC_APP_set_sound_detect_onoff; 0x75078 IPC_APP_get_sound_detect_onoff | non |
| 140 | sound_sensitivity | enum | `0` `1` | str | str | config `enum_sound_sensitivity` puis `ht_tuya_func_set_sound_detect_sensitivity(v)` | 0x750a4 IPC_APP_set_sound_sensitivity; 0x7513c IPC_APP_get_sound_sensitivity : atoi, refus hors 0/1; défaut 0 | non |
| 167 | cry_detect_onoff | bool | ON / OFF | int | int | config `bool_cry_detect_enable` puis `ht_tuya_func_set_cry_detect(v)` | 0x7988c IPC_APP_set_cry_detect_onoff; 0x798d0 IPC_APP_get_cry_detect_onoff : défaut selon la clé `support_cry_detect` | non |

## Lot 3 : carte SD (#56)

À compléter (DP 109, 110, 111, 112, 117, 150, 151).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 150 | sd_record_onoff | bool | ON / OFF | int | int | config `bool_sd_record_enable` puis `ht_tuya_stream_storage_set_record_onoff(v)` | 0x75e80 IPC_APP_set_sd_record_onoff; 0x75f08 IPC_APP_get_sd_record_onoff : enregistrement du stockage Tuya, pas celui du NVR (#5) | non |
| 151 | sd_record_mode | enum | `0` `1` | int | int | config `enum_sd_record_mode` puis force `record_onoff(1)` | 0x75f34 IPC_APP_set_sd_record_mode; 0x75fdc IPC_APP_get_sd_record_mode : refus hors 0/1; défaut 0 | non |
| 110 | sd_status | sensor | code d'état (entier) | aucun | int | lecture seule : `ht_tuya_stream_storage_tf_status()` | 0x75de8 IPC_APP_get_sd_status : codes non décodés | non |
| 111 | format_sd_card | action | aucun | aucun | aucun | lance un thread de formatage de la carte SD : **destructif** | 0x76420 IPC_APP_format_sd_card : statut via DP 117 (`get_sd_format_status`) | non |

## Lot 4 : PTZ natif (#57)

À compléter (DP 116, 119, 132, 161, 174 à 179, 190, 199).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 119 | ptz_start_move | action | chaîne `0` à `7` (0 haut, 2 droite, 4 bas, 6 gauche selon le firmware) | str | aucun | `ht_ptz_set_cmd` ; directions inversées si `bool_rotate180` | 0x76558 IPC_APP_ptz_start_move : valeurs impaires ignorées | non |
| 116 | ptz_stop_move | action | aucun | aucun | aucun | `ht_ptz_set_cmd(0,-1)` puis `IPC_APP_ptz_save_pos` | 0x76858 IPC_APP_ptz_stop_move | non |
| 132 | ptz_check | action | aucun | aucun | aucun | `ht_ptz_calibration()` : recalibrage mécanique | 0x768b0 IPC_APP_ptz_check | non |
| 161 | track_enable | bool | ON / OFF | int | int | config `bool_track_enable` | 0x76900 IPC_APP_track_enable; 0x7699c IPC_APP_get_track_enable : effet réel non vérifié | non |
| 175 | patrol_mode | enum | `0` `1` | str | str | config `enum_patrol_mode` | 0x76a7c IPC_APP_set_patrol_mode; 0x76b88 IPC_APP_get_patrol_mode | non |
| 174 | patrol_switch | bool | ON / OFF | int | aucun | `ht_ptz_set_patrol_mode(0 à 2)` | 0x76c24 IPC_APP_set_patrol_switch | non |

## Lot 5 : berceuse, température, lumière, PIR, porte (#58)

À compléter (DP 232 à 236, 160, 238 à 246, 101, 138, 158, 152, 248).

| DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets de bord / persistance | Preuve | Exposé |
|---|---|---|---|---|---|---|---|---|
| 101 | light_onoff | bool | ON / OFF | int | int | config `bool_led_enable` puis `ht_tuya_func_set_indicator_light` : LED témoin de la caméra | 0x7499c IPC_APP_set_light_onoff; 0x74a4c IPC_APP_get_light_onoff : défaut 1 | oui |
| 105 | sleep_mode | bool | ON / OFF | int | int | config `bool_sleep_mode` ; coupe le service média Tuya, `ht_ptz_set_cmd(9/10)` | 0x748e0 IPC_APP_set_sleep_mode; 0x74970 IPC_APP_get_sleep_mode : remplacé par l'alias `privacy` | non |
| 232 | ipc_lullaby_mode | enum | `0` `1` `2` | str | str | config `enum_ipc_lullaby_mode` | 0x78604 IPC_APP_set_ipc_lullaby_mode; 0x786e4 IPC_APP_get_ipc_lullaby_mode : lecteur de berceuses non vérifié | non |
| 160 | ipc_lullaby_volume | int | `0` `10` | int | int | config `enum_ipc_lullaby_volume` | 0x787ec IPC_APP_set_ipc_lullaby_volume; 0x7878c IPC_APP_get_ipc_lullaby_volume | non |
| 234 | ipc_lullaby_control | enum | `0` `1` | str | str | config `enum_ipc_lullaby_control` | 0x78850 IPC_APP_set_ipc_lullaby_control; 0x78924 IPC_APP_get_ipc_lullaby_control | non |
| 235 | ipc_lullaby_list | enum | `0` `1` `2` `3` `4` | str | str | config `enum_ipc_lullaby_list` | 0x789c0 IPC_APP_set_ipc_lullaby_list; 0x78a98 IPC_APP_get_ipc_lullaby_list | non |
