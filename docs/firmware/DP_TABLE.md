# Table des commandes Tuya (DP) de `anyka_ipc`

Généré par `tools/firmware/gen_reference.py` à partir du désassemblage. **Ne pas éditer à la main.**

Le binaire contient une table de 67 paires **(numéro de DP, fonction)** dans `.data` à partir de 0x50ed70. Quand le service Tuya reçoit un ordre du cloud, il cherche le numéro dans cette table et appelle la fonction. Ces fonctions appellent à leur tour les `IPC_APP_*` puis les `ht_*`.

**Conséquence pour le hack** : sans cloud, cette table n'est jamais parcourue ; on appelle directement les `IPC_APP_*` / `ht_*` (comme `md_inject.so`).

Le nom du DP Tuya n'est **pas** dans le binaire : c'est le couple `IPC_APP_set_*` / `get_*` appelé qui dit ce que fait le DP.

| DP | Gestionnaire | Adresse | `IPC_APP_*` / `ht_*` appelés (profondeur 2) | Source |
|---|---|---|---|---|
| 105 | `FUN_0007a5e4` | 0x7a5e4 | `IPC_APP_set_sleep_mode`, `IPC_APP_get_sleep_mode` | tuya_ipc_dp_utils.c |
| 101 | `FUN_0007a624` | 0x7a624 | `IPC_APP_set_light_onoff`, `IPC_APP_get_light_onoff` | tuya_ipc_dp_utils.c |
| 103 | `FUN_0007a664` | 0x7a664 | `IPC_APP_set_flip_onoff`, `IPC_APP_get_flip_onoff` | tuya_ipc_dp_utils.c |
| 104 | `FUN_0007a6a4` | 0x7a6a4 | `IPC_APP_set_watermark_onoff`, `IPC_APP_get_watermark_onoff` | tuya_ipc_dp_utils.c |
| 108 | `FUN_0007a6e4` | 0x7a6e4 | `IPC_APP_set_night_mode`, `IPC_APP_get_night_mode` | tuya_ipc_dp_utils.c |
| 124 | `FUN_0007a7a4` | 0x7a7a4 | `IPC_APP_set_night_vision_mode`, `IPC_APP_get_night_vision_mode` | tuya_ipc_dp_utils.c |
| 134 | `FUN_0007a99c` | 0x7a99c | `IPC_APP_set_alarm_function_onoff`, `IPC_APP_get_alarm_function_onoff` | tuya_ipc_dp_utils.c |
| 106 | `FUN_0007a9dc` | 0x7a9dc | `IPC_APP_set_alarm_sensitivity`, `IPC_APP_get_alarm_sensitivity` | tuya_ipc_dp_utils.c |
| 120 | `FUN_0007aa9c` | 0x7aa9c | `IPC_APP_set_alarm_siren_onoff`, `IPC_APP_get_alarm_siren_onoff` | tuya_ipc_dp_utils.c |
| 168 | `FUN_0007ab14` | 0x7ab14 | `IPC_APP_set_alarm_zone_onoff`, `IPC_APP_get_alarm_zone_onoff` | tuya_ipc_dp_utils.c |
| 169 | `FUN_0007aba4` | 0x7aba4 | `IPC_APP_set_alarm_zone_draw`, `IPC_APP_get_alarm_zone_draw` | tuya_ipc_dp_utils.c |
| 110 | `FUN_0007ac28` | 0x7ac28 | `IPC_APP_get_sd_status` | tuya_ipc_dp_utils.c |
| 109 | `FUN_0007ac54` | 0x7ac54 | `IPC_APP_get_sd_storage` | tuya_ipc_dp_utils.c |
| 150 | `FUN_0007acfc` | 0x7acfc | `IPC_APP_set_sd_record_onoff`, `IPC_APP_get_sd_record_onoff` | tuya_ipc_dp_utils.c |
| 151 | `FUN_0007ad3c` | 0x7ad3c | `IPC_APP_set_sd_record_mode`, `IPC_APP_get_sd_record_mode` | tuya_ipc_dp_utils.c |
| 112 | `FUN_0007ae08` | 0x7ae08 | `IPC_APP_unmount_sd_card` | tuya_ipc_dp_utils.c |
| 111 | `FUN_0007ae34` | 0x7ae34 | `IPC_APP_format_sd_card` | tuya_ipc_dp_utils.c |
| 117 | `FUN_0007ae5c` | 0x7ae5c | `IPC_APP_get_sd_format_status` | tuya_ipc_dp_utils.c |
| 119 | `FUN_0007ae88` | 0x7ae88 | `IPC_APP_ptz_start_move` | tuya_ipc_dp_utils.c |
| 116 | `FUN_0007af90` | 0x7af90 | `IPC_APP_ptz_stop_move` | tuya_ipc_dp_utils.c |
| 159 | `FUN_0007afb8` | 0x7afb8 | `IPC_APP_set_siren_onoff`, `IPC_APP_get_siren_onoff` | tuya_ipc_dp_utils.c |
| 162 | `FUN_0007b01c` | 0x7b01c | `IPC_APP_set_system_reboot` | tuya_ipc_dp_utils.c |
| 132 | `FUN_0007b054` | 0x7b054 | `IPC_APP_ptz_check` | tuya_ipc_dp_utils.c |
| 161 | `FUN_0007b07c` | 0x7b07c | `IPC_APP_track_enable`, `IPC_APP_get_track_enable` | tuya_ipc_dp_utils.c |
| 170 | `FUN_0007b254` | 0x7b254 | `IPC_APP_set_human_filter`, `IPC_APP_get_human_filter` | tuya_ipc_dp_utils.c |
| 175 | `FUN_0007b294` | 0x7b294 | `IPC_APP_set_patrol_mode`, `IPC_APP_get_patrol_mode` | tuya_ipc_dp_utils.c |
| 174 | `FUN_0007b354` | 0x7b354 | `IPC_APP_set_patrol_switch` | tuya_ipc_dp_utils.c |
| 176 | `FUN_0007b38c` | 0x7b38c | `IPC_APP_set_patrol_tmode` | tuya_ipc_dp_utils.c |
| 177 | `FUN_0007b450` | 0x7b450 | `IPC_APP_set_patrol_time` | tuya_ipc_dp_utils.c |
| 179 | `FUN_0007b4c0` | 0x7b4c0 | `IPC_APP_patrol_state` | tuya_ipc_dp_utils.c |
| 178 | `FUN_0007b55c` | 0x7b55c | `IPC_APP_set_preset` | tuya_ipc_dp_utils.c |
| 190 | `FUN_0007b0bc` | 0x7b0bc | `IPC_APP_set_link_move` | tuya_ipc_dp_utils.c |
| 199 | `FUN_0007b188` | 0x7b188 | `IPC_APP_set_link_pos` | tuya_ipc_dp_utils.c |
| 136 | `FUN_0007b588` | 0x7b588 | — | tuya_ipc_dp_utils.c |
| 138 | `FUN_0007b5f0` | 0x7b5f0 | `IPC_APP_set_blub_onoff`, `IPC_APP_get_blub_onoff` | tuya_ipc_dp_utils.c |
| 139 | `FUN_0007a89c` | 0x7a89c | `IPC_APP_set_sound_detect_onoff`, `IPC_APP_get_sound_detect_onoff` | tuya_ipc_dp_utils.c |
| 140 | `FUN_0007a8dc` | 0x7a8dc | `IPC_APP_set_sound_sensitivity`, `IPC_APP_get_sound_sensitivity` | tuya_ipc_dp_utils.c |
| 141 | `FUN_0007cb40` | 0x7cb40 | — | tuya_ipc_dp_utils.c |
| 142 | `FUN_0007cb40` | 0x7cb40 | — | tuya_ipc_dp_utils.c |
| 143 | `FUN_0007cb40` | 0x7cb40 | — | tuya_ipc_dp_utils.c |
| 145 | `FUN_0007b658` | 0x7b658 | `IPC_APP_get_battery_percent` | tuya_ipc_dp_utils.c |
| 146 | `FUN_0007b6c0` | 0x7b6c0 | `IPC_APP_get_power_mode` | tuya_ipc_dp_utils.c |
| 147 | `FUN_0007b72c` | 0x7b72c | — | tuya_ipc_dp_utils.c |
| 188 | `FUN_0007b7c0` | 0x7b7c0 | `IPC_APP_set_basic_anti_flicker`, `IPC_APP_get_basic_anti_flicker` | tuya_ipc_dp_utils.c |
| 158 | `FUN_0007b880` | 0x7b880 | `IPC_APP_set_floodlight_lightness`, `IPC_APP_get_floodlight_lightness` | tuya_ipc_dp_utils.c |
| 232 | `FUN_0007b928` | 0x7b928 | `IPC_APP_set_ipc_lullaby_mode`, `IPC_APP_get_ipc_lullaby_mode` | tuya_ipc_dp_utils.c |
| 160 | `FUN_0007b9e8` | 0x7b9e8 | `IPC_APP_set_ipc_lullaby_volume`, `IPC_APP_get_ipc_lullaby_volume` | tuya_ipc_dp_utils.c |
| 234 | `FUN_0007ba90` | 0x7ba90 | `IPC_APP_set_ipc_lullaby_control`, `IPC_APP_get_ipc_lullaby_control` | tuya_ipc_dp_utils.c |
| 235 | `FUN_0007bb78` | 0x7bb78 | `IPC_APP_set_ipc_lullaby_list`, `IPC_APP_get_ipc_lullaby_list` | tuya_ipc_dp_utils.c |
| 236 | `FUN_0007bca0` | 0x7bca0 | `IPC_APP_set_ipc_lullaby_command` | tuya_ipc_dp_utils.c |
| 238 | `FUN_0007bd5c` | 0x7bd5c | `IPC_APP_get_temp_report_c` | tuya_ipc_dp_utils.c |
| 239 | `FUN_0007c144` | 0x7c144 | `IPC_APP_get_temp_report_f` | tuya_ipc_dp_utils.c |
| 240 | `FUN_0007c1b8` | 0x7c1b8 | `IPC_APP_set_temp_unit_select`, `IPC_APP_get_temp_unit_select` | tuya_ipc_dp_utils.c |
| 241 | `FUN_0007c278` | 0x7c278 | `IPC_APP_set_temp_max_onoff`, `IPC_APP_get_temp_max_onoff` | tuya_ipc_dp_utils.c |
| 242 | `FUN_0007c2b8` | 0x7c2b8 | `IPC_APP_set_temp_min_onoff`, `IPC_APP_get_temp_min_onoff` | tuya_ipc_dp_utils.c |
| 243 | `FUN_0007c2f8` | 0x7c2f8 | `IPC_APP_set_temp_max_cvalue`, `IPC_APP_get_temp_max_cvalue` | tuya_ipc_dp_utils.c |
| 244 | `FUN_0007c3c8` | 0x7c3c8 | `IPC_APP_set_temp_min_cvalue`, `IPC_APP_get_temp_min_cvalue` | tuya_ipc_dp_utils.c |
| 245 | `FUN_0007c498` | 0x7c498 | `IPC_APP_set_temp_max_fvalue`, `IPC_APP_get_temp_max_fvalue` | tuya_ipc_dp_utils.c |
| 246 | `FUN_0007c568` | 0x7c568 | `IPC_APP_set_temp_min_fvalue`, `IPC_APP_get_temp_min_fvalue` | tuya_ipc_dp_utils.c |
| 247 | `FUN_0007c638` | 0x7c638 | `IPC_APP_set_audible_alarm_onoff`, `IPC_APP_get_audible_alarm_onoff` | tuya_ipc_dp_utils.c |
| 248 | `FUN_0007c678` | 0x7c678 | `IPC_APP_set_door_control`, `IPC_APP_get_door_control` | tuya_ipc_dp_utils.c |
| 167 | `FUN_0007c840` | 0x7c840 | `IPC_APP_set_cry_detect_onoff`, `IPC_APP_get_cry_detect_onoff` | tuya_ipc_dp_utils.c |
| 152 | `FUN_0007c880` | 0x7c880 | `IPC_APP_set_pir_detect_switch`, `IPC_APP_get_pir_detect_switch` | tuya_ipc_dp_utils.c |
| 233 | `FUN_0007c940` | 0x7c940 | — | tuya_ipc_dp_utils.c |
| 172 | `FUN_0007ca18` | 0x7ca18 | `IPC_APP_set_flight_warn_onoff`, `IPC_APP_get_flight_warn_onoff` | tuya_ipc_dp_utils.c |
| 173 | `FUN_0007ca58` | 0x7ca58 | `IPC_APP_set_flight_warn_time`, `IPC_APP_get_flight_warn_time` | tuya_ipc_dp_utils.c |
| 198 | `FUN_0007cb00` | 0x7cb00 | `IPC_APP_set_object_outline_onoff`, `IPC_APP_get_object_outline_onoff` | tuya_ipc_dp_utils.c |
