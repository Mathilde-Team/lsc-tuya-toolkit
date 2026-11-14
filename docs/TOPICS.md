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
- Les **types sont une heuristique** sur le nom tant qu'un lot de décompilation (issues #54 à #58) ne les a pas confirmés dans `docs/firmware/DP_VALUES.md`.
- **Seuls les DP « exposé = oui »** sont pilotés par la passerelle (`tools/gateway/gateway.c`) ; les autres sont ignorés (journal `gateway: unknown or unexposed`).
- Le canal fichier `/tmp/md_cmd` accepte une ligne `<fonction> <valeur>` (ex. `night day`, `flip_onoff ON`).

## Table

| DP | Topic | Type | Valeurs | Écriture | Lecture | Lot | Exposé |
|---|---|---|---|---|---|---|---|
| 101 | `light_onoff` | bool ? | ON / OFF | `set_light_onoff` | `get_light_onoff` | #58 | non |
| 103 | `flip_onoff` | bool ? | ON / OFF | `set_flip_onoff` | `get_flip_onoff` | #54 | non |
| 104 | `watermark_onoff` | bool ? | ON / OFF | `set_watermark_onoff` | `get_watermark_onoff` | #54 | non |
| 105 | `sleep_mode` | enum ? | valeur à confirmer | `set_sleep_mode` | `get_sleep_mode` | #58 | non |
| 106 | `alarm_sensitivity` | int ? | entier, plage à confirmer | `set_alarm_sensitivity` | `get_alarm_sensitivity` | #55 | non |
| 108 | `night_mode` | enum ? | valeur à confirmer | `set_night_mode` | `get_night_mode` | #58 | non |
| 109 | `sd_storage` | sensor ? | lecture seule | — | `get_sd_storage` | #56 | non |
| 110 | `sd_status` | sensor ? | lecture seule | — | `get_sd_status` | #56 | non |
| 111 | `format_sd_card` | action ? | ordre (charge utile ignorée ou paramètres) | `format_sd_card` | — | #56 | non |
| 112 | `unmount_sd_card` | action ? | ordre (charge utile ignorée ou paramètres) | `unmount_sd_card` | — | #56 | non |
| 116 | `ptz_stop_move` | action ? | ordre (charge utile ignorée ou paramètres) | `ptz_stop_move` | — | #57 | non |
| 117 | `sd_format_status` | sensor ? | lecture seule | — | `get_sd_format_status` | #56 | non |
| 119 | `ptz_start_move` | action ? | ordre (charge utile ignorée ou paramètres) | `ptz_start_move` | — | #57 | non |
| 120 | `alarm_siren_onoff` | bool ? | ON / OFF | `set_alarm_siren_onoff` | `get_alarm_siren_onoff` | #55 | non |
| 124 | `night_vision_mode` | enum ? | valeur à confirmer | `set_night_vision_mode` | `get_night_vision_mode` | #58 | non |
| 132 | `ptz_check` | action ? | ordre (charge utile ignorée ou paramètres) | `ptz_check` | — | #57 | non |
| 134 | `alarm_function_onoff` | bool ? | ON / OFF | `set_alarm_function_onoff` | `get_alarm_function_onoff` | #55 | non |
| 138 | `blub_onoff` | bool ? | ON / OFF | `set_blub_onoff` | `get_blub_onoff` | #58 | non |
| 139 | `sound_detect_onoff` | bool ? | ON / OFF | `set_sound_detect_onoff` | `get_sound_detect_onoff` | #55 | non |
| 140 | `sound_sensitivity` | int ? | entier, plage à confirmer | `set_sound_sensitivity` | `get_sound_sensitivity` | #55 | non |
| 145 | `battery_percent` | sensor ? | lecture seule | — | `get_battery_percent` | #58 | non |
| 146 | `power_mode` | sensor ? | lecture seule | — | `get_power_mode` | #58 | non |
| 150 | `sd_record_onoff` | bool ? | ON / OFF | `set_sd_record_onoff` | `get_sd_record_onoff` | #56 | non |
| 151 | `sd_record_mode` | enum ? | valeur à confirmer | `set_sd_record_mode` | `get_sd_record_mode` | #56 | non |
| 152 | `pir_detect_switch` | bool ? | ON / OFF | `set_pir_detect_switch` | `get_pir_detect_switch` | #58 | non |
| 158 | `floodlight_lightness` | int ? | entier, plage à confirmer | `set_floodlight_lightness` | `get_floodlight_lightness` | #58 | non |
| 159 | `siren_onoff` | bool ? | ON / OFF | `set_siren_onoff` | `get_siren_onoff` | #55 | non |
| 160 | `ipc_lullaby_volume` | int ? | entier, plage à confirmer | `set_ipc_lullaby_volume` | `get_ipc_lullaby_volume` | #58 | non |
| 161 | `track_enable` | sensor ? | lecture seule | — | `get_track_enable` | #57 | non |
| 162 | `system_reboot` | int ? | entier, plage à confirmer | `set_system_reboot` | — | #54 | non |
| 167 | `cry_detect_onoff` | bool ? | ON / OFF | `set_cry_detect_onoff` | `get_cry_detect_onoff` | #55 | non |
| 168 | `alarm_zone_onoff` | bool ? | ON / OFF | `set_alarm_zone_onoff` | `get_alarm_zone_onoff` | #55 | non |
| 169 | `alarm_zone_draw` | enum ? | valeur à confirmer | `set_alarm_zone_draw` | `get_alarm_zone_draw` | #55 | non |
| 170 | `human_filter` | int ? | entier, plage à confirmer | `set_human_filter` | `get_human_filter` | #55 | non |
| 172 | `flight_warn_onoff` | bool ? | ON / OFF | `set_flight_warn_onoff` | `get_flight_warn_onoff` | #58 | non |
| 173 | `flight_warn_time` | int ? | entier, plage à confirmer | `set_flight_warn_time` | `get_flight_warn_time` | #58 | non |
| 174 | `patrol_switch` | bool ? | ON / OFF | `set_patrol_switch` | — | #57 | non |
| 175 | `patrol_mode` | enum ? | valeur à confirmer | `set_patrol_mode` | `get_patrol_mode` | #57 | non |
| 176 | `patrol_tmode` | enum ? | valeur à confirmer | `set_patrol_tmode` | — | #57 | non |
| 177 | `patrol_time` | int ? | entier, plage à confirmer | `set_patrol_time` | — | #57 | non |
| 178 | `preset` | int ? | entier, plage à confirmer | `set_preset` | — | #57 | non |
| 179 | `patrol_state` | sensor ? | lecture seule | — | — | #57 | non |
| 188 | `basic_anti_flicker` | int ? | entier, plage à confirmer | `set_basic_anti_flicker` | `get_basic_anti_flicker` | #54 | non |
| 190 | `link_move` | int ? | entier, plage à confirmer | `set_link_move` | — | #57 | non |
| 198 | `object_outline_onoff` | bool ? | ON / OFF | `set_object_outline_onoff` | `get_object_outline_onoff` | #54 | non |
| 199 | `link_pos` | int ? | entier, plage à confirmer | `set_link_pos` | — | #57 | non |
| 232 | `ipc_lullaby_mode` | enum ? | valeur à confirmer | `set_ipc_lullaby_mode` | `get_ipc_lullaby_mode` | #58 | non |
| 234 | `ipc_lullaby_control` | enum ? | valeur à confirmer | `set_ipc_lullaby_control` | `get_ipc_lullaby_control` | #58 | non |
| 235 | `ipc_lullaby_list` | enum ? | valeur à confirmer | `set_ipc_lullaby_list` | `get_ipc_lullaby_list` | #58 | non |
| 236 | `ipc_lullaby_command` | enum ? | valeur à confirmer | `set_ipc_lullaby_command` | — | #58 | non |
| 238 | `temp_report_c` | sensor ? | lecture seule | — | `get_temp_report_c` | #58 | non |
| 239 | `temp_report_f` | sensor ? | lecture seule | — | `get_temp_report_f` | #58 | non |
| 240 | `temp_unit_select` | enum ? | valeur à confirmer | `set_temp_unit_select` | `get_temp_unit_select` | #58 | non |
| 241 | `temp_max_onoff` | bool ? | ON / OFF | `set_temp_max_onoff` | `get_temp_max_onoff` | #58 | non |
| 242 | `temp_min_onoff` | bool ? | ON / OFF | `set_temp_min_onoff` | `get_temp_min_onoff` | #58 | non |
| 243 | `temp_max_cvalue` | int ? | entier, plage à confirmer | `set_temp_max_cvalue` | `get_temp_max_cvalue` | #58 | non |
| 244 | `temp_min_cvalue` | int ? | entier, plage à confirmer | `set_temp_min_cvalue` | `get_temp_min_cvalue` | #58 | non |
| 245 | `temp_max_fvalue` | int ? | entier, plage à confirmer | `set_temp_max_fvalue` | `get_temp_max_fvalue` | #58 | non |
| 246 | `temp_min_fvalue` | int ? | entier, plage à confirmer | `set_temp_min_fvalue` | `get_temp_min_fvalue` | #58 | non |
| 247 | `audible_alarm_onoff` | bool ? | ON / OFF | `set_audible_alarm_onoff` | `get_audible_alarm_onoff` | #55 | non |
| 248 | `door_control` | enum ? | valeur à confirmer | `set_door_control` | `get_door_control` | #58 | non |

## DP sans fonction `IPC_APP_*` identifiée

| DP | Gestionnaire | Adresse |
|---|---|---|
| 136 | `FUN_0007b588` | 0x7b588 |
| 141 | `FUN_0007cb40` | 0x7cb40 |
| 142 | `FUN_0007cb40` | 0x7cb40 |
| 143 | `FUN_0007cb40` | 0x7cb40 |
| 147 | `FUN_0007b72c` | 0x7b72c |
| 233 | `FUN_0007c940` | 0x7c940 |
