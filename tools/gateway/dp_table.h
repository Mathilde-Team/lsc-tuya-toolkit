/* Généré par tools/gateway/gen_gateway.py : ne pas éditer. */
#ifndef DP_TABLE_H
#define DP_TABLE_H

enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR };
/* Passage de la valeur (r0) : ABI_INT entier, ABI_STR chaîne, ABI_NONE sans argument / sans lecture. */
enum dp_abi { ABI_NONE, ABI_INT, ABI_STR };

/* Type générique : l'appel réel est casté selon set_abi/get_abi (gateway.c). */
typedef long (*dp_fn)(void);

struct dp_entry {
  int dp;
  const char *name;      /* topic : <base>/<name>[/set] */
  enum dp_kind kind;     /* confirmé si exposed, sinon heuristique */
  const char *setter;    /* IPC_APP_* d'écriture ou NULL */
  const char *getter;    /* IPC_APP_* de lecture ou NULL */
  enum dp_abi set_abi;   /* argument de setter */
  enum dp_abi get_abi;   /* retour de getter */
  const char *values;    /* enum : "a|b|c" ; int : "min..max" ; NULL sinon */
  int exposed;           /* 1 = confirmé par DP_VALUES.md (Exposé = oui) */
  dp_fn set_fn;          /* résolu au chargement (symbole faible), NULL si non exposé */
  dp_fn get_fn;
};

