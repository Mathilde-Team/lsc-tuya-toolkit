# Convention de topics MQTT (passerelle locale)

Généré par `tools/gateway/gen_gateway.py` depuis `docs/firmware/DP_TABLE.md`. **Ne pas éditer le tableau à la main.**

## Règles

- `<base>` = nom de la caméra (ex. `camera188`), déjà utilisé par `night` et `privacy`.
- **Commande** : `<base>/<fonction>/set`. **État** : `<base>/<fonction>`, publié *retained*.
- `<fonction>` = nom de l'`IPC_APP_set_*`/`get_*` sans préfixe (ex. `IPC_APP_set_flip_onoff` → `flip_onoff`).
- Charges utiles : booléen `ON`/`OFF` ; énumération et entier en texte ; action = toute charge utile (les paramètres, s'il y en a, viennent des lots de décompilation).
- Fonction sans `set_` : lecture seule (`sensor`), pas de topic `/set`.
- Le même ordre peut venir d'ONVIF : les deux canaux écrivent dans `/tmp/md_cmd`.
- **Alias historiques conservés** : `<base>/night` (= `night_mode`, DP 108) et `<base>/privacy` (preset privé).
- **Alias PTZ et haut-parleur** (hors table des DP, scripts du toolkit ; arguments validés avant le shell) :
  `ptz_absolute/set` = `x,y` de -1 à 1 ; `ptz_preset_goto/set` = n° ; `ptz_preset_save/set` = nom (`A-Za-z0-9_`, publie `ptz_preset_saved` = n°) ;
  `ptz_preset_delete/set` = n° ; `ptz_move/set` = `up`/`down`/`left`/`right` (1 s puis arrêt) ou `stop` ; `ptz_home/set` = recalage sur les butées ; `ptz_calibrate/set` = `ON` (séquence de test, longue) ;
  `speaker_siren/set` = `ON`/`OFF` (boucle `sirene.ulaw`, 30 s max) ; `speaker_play/set` = nom d'un fichier `<nom>.ulaw` du dossier audio ou `stop`.
  États retained : `ptz_position` (`pan,tilt,zoom`, position suivie par calcul), `ptz_presets` (`n=nom,pan,tilt,zoom;…`), `speaker_siren`.
- `sd_health` : `ON` = problème (SD absente ou montée en lecture seule après une FAT corrompue), `OFF` = saine ; lu dans `/proc/mounts`, entité HA « Problème carte SD ».
- `sd_storage` (DP 109) est publié en JSON `{"total":…,"used":…,"free":…}` en **Kio** ; si le firmware renvoie 0 (stockage Tuya non initialisé sans cloud), la valeur vient de `df -k /tmp/sd`.
- Les **types sont une heuristique** sur le nom tant qu'un lot de décompilation (issues #54 à #58) ne les a pas confirmés dans `docs/firmware/DP_VALUES.md`.
- **Seuls les DP « exposé = oui »** sont pilotés par la passerelle (`tools/gateway/gateway.c`) ; les autres sont ignorés (journal `gateway: unknown or unexposed`).
- Le canal fichier `/tmp/md_cmd` accepte une ligne `<fonction> <valeur>` (ex. `night day`, `flip_onoff ON`).

## Table

| DP | Topic | Type | Valeurs | Écriture | Lecture | Lot | Exposé |
|---|---|---|---|---|---|---|---|
| 101 | `light_onoff` | bool | ON / OFF | `set_light_onoff` | `get_light_onoff` | #58 | oui |
| 103 | `flip_onoff` | bool | ON / OFF | `set_flip_onoff` | `get_flip_onoff` | #54 | oui |
| 104 | `watermark_onoff` | bool | ON / OFF | `set_watermark_onoff` | `get_watermark_onoff` | #54 | oui |
| 105 | `sleep_mode` | bool | ON / OFF | `set_sleep_mode` | `get_sleep_mode` | #58 | oui |
| 106 | `alarm_sensitivity` | enum | `0` `1` `2` | `set_alarm_sensitivity` | `get_alarm_sensitivity` | #55 | oui |
| 108 | `night_mode` | enum | `0` `1` `2` | `set_night_mode` | `get_night_mode` | #58 | oui |
| 109 | `sd_storage` | sensor | capacité de la carte SD | — | `get_sd_storage` | #56 | oui |
| 110 | `sd_status` | sensor | code d'état (entier) | — | `get_sd_status` | #56 | oui |
| 111 | `format_sd_card` | action | aucun | `format_sd_card` | — | #56 | non |
| 112 | `unmount_sd_card` | action | aucun | `unmount_sd_card` | — | #56 | non |
| 116 | `ptz_stop_move` | action | aucun | `ptz_stop_move` | — | #57 | oui |
| 117 | `sd_format_status` | sensor | progression du formatage | — | `get_sd_format_status` | #56 | oui |
| 119 | `ptz_start_move` | action | chaîne `0` `2` `4` `6` (0 haut, 2 droite, 4 bas, 6 gauche ; inversées si flip actif) | `ptz_start_move` | — | #57 | oui |
| 120 | `alarm_siren_onoff` | bool | ON / OFF | `set_alarm_siren_onoff` | `get_alarm_siren_onoff` | #55 | oui |
| 124 | `night_vision_mode` | enum | `0` `1` | `set_night_vision_mode` | `get_night_vision_mode` | #58 | oui |
| 132 | `ptz_check` | action | aucun | `ptz_check` | — | #57 | oui |
| 134 | `alarm_function_onoff` | bool | ON / OFF | `set_alarm_function_onoff` | `get_alarm_function_onoff` | #55 | oui |
| 138 | `blub_onoff` | bool | ON / OFF | `set_blub_onoff` | `get_blub_onoff` | #58 | oui |
| 139 | `sound_detect_onoff` | bool | ON / OFF | `set_sound_detect_onoff` | `get_sound_detect_onoff` | #55 | oui |
| 140 | `sound_sensitivity` | enum | `0` `1` | `set_sound_sensitivity` | `get_sound_sensitivity` | #55 | oui |
| 145 | `battery_percent` | sensor | pourcentage (constante 80) | — | `get_battery_percent` | #58 | oui |
| 146 | `power_mode` | sensor | `1` constant | — | `get_power_mode` | #58 | oui |
| 150 | `sd_record_onoff` | bool | ON / OFF | `set_sd_record_onoff` | `get_sd_record_onoff` | #56 | non |
| 151 | `sd_record_mode` | enum | `0` `1` | `set_sd_record_mode` | `get_sd_record_mode` | #56 | non |
| 152 | `pir_detect_switch` | enum | `0` `1` `2` `3` | `set_pir_detect_switch` | `get_pir_detect_switch` | #58 | oui |
| 158 | `floodlight_lightness` | int | `1` `100` | `set_floodlight_lightness` | `get_floodlight_lightness` | #58 | oui |
| 159 | `siren_onoff` | bool | ON / OFF | `set_siren_onoff` | `get_siren_onoff` | #55 | oui |
| 160 | `ipc_lullaby_volume` | int | `0` `10` | `set_ipc_lullaby_volume` | `get_ipc_lullaby_volume` | #58 | oui |
| 161 | `track_enable` | bool | ON / OFF | `track_enable` | `get_track_enable` | #57 | oui |
| 162 | `system_reboot` | action | charge utile ignorée (envoyer `1`) | `set_system_reboot` | — | #54 | oui |
| 167 | `cry_detect_onoff` | bool | ON / OFF | `set_cry_detect_onoff` | `get_cry_detect_onoff` | #55 | oui |
| 168 | `alarm_zone_onoff` | bool | ON / OFF | `set_alarm_zone_onoff` | `get_alarm_zone_onoff` | #55 | oui |
| 169 | `alarm_zone_draw` | json | JSON `{"num":1,"region0":{"x":0,"y":0,"xlen":100,"ylen":100}}` | `set_alarm_zone_draw` | `get_alarm_zone_draw` | #55 | oui |
| 170 | `human_filter` | bool | ON / OFF | `set_human_filter` | `get_human_filter` | #55 | oui |
| 172 | `flight_warn_onoff` | bool | ON / OFF | `set_flight_warn_onoff` | `get_flight_warn_onoff` | #58 | oui |
| 173 | `flight_warn_time` | int | `0` `3600` | `set_flight_warn_time` | `get_flight_warn_time` | #58 | oui |
| 174 | `patrol_switch` | bool | ON / OFF | `set_patrol_switch` | — | #57 | oui |
| 175 | `patrol_mode` | enum | `0` `1` | `set_patrol_mode` | `get_patrol_mode` | #57 | oui |
| 176 | `patrol_tmode` | int | `0` `255` | `set_patrol_tmode` | — | #57 | oui |
| 177 | `patrol_time` | int | `0` `86400` | `set_patrol_time` | — | #57 | oui |
| 178 | `preset` | json | JSON `{"type":1,"data":{"name":"p1"}}` | `set_preset` | — | #57 | oui |
| 179 | `patrol_state` | sensor | état de patrouille (entier) | — | `patrol_state` | #57 | oui |
| 188 | `basic_anti_flicker` | enum | `0` `1` `2` | `set_basic_anti_flicker` | `get_basic_anti_flicker` | #54 | oui |
| 190 | `link_move` | int | `0` `255` | `set_link_move` | — | #57 | oui |
| 198 | `object_outline_onoff` | bool | ON / OFF | `set_object_outline_onoff` | `get_object_outline_onoff` | #54 | oui |
| 199 | `link_pos` | int | `0` `255` | `set_link_pos` | — | #57 | oui |
| 232 | `ipc_lullaby_mode` | enum | `0` `1` `2` | `set_ipc_lullaby_mode` | `get_ipc_lullaby_mode` | #58 | oui |
| 234 | `ipc_lullaby_control` | enum | `0` `1` | `set_ipc_lullaby_control` | `get_ipc_lullaby_control` | #58 | oui |
| 235 | `ipc_lullaby_list` | enum | `0` `1` `2` `3` `4` | `set_ipc_lullaby_list` | `get_ipc_lullaby_list` | #58 | oui |
| 236 | `ipc_lullaby_command` | enum | `0` `1` | `set_ipc_lullaby_command` | — | #58 | oui |
| 238 | `temp_report_c` | sensor | °C × 10 ? (0 si pas de capteur) | — | `get_temp_report_c` | #58 | oui |
| 239 | `temp_report_f` | sensor | °F (320 si pas de capteur) | — | `get_temp_report_f` | #58 | oui |
| 240 | `temp_unit_select` | enum | `0` `1` | `set_temp_unit_select` | `get_temp_unit_select` | #58 | oui |
| 241 | `temp_max_onoff` | bool | ON / OFF | `set_temp_max_onoff` | `get_temp_max_onoff` | #58 | oui |
| 242 | `temp_min_onoff` | bool | ON / OFF | `set_temp_min_onoff` | `get_temp_min_onoff` | #58 | oui |
| 243 | `temp_max_cvalue` | int | `0` `500` | `set_temp_max_cvalue` | `get_temp_max_cvalue` | #58 | oui |
| 244 | `temp_min_cvalue` | int | `0` `500` | `set_temp_min_cvalue` | `get_temp_min_cvalue` | #58 | oui |
| 245 | `temp_max_fvalue` | int | `320` `1220` | `set_temp_max_fvalue` | `get_temp_max_fvalue` | #58 | oui |
| 246 | `temp_min_fvalue` | int | `320` `1220` | `set_temp_min_fvalue` | `get_temp_min_fvalue` | #58 | oui |
| 247 | `audible_alarm_onoff` | bool | ON / OFF | `set_audible_alarm_onoff` | `get_audible_alarm_onoff` | #55 | oui |
| 248 | `door_control` | enum | `0` `1` `2` `3` `4` `5` `6` | `set_door_control` | `get_door_control` | #58 | oui |

## DP sans fonction `IPC_APP_*` identifiée

| DP | Gestionnaire | Adresse |
|---|---|---|
| 136 | `FUN_0007b588` | 0x7b588 |
| 141 | `FUN_0007cb40` | 0x7cb40 |
| 142 | `FUN_0007cb40` | 0x7cb40 |
| 143 | `FUN_0007cb40` | 0x7cb40 |
| 147 | `FUN_0007b72c` | 0x7b72c |
| 233 | `FUN_0007c940` | 0x7c940 |
