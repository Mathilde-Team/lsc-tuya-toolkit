# Commandes connues du firmware (anyka_ipc 3.10.78.1269)

Liste obtenue **par désassemblage / table des symboles** de `anyka_ipc` (`tools/firmware/gen_commands.py`, `arm-linux-gnueabi-nm -D` + `objdump -d`).
Toutes les fonctions listées sont des symboles `T` exportés : **appelables depuis une bibliothèque `LD_PRELOAD`** (comme `md_inject.so`).

Colonnes :

- **Args** : nombre de registres r0–r3 sauvegardés dans le prologue (≈ nombre d'arguments, approximation).
- **Rôle** : en *gras-italique* = confirmé par lecture du désassemblage ; sinon simple lecture du nom (**hypothèse**).
- **Valeurs** : valeurs attendues ; déduites des appels internes (`atoi`, `ht_tuya_config_set_*`) sauf si confirmées.
- **Exposé** : où la fonction est atteignable aujourd'hui (MQTT, ONVIF, `hack.conf`) ; « non » = seulement par appel direct.
- **Utilisable** : « testé » = essayée sur la caméra ; sinon « non testé ».

**Principe : toute affirmation sur le firmware doit venir du désassemblage** (`objdump -d`), pas d'une supposition.

## Commandes applicatives Tuya (`IPC_APP_*`) (147)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `IPC_APP_Get_Mqtt_Status` | 0x00084234 | 2 | Get Mqtt Status | non analysé | non | non testé |
| `IPC_APP_Init_SDK` | 0x00084360 | 4 | Init SDK | non analysé | non | non testé |
| `IPC_APP_Notify_LED_Sound_Status_CB` | 0x00081058 | 1 | Notify LED Sound Status CB | non analysé | non | non testé |
| `IPC_APP_Reset_System_CB` | 0x000807b8 | 2 | Reset System CB | non analysé | non | non testé |
| `IPC_APP_Restart_Process_CB` | 0x000808c8 | 1 | Restart Process CB | non analysé | non | non testé |
| `IPC_APP_Set_Media_Info` | 0x0007d6c4 | 0 | Set Media Info | non analysé | non | non testé |
| `IPC_APP_Show_OSD_Time` | 0x0008142c | 0 | Show OSD Time | non analysé | non | non testé |
| `IPC_APP_Sync_Utc_Time` | 0x00081180 | 0 | Sync Utc Time | non analysé | non | non testé |
| `IPC_APP_Upgrade_Firmware_CB` | 0x00080948 | 0 | Upgrade Firmware CB | non analysé | non | non testé |
| `IPC_APP_Upgrade_Inform_cb` | 0x00080d24 | 3 | Upgrade Inform cb | non analysé | non | non testé |
| `IPC_APP_format_sd_card` | 0x00076420 | 1 | format sd card | non analysé | non | non testé |
| `IPC_APP_get_alarm_function_onoff` | 0x00075268 | 1 | Lit alarm function onoff | non analysé | non | non testé |
| `IPC_APP_get_alarm_sensitivity` | 0x0007537c | 0 | Lit alarm sensitivity | non analysé | non | non testé |
| `IPC_APP_get_alarm_siren_onoff` | 0x0007549c | 0 | Lit alarm siren onoff | non analysé | non | non testé |
| `IPC_APP_get_alarm_zone_draw` | 0x00075d00 | 1 | Lit alarm zone draw | non analysé | non | non testé |
| `IPC_APP_get_alarm_zone_onoff` | 0x00075680 | 0 | Lit alarm zone onoff | non analysé | non | non testé |
| `IPC_APP_get_audible_alarm_onoff` | 0x0007948c | 0 | Lit audible alarm onoff | non analysé | non | non testé |
| `IPC_APP_get_basic_anti_flicker` | 0x00078488 | 1 | Lit basic anti flicker | non analysé | non | non testé |
| `IPC_APP_get_battery_percent` | 0x00078350 | 0 | Lit battery percent | non analysé | non | non testé |
| `IPC_APP_get_blub_onoff` | 0x00078324 | 0 | Lit blub onoff | non analysé | non | non testé |
| `IPC_APP_get_color_ctrl` | 0x00079b54 | 0 | Lit color ctrl | non analysé | non | non testé |
| `IPC_APP_get_cry_detect_onoff` | 0x000798d0 | 1 | Lit cry detect onoff | non analysé | non | non testé |
| `IPC_APP_get_custom_alarm_control` | 0x000797e4 | 1 | Lit custom alarm control | non analysé | non | non testé |
| `IPC_APP_get_door_control` | 0x00079640 | 1 | Lit door control | non analysé | non | non testé |
| `IPC_APP_get_flight_warn_onoff` | 0x00079c48 | 0 | Lit flight warn onoff | non analysé | non | non testé |
| `IPC_APP_get_flight_warn_time` | 0x00079c74 | 0 | Lit flight warn time | non analysé | non | non testé |
| `IPC_APP_get_flip_onoff` | 0x00074b08 | 0 | Lit flip onoff | non analysé | non | non testé |
| `IPC_APP_get_floodlight_lightness` | 0x00078530 | 1 | Lit floodlight lightness | non analysé | non | non testé |
| `IPC_APP_get_human_filter` | 0x00076a50 | 0 | Lit human filter | non analysé | non | non testé |
| `IPC_APP_get_ipc_lullaby_control` | 0x00078924 | 1 | Lit ipc lullaby control | non analysé | non | non testé |
| `IPC_APP_get_ipc_lullaby_list` | 0x00078a98 | 1 | Lit ipc lullaby list | non analysé | non | non testé |
| `IPC_APP_get_ipc_lullaby_mode` | 0x000786e4 | 1 | Lit ipc lullaby mode | non analysé | non | non testé |
| `IPC_APP_get_ipc_lullaby_volume` | 0x0007878c | 1 | Lit ipc lullaby volume | non analysé | non | non testé |
| `IPC_APP_get_light_onoff` | 0x00074a4c | 0 | Lit light onoff | non analysé | non | non testé |
| `IPC_APP_get_md_area_onoff` | 0x000755f4 | 0 | Lit md area onoff | non analysé | non | non testé |
| `IPC_APP_get_md_area_rect` | 0x000757e0 | 4 | Lit md area rect | non analysé | non | non testé |
| `IPC_APP_get_night_mode` | 0x00074ccc | 1 | Lit night mode | non analysé | non | non testé |
| `IPC_APP_get_night_vision_mode` | 0x00074eac | 1 | Lit night vision mode | non analysé | non | non testé |
| `IPC_APP_get_object_outline_onoff` | 0x00079d50 | 0 | Lit object outline onoff | non analysé | non | non testé |
| `IPC_APP_get_patrol_mode` | 0x00076b88 | 1 | Lit patrol mode | non analysé | non | non testé |
| `IPC_APP_get_patrol_switch` | 0x00076d00 | 0 | Lit patrol switch | non analysé | non | non testé |
| `IPC_APP_get_patrol_tmode` | 0x00077148 | 0 | Lit patrol tmode | non analysé | non | non testé |
| `IPC_APP_get_pir_detect_switch` | 0x00079a38 | 1 | Lit pir detect switch | non analysé | non | non testé |
| `IPC_APP_get_power_mode` | 0x0007836c | 0 | Lit power mode | non analysé | non | non testé |
| `IPC_APP_get_res_change` | 0x00079b78 | 0 | Lit res change | non analysé | non | non testé |
| `IPC_APP_get_sd_format_status` | 0x00076514 | 0 | Lit sd format status | non analysé | non | non testé |
| `IPC_APP_get_sd_record_mode` | 0x00075fdc | 0 | Lit sd record mode | non analysé | non | non testé |
| `IPC_APP_get_sd_record_onoff` | 0x00075f08 | 0 | Lit sd record onoff | non analysé | non | non testé |
| `IPC_APP_get_sd_status` | 0x00075de8 | 0 | Lit sd status | non analysé | non | non testé |
| `IPC_APP_get_sd_storage` | 0x00075e00 | 4 | Lit sd storage | non analysé | non | non testé |
| `IPC_APP_get_siren_onoff` | 0x00078248 | 0 | Lit siren onoff | non analysé | non | non testé |
| `IPC_APP_get_sleep_mode` | 0x00074970 | 0 | ***Lit bool_sleep_mode*** | — | non | non testé |
| `IPC_APP_get_sound_detect_onoff` | 0x00075078 | 0 | Lit sound detect onoff | non analysé | non | non testé |
| `IPC_APP_get_sound_sensitivity` | 0x0007513c | 0 | Lit sound sensitivity | non analysé | non | non testé |
| `IPC_APP_get_system_reboot` | 0x00075500 | 0 | Lit system reboot | non analysé | non | non testé |
| `IPC_APP_get_temp_max_cvalue` | 0x00078ed4 | 1 | Lit temp max cvalue | non analysé | non | non testé |
| `IPC_APP_get_temp_max_fvalue` | 0x00079194 | 1 | Lit temp max fvalue | non analysé | non | non testé |
| `IPC_APP_get_temp_max_onoff` | 0x00078e4c | 0 | Lit temp max onoff | non analysé | non | non testé |
| `IPC_APP_get_temp_min_cvalue` | 0x00079034 | 1 | Lit temp min cvalue | non analysé | non | non testé |
| `IPC_APP_get_temp_min_fvalue` | 0x000792f8 | 1 | Lit temp min fvalue | non analysé | non | non testé |
| `IPC_APP_get_temp_min_onoff` | 0x00078ea8 | 0 | Lit temp min onoff | non analysé | non | non testé |
| `IPC_APP_get_temp_report_c` | 0x00078b90 | 1 | Lit temp report c | non analysé | non | non testé |
| `IPC_APP_get_temp_report_f` | 0x00078bd4 | 1 | Lit temp report f | non analysé | non | non testé |
| `IPC_APP_get_temp_unit_select` | 0x00078d80 | 1 | Lit temp unit select | non analysé | non | non testé |
| `IPC_APP_get_track_enable` | 0x0007699c | 0 | Lit track enable | non analysé | non | non testé |
| `IPC_APP_get_watermark_onoff` | 0x00074bc4 | 0 | Lit watermark onoff | non analysé | non | non testé |
| `IPC_APP_handle_dp_cmd_objs` | 0x0007cb90 | 2 | handle dp cmd objs | non analysé | non | non testé |
| `IPC_APP_handle_dp_query_objs` | 0x0007cc98 | 2 | handle dp query objs | non analysé | non | non testé |
| `IPC_APP_patrol_state` | 0x00078000 | 1 | patrol state | non analysé | non | non testé |
| `IPC_APP_ptz_check` | 0x000768b0 | 1 | ptz check | non analysé | non | non testé |
| `IPC_APP_ptz_goto_save_pos` | 0x000767ac | 1 | ptz goto save pos | non analysé | non | non testé |
| `IPC_APP_ptz_preset_reset` | 0x00076f4c | 2 | ptz preset reset | non analysé | non | non testé |
| `IPC_APP_ptz_preset_sync` | 0x00077030 | 0 | ptz preset sync | non analysé | non | non testé |
| `IPC_APP_ptz_save_pos` | 0x00076718 | 0 | ptz save pos | chaîne | non | non testé |
| `IPC_APP_ptz_start_move` | 0x00076558 | 2 | ptz start move | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_ptz_stop_move` | 0x00076858 | 1 | ptz stop move | non analysé | non | non testé |
| `IPC_APP_report_blub_onoff` | 0x0007b630 | 0 | report blub onoff | non analysé | non | non testé |
| `IPC_APP_report_custom_alarm_ctrl` | 0x0007c81c | 1 | report custom alarm ctrl | non analysé | non | non testé |
| `IPC_APP_report_lullaby_control` | 0x0007bb50 | 0 | report lullaby control | non analysé | non | non testé |
| `IPC_APP_report_lullaby_list` | 0x0007bc38 | 0 | report lullaby list | non analysé | non | non testé |
| `IPC_APP_report_sd_format_status` | 0x0007a178 | 1 | report sd format status | non analysé | non | non testé |
| `IPC_APP_report_sd_status_changed` | 0x0007a19c | 1 | report sd status changed | non analysé | non | non testé |
| `IPC_APP_report_sd_storage` | 0x0007a1d4 | 0 | report sd storage | non analysé | non | non testé |
| `IPC_APP_report_siren_switch` | 0x0007aff8 | 1 | report siren switch | non analysé | non | non testé |
| `IPC_APP_report_temp` | 0x0007bdd0 | 1 | report temp | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_report_temp_c` | 0x0007bd98 | 1 | report temp c | non analysé | non | non testé |
| `IPC_APP_report_temp_f` | 0x0007c180 | 1 | report temp f | non analysé | non | non testé |
| `IPC_APP_report_temp_max_cvalue` | 0x0007c3a0 | 0 | report temp max cvalue | non analysé | non | non testé |
| `IPC_APP_report_temp_max_fvalue` | 0x0007c540 | 0 | report temp max fvalue | non analysé | non | non testé |
| `IPC_APP_report_temp_min_cvalue` | 0x0007c470 | 0 | report temp min cvalue | non analysé | non | non testé |
| `IPC_APP_report_temp_min_fvalue` | 0x0007c610 | 0 | report temp min fvalue | non analysé | non | non testé |
| `IPC_APP_set_alarm_function_onoff` | 0x000751d0 | 2 | Règle alarm function onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_alarm_sensitivity` | 0x000752ec | 2 | Règle alarm sensitivity | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_alarm_siren_onoff` | 0x0007541c | 2 | Règle alarm siren onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_alarm_zone_draw` | 0x00075864 | 1 | Règle alarm zone draw | non analysé | non | non testé |
| `IPC_APP_set_alarm_zone_onoff` | 0x00075620 | 2 | Règle alarm zone onoff | non analysé | non | non testé |
| `IPC_APP_set_audible_alarm_onoff` | 0x0007945c | 1 | Règle audible alarm onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_basic_anti_flicker` | 0x00078390 | 2 | Règle basic anti flicker | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_blub_onoff` | 0x00078270 | 2 | Règle blub onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_color_ctrl` | 0x00079aec | 2 | Règle color ctrl | non analysé | non | non testé |
| `IPC_APP_set_cry_detect_onoff` | 0x0007988c | 1 | Règle cry detect onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_custom_alarm_control` | 0x00079718 | 2 | Règle custom alarm control | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_door_control` | 0x000794b8 | 2 | Règle door control | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_flight_warn_onoff` | 0x00079bdc | 2 | Règle flight warn onoff | non analysé | non | non testé |
| `IPC_APP_set_flight_warn_time` | 0x00079ca0 | 2 | Règle flight warn time | entier | non | non testé |
| `IPC_APP_set_flip_onoff` | 0x00074a78 | 2 | Règle flip onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_floodlight_lightness` | 0x00078590 | 2 | Règle floodlight lightness | entier | non | non testé |
| `IPC_APP_set_human_filter` | 0x000769c8 | 2 | Règle human filter | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_ipc_lullaby_command` | 0x00078b2c | 2 | Règle ipc lullaby command | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_ipc_lullaby_control` | 0x00078850 | 2 | Règle ipc lullaby control | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_ipc_lullaby_list` | 0x000789c0 | 2 | Règle ipc lullaby list | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_ipc_lullaby_mode` | 0x00078604 | 2 | Règle ipc lullaby mode | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_ipc_lullaby_volume` | 0x000787ec | 2 | Règle ipc lullaby volume | entier | non | non testé |
| `IPC_APP_set_light_onoff` | 0x0007499c | 2 | Règle light onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_link_move` | 0x000780b4 | 1 | Règle link move | non analysé | non | non testé |
| `IPC_APP_set_link_pos` | 0x00078080 | 1 | Règle link pos | non analysé | non | non testé |
| `IPC_APP_set_md_area_onoff` | 0x0007551c | 2 | Règle md area onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_md_area_rect` | 0x000756e0 | 4 | Règle md area rect | chaîne | non | non testé |
| `IPC_APP_set_night_mode` | 0x00074bf0 | 2 | ***Mode de la LED IR / nuit (clé enum_night_mode)*** | chaîne « 0 »/« 1 »/« 2 » | MQTT <base>/night/set, ONVIF IrCutFilter (via md_inject) | testé |
| `IPC_APP_set_night_vision_mode` | 0x00074dc0 | 2 | ***Vision nocturne IR ou couleur (enum_night_vision_mode)*** | chaîne « 0 »/« 1 » | non | non testé |
| `IPC_APP_set_object_outline_onoff` | 0x00079d20 | 1 | Règle object outline onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_patrol_mode` | 0x00076a7c | 2 | Règle patrol mode | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_patrol_switch` | 0x00076c24 | 2 | Règle patrol switch | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_patrol_time` | 0x00077170 | 1 | Règle patrol time | non analysé | non | non testé |
| `IPC_APP_set_patrol_tmode` | 0x00077114 | 1 | Règle patrol tmode | non analysé | non | non testé |
| `IPC_APP_set_pir_detect_switch` | 0x0007992c | 2 | Règle pir detect switch | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_preset` | 0x00077190 | 1 | Règle preset | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_sd_format_status` | 0x0007653c | 1 | Règle sd format status | non analysé | non | non testé |
| `IPC_APP_set_sd_record_mode` | 0x00075f34 | 2 | Règle sd record mode | entier | non | non testé |
| `IPC_APP_set_sd_record_onoff` | 0x00075e80 | 2 | Règle sd record onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_siren_onoff` | 0x0007817c | 1 | Règle siren onoff | non analysé | non | non testé |
| `IPC_APP_set_sleep_mode` | 0x000748e0 | 1 | ***Veille/privacy cloud : arrête le stockage, l'enregistrement et envoie ht_ptz_set_cmd(9/10)*** | 0 ou 1 | non (dépend du cloud Tuya) | non testé |
| `IPC_APP_set_sound_detect_onoff` | 0x00074fe4 | 2 | Règle sound detect onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_sound_sensitivity` | 0x000750a4 | 2 | Règle sound sensitivity | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_system_reboot` | 0x000754c8 | 1 | Règle system reboot | non analysé | non | non testé |
| `IPC_APP_set_temp_max_cvalue` | 0x00078f34 | 2 | Règle temp max cvalue | entier | non | non testé |
| `IPC_APP_set_temp_max_fvalue` | 0x000791fc | 2 | Règle temp max fvalue | entier | non | non testé |
| `IPC_APP_set_temp_max_onoff` | 0x00078e1c | 1 | Règle temp max onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_temp_min_cvalue` | 0x00079094 | 2 | Règle temp min cvalue | entier | non | non testé |
| `IPC_APP_set_temp_min_fvalue` | 0x00079360 | 2 | Règle temp min fvalue | entier | non | non testé |
| `IPC_APP_set_temp_min_onoff` | 0x00078e78 | 1 | Règle temp min onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_set_temp_unit_select` | 0x00078cb0 | 2 | Règle temp unit select | chaîne numérique (atoi) | non | non testé |
| `IPC_APP_set_watermark_onoff` | 0x00074b34 | 2 | Règle watermark onoff | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_track_enable` | 0x00076900 | 2 | track enable | 0 ou 1 (booléen) | non | non testé |
| `IPC_APP_unmount_sd_card` | 0x00076038 | 1 | unmount sd card | non analysé | non | non testé |
| `IPC_APP_update_battery_status` | 0x0007a278 | 0 | Met à jour battery status | non analysé | non | non testé |
| `IPC_APP_upload_all_status` | 0x00079d7c | 1 | upload all status | non analysé | non | non testé |

## Réglages Tuya (`ht_tuya_func_*`) (18)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_tuya_func_set_basic_anti_flicker` | 0x0006eca4 | 1 | Règle basic anti flicker | non analysé | non | non testé |
| `ht_tuya_func_set_cry_detect` | 0x0006ece4 | 1 | Règle cry detect | non analysé | non | non testé |
| `ht_tuya_func_set_flip` | 0x0006e71c | 2 | Règle flip | non analysé | non | non testé |
| `ht_tuya_func_set_flip_mirror` | 0x0006e690 | 2 | Règle flip mirror | non analysé | non | non testé |
| `ht_tuya_func_set_human_filter` | 0x0006e98c | 1 | Règle human filter | non analysé | non | non testé |
| `ht_tuya_func_set_indicator_light` | 0x0006e3ac | 1 | Règle indicator light | non analysé | non | non testé |
| `ht_tuya_func_set_mirror` | 0x0006e7a8 | 2 | Règle mirror | non analysé | non | non testé |
| `ht_tuya_func_set_montion_detect` | 0x0006e90c | 1 | Règle montion detect | non analysé | non | non testé |
| `ht_tuya_func_set_montion_detect_area` | 0x0006e9ac | 4 | Règle montion detect area | non analysé | non | non testé |
| `ht_tuya_func_set_montion_detect_sensitivity` | 0x0006e93c | 1 | Règle montion detect sensitivity | non analysé | non | non testé |
| `ht_tuya_func_set_night_mode` | 0x0006e864 | 1 | ***Appelle ht_night_mode_set_irled_mode*** | entier 0..2 | indirect (voir IPC_APP_set_night_mode) | non testé |
| `ht_tuya_func_set_night_vision_mode` | 0x0006e884 | 1 | ***Appelle ht_night_mode_set*** | entier | non | non testé |
| `ht_tuya_func_set_object_outline` | 0x0006ed80 | 1 | Règle object outline | non analysé | non | non testé |
| `ht_tuya_func_set_osd` | 0x0006e834 | 1 | Règle osd | non analysé | non | non testé |
| `ht_tuya_func_set_pir_detect` | 0x0006ed00 | 1 | Règle pir detect | non analysé | non | non testé |
| `ht_tuya_func_set_pir_detect_sensitivity` | 0x0006ed30 | 1 | Règle pir detect sensitivity | non analysé | non | non testé |
| `ht_tuya_func_set_sound_detect` | 0x0006e8a4 | 1 | Règle sound detect | non analysé | non | non testé |
| `ht_tuya_func_set_sound_detect_sensitivity` | 0x0006e8d4 | 1 | Règle sound detect sensitivity | non analysé | non | non testé |

## PTZ (`ht_ptz_*`) (17)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_ptz_calibration` | 0x000b15e8 | 0 | ptz calibration | non analysé | non | non testé |
| `ht_ptz_deinit` | 0x000b1194 | 0 | ptz deinit | non analysé | non | non testé |
| `ht_ptz_dump_preset` | 0x000b164c | 1 | ptz dump preset | non analysé | non | non testé |
| `ht_ptz_factory_test` | 0x000b1054 | 0 | ptz factory test | non analysé | non | non testé |
| `ht_ptz_get_current_pos` | 0x000b24d0 | 1 | ptz get current pos | non analysé | non | non testé |
| `ht_ptz_get_patrol_mode` | 0x000b22bc | 1 | ptz get patrol mode | non analysé | non | non testé |
| `ht_ptz_goto_pos` | 0x000b2558 | 2 | ptz goto pos | non analysé | non | non testé |
| `ht_ptz_goto_preset` | 0x000b2720 | 1 | ***Va à une position mémorisée*** | numéro de preset | ONVIF GotoPreset (via ptz_move -p) | testé |
| `ht_ptz_init` | 0x000b0f48 | 1 | ptz init | non analysé | non | non testé |
| `ht_ptz_is_run` | 0x000b14b8 | 0 | ptz is run | non analysé | non | non testé |
| `ht_ptz_set_cmd` | 0x000b11fc | 3 | ***Commande de veille PTZ*** | 9 = entrer en veille, 10 = sortir ; autres ignorées | non | non testé |
| `ht_ptz_set_patrol_mode` | 0x000b21d4 | 1 | ptz set patrol mode | non analysé | non | non testé |
| `ht_ptz_set_preset` | 0x000b17b8 | 3 | ptz set preset | non analysé | non | non testé |
| `ht_ptz_set_preset_stop_time` | 0x000b22f8 | 2 | ptz set preset stop time | non analysé | non | non testé |
| `ht_ptz_start_mt` | 0x000b150c | 2 | ptz start mt | non analysé | non | non testé |
| `ht_ptz_stop_mt` | 0x000b15a4 | 0 | ptz stop mt | non analysé | non | non testé |
| `ht_ptz_update_filp_mirror` | 0x000b11d0 | 0 | ptz update filp mirror | non analysé | non | non testé |

## Vision nocturne (`ht_night_mode_*`) (11)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_night_mode_check_is_night` | 0x000ab4c8 | 0 | ***Vrai si l'état courant est nuit*** | — | non | non testé |
| `ht_night_mode_deinit` | 0x000aba3c | 0 | night mode deinit | non analysé | non | non testé |
| `ht_night_mode_get` | 0x000ab1c0 | 1 | ***Retourne le mode courant (+12)*** | — | non | non testé |
| `ht_night_mode_get_fillled_state` | 0x000aba14 | 0 | night mode get fillled state | non analysé | non | non testé |
| `ht_night_mode_get_state` | 0x000ab44c | 0 | ***Retourne l'état jour/nuit courant*** | — | non | non testé |
| `ht_night_mode_init` | 0x000aae3c | 0 | night mode init | non analysé | non | non testé |
| `ht_night_mode_light_sensor_test` | 0x000aacb4 | 1 | night mode light sensor test | non analysé | non | non testé |
| `ht_night_mode_set` | 0x000aafe0 | 1 | ***Mode « couleur » (+12) et éclairage*** | 0..2 (2 = lumière d'appoint) | non | non testé |
| `ht_night_mode_set_fillled_custom_ctrl_cb` | 0x000ab500 | 1 | night mode set fillled custom ctrl cb | non analysé | non | non testé |
| `ht_night_mode_set_fillled_state` | 0x000ab584 | 1 | night mode set fillled state | non analysé | non | non testé |
| `ht_night_mode_set_irled_mode` | 0x000ab2a8 | 1 | ***Mode jour/nuit : change l'état +16 sous mutex, met la détection en pause*** | 0 auto, 1 jour, 2 nuit forcée ; refusé si mode couleur (+12) = 2 | MQTT <base>/night/set, ONVIF IrCutFilter ON/OFF/AUTO | utilisé |

## Détection de mouvement (`ht_montion_detect_*`) (15)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_montion_detect_close` | 0x000a844c | 1 | montion detect close | non analysé | non | non testé |
| `ht_montion_detect_deinit` | 0x000a8c5c | 0 | montion detect deinit | chaîne numérique (atoi) | non | non testé |
| `ht_montion_detect_get_sensitivity` | 0x000a888c | 1 | montion detect get sensitivity | non analysé | non | non testé |
| `ht_montion_detect_get_status` | 0x000a8c34 | 0 | montion detect get status | non analysé | non | non testé |
| `ht_montion_detect_init` | 0x000a7c48 | 1 | montion detect init | non analysé | non | non testé |
| `ht_montion_detect_open` | 0x000a8364 | 1 | ***Active la détection de mouvement*** | aucun | MQTT <base>/motion (ON/OFF) via md_inject | testé |
| `ht_montion_detect_pause` | 0x000a8b98 | 1 | montion detect pause | non analysé | non | non testé |
| `ht_montion_detect_resume` | 0x000a8bec | 0 | montion detect resume | non analysé | non | non testé |
| `ht_montion_detect_set_active_cb` | 0x000a8b44 | 1 | ***Enregistre la fonction appelée à chaque mouvement*** | pointeur de fonction(long long ms) | MQTT motion | testé |
| `ht_montion_detect_set_area` | 0x000a88cc | 2 | montion detect set area | non analysé | non | non testé |
| `ht_montion_detect_set_human_filter` | 0x000a8820 | 1 | ***Filtre « humain »*** | 0 ou 1 | MOTION_HUMAN (hack.conf) | testé, efficacité non démontrée |
| `ht_montion_detect_set_interval` | 0x000a8a50 | 2 | montion detect set interval | non analysé | non | non testé |
| `ht_montion_detect_set_mode` | 0x000a80a0 | 2 | montion detect set mode | non analysé | non | non testé |
| `ht_montion_detect_set_sensitivity` | 0x000a8680 | 2 | ***Sensibilité de détection*** | 0..2 | MOTION_SENS (hack.conf), pas de commande à chaud | testé |
| `ht_montion_detect_update_sensi_level` | 0x000a853c | 3 | montion detect update sensi level | non analysé | non | non testé |

## Audio (`ht_audio_codec_*`) (25)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_audio_codec_ao_enable` | 0x000a0d48 | 1 | audio codec ao enable | non analysé | non | non testé |
| `ht_audio_codec_deinit` | 0x000a1578 | 0 | audio codec deinit | non analysé | non | non testé |
| `ht_audio_codec_get_ai_volume` | 0x000a0fc4 | 1 | audio codec get ai volume | non analysé | non | non testé |
| `ht_audio_codec_init` | 0x0009f7b4 | 1 | audio codec init | non analysé | non | non testé |
| `ht_audio_codec_play_audio_file` | 0x000a054c | 0 | audio codec play audio file | non analysé | non | non testé |
| `ht_audio_codec_play_talk_data` | 0x000a0cb4 | 3 | audio codec play talk data | non analysé | non | non testé |
| `ht_audio_codec_reset_param` | 0x000a1044 | 4 | audio codec reset param | non analysé | non | non testé |
| `ht_audio_codec_sd_close` | 0x000a12e4 | 0 | audio codec sd close | non analysé | non | non testé |
| `ht_audio_codec_sd_open` | 0x000a12bc | 0 | audio codec sd open | non analysé | non | non testé |
| `ht_audio_codec_sd_set_cb` | 0x000a1524 | 1 | audio codec sd set cb | non analysé | non | non testé |
| `ht_audio_codec_sd_set_interval` | 0x000a1480 | 2 | audio codec sd set interval | non analysé | non | non testé |
| `ht_audio_codec_sd_set_sensitivity` | 0x000a1380 | 2 | audio codec sd set sensitivity | non analysé | non | non testé |
| `ht_audio_codec_sd_update_sensi_level` | 0x000a130c | 3 | audio codec sd update sensi level | non analysé | non | non testé |
| `ht_audio_codec_set_aec_enable` | 0x000a1160 | 2 | audio codec set aec enable | non analysé | non | non testé |
| `ht_audio_codec_set_ai_volume` | 0x000a0f20 | 2 | audio codec set ai volume | non analysé | non | non testé |
| `ht_audio_codec_set_ao_volume` | 0x000a0e20 | 1 | audio codec set ao volume | non analysé | non | non testé |
| `ht_audio_codec_set_get_aenc_cb` | 0x000a11c0 | 3 | audio codec set get aenc cb | non analysé | non | non testé |
| `ht_audio_codec_set_get_ai_cb` | 0x000a1264 | 1 | audio codec set get ai cb | non analysé | non | non testé |
| `ht_audio_codec_start_decode` | 0x000a010c | 3 | audio codec start decode | non analysé | non | non testé |
| `ht_audio_codec_start_encode` | 0x0009fd18 | 3 | audio codec start encode | non analysé | non | non testé |
| `ht_audio_codec_stop_decode` | 0x000a041c | 2 | audio codec stop decode | non analysé | non | non testé |
| `ht_audio_codec_stop_encode` | 0x0009ffe0 | 2 | audio codec stop encode | non analysé | non | non testé |
| `ht_audio_codec_stop_play_audio_file` | 0x000a09ec | 1 | audio codec stop play audio file | non analysé | non | non testé |
| `ht_audio_codec_talk_close` | 0x000a0c38 | 0 | audio codec talk close | non analysé | non | non testé |
| `ht_audio_codec_talk_open` | 0x000a0ab4 | 1 | audio codec talk open | non analysé | non | non testé |

## Vidéo, OSD, codec (`ht_video_*`) (27)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_video_codec_auto_change_fps` | 0x000b6398 | 1 | video codec auto change fps | non analysé | non | non testé |
| `ht_video_codec_config_qrcode` | 0x000b9a94 | 1 | video codec config qrcode | non analysé | non | non testé |
| `ht_video_codec_deinit` | 0x000ba42c | 1 | video codec deinit | non analysé | non | non testé |
| `ht_video_codec_get_auto_day_night_level` | 0x000b91b8 | 3 | video codec get auto day night level | non analysé | non | non testé |
| `ht_video_codec_get_effect` | 0x000b99e8 | 3 | video codec get effect | non analysé | non | non testé |
| `ht_video_codec_get_encode_param` | 0x000b8c84 | 3 | video codec get encode param | non analysé | non | non testé |
| `ht_video_codec_get_vi_mode` | 0x000b9830 | 0 | video codec get vi mode | non analysé | non | non testé |
| `ht_video_codec_init` | 0x000b8268 | 4 | video codec init | non analysé | non | non testé |
| `ht_video_codec_request_idr` | 0x000b9858 | 2 | video codec request idr | non analysé | non | non testé |
| `ht_video_codec_scan_qrcode` | 0x000b9e78 | 3 | video codec scan qrcode | non analysé | non | non testé |
| `ht_video_codec_set_effect` | 0x000b990c | 3 | video codec set effect | non analysé | non | non testé |
| `ht_video_codec_set_encode_param` | 0x000b8b70 | 3 | video codec set encode param | non analysé | non | non testé |
| `ht_video_codec_set_get_venc_cb` | 0x000b9160 | 2 | video codec set get venc cb | non analysé | non | non testé |
| `ht_video_codec_set_human_detect` | 0x000ba234 | 1 | video codec set human detect | non analysé | non | non testé |
| `ht_video_codec_set_human_detect_day_night` | 0x000ba364 | 1 | video codec set human detect day night | non analysé | non | non testé |
| `ht_video_codec_show_detect_box` | 0x000ba39c | 2 | video codec show detect box | non analysé | non | non testé |
| `ht_video_codec_snap_picture` | 0x000b8d98 | 2 | video codec snap picture | non analysé | non | non testé |
| `ht_video_codec_start_encode` | 0x000b84d4 | 3 | video codec start encode | non analysé | non | non testé |
| `ht_video_codec_stop_encode` | 0x000b890c | 2 | video codec stop encode | non analysé | non | non testé |
| `ht_video_codec_switch_mode` | 0x000b971c | 2 | video codec switch mode | non analysé | non | non testé |
| `ht_video_osd_close` | 0x000bbc7c | 0 | video osd close | non analysé | non | non testé |
| `ht_video_osd_deinit` | 0x000bbd04 | 0 | video osd deinit | non analysé | non | non testé |
| `ht_video_osd_init` | 0x000bb9e8 | 4 | video osd init | non analysé | non | non testé |
| `ht_video_osd_open` | 0x000bbc54 | 0 | video osd open | non analysé | non | non testé |
| `ht_video_osd_set_format` | 0x000bbca4 | 3 | video osd set format | non analysé | non | non testé |
| `ht_video_osd_set_string` | 0x000bbdbc | 1 | video osd set string | non analysé | non | non testé |
| `ht_video_osd_th` | 0x000bac78 | 2 | video osd th | non analysé | non | non testé |

## Configuration (`ht_config_*`) (6)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_config_deinit` | 0x00084e44 | 0 | config deinit | non analysé | non | non testé |
| `ht_config_get_int` | 0x00084eb8 | 4 | config get int | non analysé | non | non testé |
| `ht_config_get_string` | 0x0008510c | 4 | config get string | non analysé | non | non testé |
| `ht_config_init` | 0x00084a8c | 2 | config init | non analysé | non | non testé |
| `ht_config_set_int` | 0x00084fb8 | 3 | config set int | non analysé | non | non testé |
| `ht_config_set_string` | 0x00085210 | 3 | config set string | non analysé | non | non testé |

## Système (`ht_system_*`) (10)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_system_call_cb` | 0x0006da08 | 0 | system call cb | non analysé | non | non testé |
| `ht_system_ctrl_blue_led` | 0x000b4b44 | 3 | system ctrl blue led | non analysé | non | non testé |
| `ht_system_ctrl_deinit` | 0x000b4e00 | 0 | system ctrl deinit | chaîne numérique (atoi) | non | non testé |
| `ht_system_ctrl_init` | 0x000b487c | 0 | system ctrl init | non analysé | non | non testé |
| `ht_system_ctrl_red_led` | 0x000b4a38 | 3 | system ctrl red led | non analysé | non | non testé |
| `ht_system_ctrl_state_led` | 0x000b4c54 | 3 | system ctrl state led | non analysé | non | non testé |
| `ht_system_reset` | 0x00071b5c | 0 | system reset | non analysé | non | non testé |
| `ht_system_set_call_cb` | 0x000b4dd0 | 1 | system set call cb | non analysé | non | non testé |
| `ht_system_set_factory_bt_cb` | 0x000b4da0 | 1 | system set factory bt cb | non analysé | non | non testé |
| `ht_system_set_reset_cb` | 0x000b4d70 | 1 | system set reset cb | non analysé | non | non testé |

## Autres `ht_tuya_*` (35)

| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |
|---|---|---|---|---|---|---|
| `ht_tuya_av_capture_start` | 0x00070540 | 0 | tuya av capture start | non analysé | non | non testé |
| `ht_tuya_av_capture_stop` | 0x000708a4 | 0 | tuya av capture stop | non analysé | non | non testé |
| `ht_tuya_baby_monitor_deinit` | 0x0006c210 | 1 | tuya baby monitor deinit | non analysé | non | non testé |
| `ht_tuya_baby_monitor_init` | 0x0006bff0 | 0 | tuya baby monitor init | non analysé | non | non testé |
| `ht_tuya_config_get_boolean` | 0x0006c630 | 2 | tuya config get boolean | non analysé | non | non testé |
| `ht_tuya_config_get_integer` | 0x0006c660 | 3 | tuya config get integer | non analysé | non | non testé |
| `ht_tuya_config_get_string` | 0x0006c6a4 | 4 | tuya config get string | non analysé | non | non testé |
| `ht_tuya_config_set_boolean` | 0x0006c6e4 | 2 | tuya config set boolean | entier | non | non testé |
| `ht_tuya_config_set_integer` | 0x0006c714 | 2 | tuya config set integer | non analysé | non | non testé |
| `ht_tuya_config_set_string` | 0x0006c748 | 2 | tuya config set string | chaîne | non | non testé |
| `ht_tuya_event_cry_cb` | 0x0006dac0 | 0 | tuya event cry cb | non analysé | non | non testé |
| `ht_tuya_event_md_cb` | 0x0006db48 | 0 | tuya event md cb | non analysé | non | non testé |
| `ht_tuya_event_sd_cb` | 0x0006da38 | 0 | tuya event sd cb | non analysé | non | non testé |
| `ht_tuya_event_start` | 0x0006db84 | 0 | tuya event start | non analysé | non | non testé |
| `ht_tuya_event_stop` | 0x0006e0bc | 0 | tuya event stop | non analysé | non | non testé |
| `ht_tuya_get_temperature` | 0x0006c5e8 | 1 | tuya get temperature | non analysé | non | non testé |
| `ht_tuya_service_start` | 0x00070f84 | 3 | tuya service start | non analysé | non | non testé |
| `ht_tuya_service_stop` | 0x000712d0 | 1 | tuya service stop | non analysé | non | non testé |
| `ht_tuya_set_ipc_lullaby_cmd` | 0x0006c4b0 | 1 | tuya set ipc lullaby cmd | entier | non | non testé |
| `ht_tuya_set_ipc_lullaby_control` | 0x0006c334 | 1 | tuya set ipc lullaby control | non analysé | non | non testé |
| `ht_tuya_set_ipc_lullaby_list` | 0x0006c388 | 1 | tuya set ipc lullaby list | non analysé | non | non testé |
| `ht_tuya_set_ipc_lullaby_mode` | 0x0006c2f8 | 1 | tuya set ipc lullaby mode | non analysé | non | non testé |
| `ht_tuya_set_ipc_lullaby_volume` | 0x0006c5c8 | 1 | tuya set ipc lullaby volume | non analysé | non | non testé |
| `ht_tuya_stream_storage_capacity` | 0x0006fd40 | 2 | tuya stream storage capacity | non analysé | non | non testé |
| `ht_tuya_stream_storage_deinit` | 0x0006fa04 | 0 | tuya stream storage deinit | non analysé | non | non testé |
| `ht_tuya_stream_storage_get_sd_is_formating` | 0x0006fdec | 1 | tuya stream storage get sd is formating | non analysé | non | non testé |
| `ht_tuya_stream_storage_init` | 0x0006f7f4 | 0 | tuya stream storage init | non analysé | non | non testé |
| `ht_tuya_stream_storage_reinit` | 0x0006f8fc | 0 | tuya stream storage reinit | non analysé | non | non testé |
| `ht_tuya_stream_storage_set_record_onoff` | 0x0006fdcc | 1 | tuya stream storage set record onoff | non analysé | non | non testé |
| `ht_tuya_stream_storage_tf_detect` | 0x0006fc88 | 0 | tuya stream storage tf detect | non analysé | non | non testé |
| `ht_tuya_stream_storage_tf_format` | 0x0006faec | 0 | tuya stream storage tf format | non analysé | non | non testé |
| `ht_tuya_stream_storage_tf_fs_error` | 0x0006fc40 | 0 | tuya stream storage tf fs error | non analysé | non | non testé |
| `ht_tuya_stream_storage_tf_mounted` | 0x0006fcd0 | 0 | tuya stream storage tf mounted | non analysé | non | non testé |
| `ht_tuya_stream_storage_tf_status` | 0x0006fd18 | 0 | tuya stream storage tf status | non analysé | non | non testé |
| `ht_tuya_uuid_check` | 0x00070f08 | 3 | tuya uuid check | non analysé | non | non testé |


## Graphe d'appels (Ghidra)

`tools/firmware/ExportCalls.java` exporte « appelant / type / appelé » (CALL = appel direct, PTR = adresse de fonction stockée) pour toutes les fonctions de `anyka_ipc`. Il demande Ghidra 12.2 et Java 25. Commande :

`JAVA_HOME=<jdk25> analyzeHeadless <projet> anyka -import anyka_ipc -processor ARM:LE:32:v5t -scriptPath tools/firmware -postScript ExportCalls.java calls.tsv`

L'analyse dure une quinzaine de minutes et donne environ 28 000 liens. Exemples vérifiés :

- `ht_night_mode_set_irled_mode` est appelée par `ht_tuya_func_set_night_mode` et par **`handleRecvData`** (réception de données : piste d'une commande locale à analyser).
- `IPC_APP_set_sleep_mode` est appelée par `ht_tuya_service_start` et `FUN_0007a5e4`.
- `ht_ptz_set_cmd` est appelée par `IPC_APP_ptz_start_move`, `IPC_APP_ptz_stop_move` et `IPC_APP_set_sleep_mode`.