static const struct dp_entry dp_table[] = {
  {101, "light_onoff", DP_BOOL, "IPC_APP_set_light_onoff", "IPC_APP_get_light_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {103, "flip_onoff", DP_BOOL, "IPC_APP_set_flip_onoff", "IPC_APP_get_flip_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {104, "watermark_onoff", DP_BOOL, "IPC_APP_set_watermark_onoff", "IPC_APP_get_watermark_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {105, "sleep_mode", DP_ENUM, "IPC_APP_set_sleep_mode", "IPC_APP_get_sleep_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {106, "alarm_sensitivity", DP_INT, "IPC_APP_set_alarm_sensitivity", "IPC_APP_get_alarm_sensitivity", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {108, "night_mode", DP_ENUM, "IPC_APP_set_night_mode", "IPC_APP_get_night_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {109, "sd_storage", DP_SENSOR, 0, "IPC_APP_get_sd_storage", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {110, "sd_status", DP_SENSOR, 0, "IPC_APP_get_sd_status", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {111, "format_sd_card", DP_ACTION, "IPC_APP_format_sd_card", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {112, "unmount_sd_card", DP_ACTION, "IPC_APP_unmount_sd_card", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {116, "ptz_stop_move", DP_ACTION, "IPC_APP_ptz_stop_move", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {117, "sd_format_status", DP_SENSOR, 0, "IPC_APP_get_sd_format_status", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {119, "ptz_start_move", DP_ACTION, "IPC_APP_ptz_start_move", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {120, "alarm_siren_onoff", DP_BOOL, "IPC_APP_set_alarm_siren_onoff", "IPC_APP_get_alarm_siren_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {124, "night_vision_mode", DP_ENUM, "IPC_APP_set_night_vision_mode", "IPC_APP_get_night_vision_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {132, "ptz_check", DP_ACTION, "IPC_APP_ptz_check", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {134, "alarm_function_onoff", DP_BOOL, "IPC_APP_set_alarm_function_onoff", "IPC_APP_get_alarm_function_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {138, "blub_onoff", DP_BOOL, "IPC_APP_set_blub_onoff", "IPC_APP_get_blub_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {139, "sound_detect_onoff", DP_BOOL, "IPC_APP_set_sound_detect_onoff", "IPC_APP_get_sound_detect_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {140, "sound_sensitivity", DP_INT, "IPC_APP_set_sound_sensitivity", "IPC_APP_get_sound_sensitivity", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {145, "battery_percent", DP_SENSOR, 0, "IPC_APP_get_battery_percent", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {146, "power_mode", DP_SENSOR, 0, "IPC_APP_get_power_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {150, "sd_record_onoff", DP_BOOL, "IPC_APP_set_sd_record_onoff", "IPC_APP_get_sd_record_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {151, "sd_record_mode", DP_ENUM, "IPC_APP_set_sd_record_mode", "IPC_APP_get_sd_record_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {152, "pir_detect_switch", DP_BOOL, "IPC_APP_set_pir_detect_switch", "IPC_APP_get_pir_detect_switch", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {158, "floodlight_lightness", DP_INT, "IPC_APP_set_floodlight_lightness", "IPC_APP_get_floodlight_lightness", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {159, "siren_onoff", DP_BOOL, "IPC_APP_set_siren_onoff", "IPC_APP_get_siren_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {160, "ipc_lullaby_volume", DP_INT, "IPC_APP_set_ipc_lullaby_volume", "IPC_APP_get_ipc_lullaby_volume", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {161, "track_enable", DP_SENSOR, 0, "IPC_APP_get_track_enable", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {162, "system_reboot", DP_INT, "IPC_APP_set_system_reboot", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {167, "cry_detect_onoff", DP_BOOL, "IPC_APP_set_cry_detect_onoff", "IPC_APP_get_cry_detect_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {168, "alarm_zone_onoff", DP_BOOL, "IPC_APP_set_alarm_zone_onoff", "IPC_APP_get_alarm_zone_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {169, "alarm_zone_draw", DP_ENUM, "IPC_APP_set_alarm_zone_draw", "IPC_APP_get_alarm_zone_draw", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {170, "human_filter", DP_INT, "IPC_APP_set_human_filter", "IPC_APP_get_human_filter", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {172, "flight_warn_onoff", DP_BOOL, "IPC_APP_set_flight_warn_onoff", "IPC_APP_get_flight_warn_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {173, "flight_warn_time", DP_INT, "IPC_APP_set_flight_warn_time", "IPC_APP_get_flight_warn_time", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {174, "patrol_switch", DP_BOOL, "IPC_APP_set_patrol_switch", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {175, "patrol_mode", DP_ENUM, "IPC_APP_set_patrol_mode", "IPC_APP_get_patrol_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {176, "patrol_tmode", DP_ENUM, "IPC_APP_set_patrol_tmode", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {177, "patrol_time", DP_INT, "IPC_APP_set_patrol_time", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {178, "preset", DP_INT, "IPC_APP_set_preset", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {179, "patrol_state", DP_SENSOR, 0, 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {188, "basic_anti_flicker", DP_INT, "IPC_APP_set_basic_anti_flicker", "IPC_APP_get_basic_anti_flicker", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {190, "link_move", DP_INT, "IPC_APP_set_link_move", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {198, "object_outline_onoff", DP_BOOL, "IPC_APP_set_object_outline_onoff", "IPC_APP_get_object_outline_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {199, "link_pos", DP_INT, "IPC_APP_set_link_pos", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {232, "ipc_lullaby_mode", DP_ENUM, "IPC_APP_set_ipc_lullaby_mode", "IPC_APP_get_ipc_lullaby_mode", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {234, "ipc_lullaby_control", DP_ENUM, "IPC_APP_set_ipc_lullaby_control", "IPC_APP_get_ipc_lullaby_control", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {235, "ipc_lullaby_list", DP_ENUM, "IPC_APP_set_ipc_lullaby_list", "IPC_APP_get_ipc_lullaby_list", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {236, "ipc_lullaby_command", DP_ENUM, "IPC_APP_set_ipc_lullaby_command", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {238, "temp_report_c", DP_SENSOR, 0, "IPC_APP_get_temp_report_c", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {239, "temp_report_f", DP_SENSOR, 0, "IPC_APP_get_temp_report_f", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {240, "temp_unit_select", DP_ENUM, "IPC_APP_set_temp_unit_select", "IPC_APP_get_temp_unit_select", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {241, "temp_max_onoff", DP_BOOL, "IPC_APP_set_temp_max_onoff", "IPC_APP_get_temp_max_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {242, "temp_min_onoff", DP_BOOL, "IPC_APP_set_temp_min_onoff", "IPC_APP_get_temp_min_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {243, "temp_max_cvalue", DP_INT, "IPC_APP_set_temp_max_cvalue", "IPC_APP_get_temp_max_cvalue", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {244, "temp_min_cvalue", DP_INT, "IPC_APP_set_temp_min_cvalue", "IPC_APP_get_temp_min_cvalue", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {245, "temp_max_fvalue", DP_INT, "IPC_APP_set_temp_max_fvalue", "IPC_APP_get_temp_max_fvalue", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {246, "temp_min_fvalue", DP_INT, "IPC_APP_set_temp_min_fvalue", "IPC_APP_get_temp_min_fvalue", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {247, "audible_alarm_onoff", DP_BOOL, "IPC_APP_set_audible_alarm_onoff", "IPC_APP_get_audible_alarm_onoff", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {248, "door_control", DP_ENUM, "IPC_APP_set_door_control", "IPC_APP_get_door_control", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
};

#define DP_TABLE_LEN 61
#define DP_EXPOSED_LEN 0

#endif
