/* Généré par tools/gateway/gen_gateway.py : ne pas éditer. */
#ifndef DP_TABLE_H
#define DP_TABLE_H

enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR, DP_JSON };
/* Passage de la valeur (r0) : ABI_INT entier, ABI_STR chaîne, ABI_NONE sans argument / sans lecture ;
   lecture seulement : ABI_OUT = la fonction écrit un entier via un pointeur (r0). */
enum dp_abi { ABI_NONE, ABI_INT, ABI_STR, ABI_OUT };

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

/* Symboles exportés par anyka_ipc (nm -D) ; faibles : absents = NULL, jamais appelés. */
extern long IPC_APP_set_light_onoff(void) __attribute__((weak));
extern long IPC_APP_get_light_onoff(void) __attribute__((weak));
extern long IPC_APP_set_flip_onoff(void) __attribute__((weak));
extern long IPC_APP_get_flip_onoff(void) __attribute__((weak));
extern long IPC_APP_set_watermark_onoff(void) __attribute__((weak));
extern long IPC_APP_get_watermark_onoff(void) __attribute__((weak));
extern long IPC_APP_set_alarm_sensitivity(void) __attribute__((weak));
extern long IPC_APP_get_alarm_sensitivity(void) __attribute__((weak));
extern long IPC_APP_set_night_mode(void) __attribute__((weak));
extern long IPC_APP_get_night_mode(void) __attribute__((weak));
extern long IPC_APP_get_sd_status(void) __attribute__((weak));
extern long IPC_APP_ptz_stop_move(void) __attribute__((weak));
extern long IPC_APP_get_sd_format_status(void) __attribute__((weak));
extern long IPC_APP_ptz_start_move(void) __attribute__((weak));
extern long IPC_APP_set_alarm_siren_onoff(void) __attribute__((weak));
extern long IPC_APP_get_alarm_siren_onoff(void) __attribute__((weak));
extern long IPC_APP_set_night_vision_mode(void) __attribute__((weak));
extern long IPC_APP_get_night_vision_mode(void) __attribute__((weak));
extern long IPC_APP_ptz_check(void) __attribute__((weak));
extern long IPC_APP_set_alarm_function_onoff(void) __attribute__((weak));
extern long IPC_APP_get_alarm_function_onoff(void) __attribute__((weak));
extern long IPC_APP_set_blub_onoff(void) __attribute__((weak));
extern long IPC_APP_get_blub_onoff(void) __attribute__((weak));
extern long IPC_APP_set_sound_detect_onoff(void) __attribute__((weak));
extern long IPC_APP_get_sound_detect_onoff(void) __attribute__((weak));
extern long IPC_APP_set_sound_sensitivity(void) __attribute__((weak));
extern long IPC_APP_get_sound_sensitivity(void) __attribute__((weak));
extern long IPC_APP_get_battery_percent(void) __attribute__((weak));
extern long IPC_APP_get_power_mode(void) __attribute__((weak));
extern long IPC_APP_set_pir_detect_switch(void) __attribute__((weak));
extern long IPC_APP_get_pir_detect_switch(void) __attribute__((weak));
extern long IPC_APP_set_floodlight_lightness(void) __attribute__((weak));
extern long IPC_APP_get_floodlight_lightness(void) __attribute__((weak));
extern long IPC_APP_set_siren_onoff(void) __attribute__((weak));
extern long IPC_APP_get_siren_onoff(void) __attribute__((weak));
extern long IPC_APP_set_ipc_lullaby_volume(void) __attribute__((weak));
extern long IPC_APP_get_ipc_lullaby_volume(void) __attribute__((weak));
extern long IPC_APP_track_enable(void) __attribute__((weak));
extern long IPC_APP_get_track_enable(void) __attribute__((weak));
extern long IPC_APP_set_system_reboot(void) __attribute__((weak));
extern long IPC_APP_set_cry_detect_onoff(void) __attribute__((weak));
extern long IPC_APP_get_cry_detect_onoff(void) __attribute__((weak));
extern long IPC_APP_set_alarm_zone_onoff(void) __attribute__((weak));
extern long IPC_APP_get_alarm_zone_onoff(void) __attribute__((weak));
extern long IPC_APP_set_alarm_zone_draw(void) __attribute__((weak));
extern long IPC_APP_get_alarm_zone_draw(void) __attribute__((weak));
extern long IPC_APP_set_human_filter(void) __attribute__((weak));
extern long IPC_APP_get_human_filter(void) __attribute__((weak));
extern long IPC_APP_set_flight_warn_onoff(void) __attribute__((weak));
extern long IPC_APP_get_flight_warn_onoff(void) __attribute__((weak));
extern long IPC_APP_set_flight_warn_time(void) __attribute__((weak));
extern long IPC_APP_get_flight_warn_time(void) __attribute__((weak));
extern long IPC_APP_set_patrol_switch(void) __attribute__((weak));
extern long IPC_APP_set_patrol_mode(void) __attribute__((weak));
extern long IPC_APP_get_patrol_mode(void) __attribute__((weak));
extern long IPC_APP_set_patrol_tmode(void) __attribute__((weak));
extern long IPC_APP_set_patrol_time(void) __attribute__((weak));
extern long IPC_APP_set_preset(void) __attribute__((weak));
extern long IPC_APP_patrol_state(void) __attribute__((weak));
extern long IPC_APP_set_basic_anti_flicker(void) __attribute__((weak));
extern long IPC_APP_get_basic_anti_flicker(void) __attribute__((weak));
extern long IPC_APP_set_link_move(void) __attribute__((weak));
extern long IPC_APP_set_object_outline_onoff(void) __attribute__((weak));
extern long IPC_APP_get_object_outline_onoff(void) __attribute__((weak));
extern long IPC_APP_set_link_pos(void) __attribute__((weak));
extern long IPC_APP_set_ipc_lullaby_mode(void) __attribute__((weak));
extern long IPC_APP_get_ipc_lullaby_mode(void) __attribute__((weak));
extern long IPC_APP_set_ipc_lullaby_control(void) __attribute__((weak));
extern long IPC_APP_get_ipc_lullaby_control(void) __attribute__((weak));
extern long IPC_APP_set_ipc_lullaby_list(void) __attribute__((weak));
extern long IPC_APP_get_ipc_lullaby_list(void) __attribute__((weak));
extern long IPC_APP_set_ipc_lullaby_command(void) __attribute__((weak));
extern long IPC_APP_get_temp_report_c(void) __attribute__((weak));
extern long IPC_APP_get_temp_report_f(void) __attribute__((weak));
extern long IPC_APP_set_temp_unit_select(void) __attribute__((weak));
extern long IPC_APP_get_temp_unit_select(void) __attribute__((weak));
extern long IPC_APP_set_temp_max_onoff(void) __attribute__((weak));
extern long IPC_APP_get_temp_max_onoff(void) __attribute__((weak));
extern long IPC_APP_set_temp_min_onoff(void) __attribute__((weak));
extern long IPC_APP_get_temp_min_onoff(void) __attribute__((weak));
extern long IPC_APP_set_temp_max_cvalue(void) __attribute__((weak));
extern long IPC_APP_get_temp_max_cvalue(void) __attribute__((weak));
extern long IPC_APP_set_temp_min_cvalue(void) __attribute__((weak));
extern long IPC_APP_get_temp_min_cvalue(void) __attribute__((weak));
extern long IPC_APP_set_temp_max_fvalue(void) __attribute__((weak));
extern long IPC_APP_get_temp_max_fvalue(void) __attribute__((weak));
extern long IPC_APP_set_temp_min_fvalue(void) __attribute__((weak));
extern long IPC_APP_get_temp_min_fvalue(void) __attribute__((weak));
extern long IPC_APP_set_audible_alarm_onoff(void) __attribute__((weak));
extern long IPC_APP_get_audible_alarm_onoff(void) __attribute__((weak));
extern long IPC_APP_set_door_control(void) __attribute__((weak));
extern long IPC_APP_get_door_control(void) __attribute__((weak));

static const struct dp_entry dp_table[] = {
  {101, "light_onoff", DP_BOOL, "IPC_APP_set_light_onoff", "IPC_APP_get_light_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_light_onoff, IPC_APP_get_light_onoff},
  {103, "flip_onoff", DP_BOOL, "IPC_APP_set_flip_onoff", "IPC_APP_get_flip_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_flip_onoff, IPC_APP_get_flip_onoff},
  {104, "watermark_onoff", DP_BOOL, "IPC_APP_set_watermark_onoff", "IPC_APP_get_watermark_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_watermark_onoff, IPC_APP_get_watermark_onoff},
  {105, "sleep_mode", DP_BOOL, "IPC_APP_set_sleep_mode", "IPC_APP_get_sleep_mode", ABI_INT, ABI_INT, 0, 0, 0, 0},
  {106, "alarm_sensitivity", DP_ENUM, "IPC_APP_set_alarm_sensitivity", "IPC_APP_get_alarm_sensitivity", ABI_STR, ABI_STR, "0|1|2", 1, IPC_APP_set_alarm_sensitivity, IPC_APP_get_alarm_sensitivity},
  {108, "night_mode", DP_ENUM, "IPC_APP_set_night_mode", "IPC_APP_get_night_mode", ABI_STR, ABI_STR, "0|1|2", 1, IPC_APP_set_night_mode, IPC_APP_get_night_mode},
  {109, "sd_storage", DP_SENSOR, 0, "IPC_APP_get_sd_storage", ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {110, "sd_status", DP_SENSOR, 0, "IPC_APP_get_sd_status", ABI_NONE, ABI_INT, 0, 1, 0, IPC_APP_get_sd_status},
  {111, "format_sd_card", DP_ACTION, "IPC_APP_format_sd_card", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {112, "unmount_sd_card", DP_ACTION, "IPC_APP_unmount_sd_card", 0, ABI_NONE, ABI_NONE, 0, 0, 0, 0},
  {116, "ptz_stop_move", DP_ACTION, "IPC_APP_ptz_stop_move", 0, ABI_NONE, ABI_NONE, 0, 1, IPC_APP_ptz_stop_move, 0},
  {117, "sd_format_status", DP_SENSOR, 0, "IPC_APP_get_sd_format_status", ABI_NONE, ABI_INT, 0, 1, 0, IPC_APP_get_sd_format_status},
  {119, "ptz_start_move", DP_ACTION, "IPC_APP_ptz_start_move", 0, ABI_STR, ABI_NONE, "0|2|4|6", 1, IPC_APP_ptz_start_move, 0},
  {120, "alarm_siren_onoff", DP_BOOL, "IPC_APP_set_alarm_siren_onoff", "IPC_APP_get_alarm_siren_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_alarm_siren_onoff, IPC_APP_get_alarm_siren_onoff},
  {124, "night_vision_mode", DP_ENUM, "IPC_APP_set_night_vision_mode", "IPC_APP_get_night_vision_mode", ABI_STR, ABI_STR, "0|1", 1, IPC_APP_set_night_vision_mode, IPC_APP_get_night_vision_mode},
  {132, "ptz_check", DP_ACTION, "IPC_APP_ptz_check", 0, ABI_NONE, ABI_NONE, 0, 1, IPC_APP_ptz_check, 0},
  {134, "alarm_function_onoff", DP_BOOL, "IPC_APP_set_alarm_function_onoff", "IPC_APP_get_alarm_function_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_alarm_function_onoff, IPC_APP_get_alarm_function_onoff},
  {138, "blub_onoff", DP_BOOL, "IPC_APP_set_blub_onoff", "IPC_APP_get_blub_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_blub_onoff, IPC_APP_get_blub_onoff},
  {139, "sound_detect_onoff", DP_BOOL, "IPC_APP_set_sound_detect_onoff", "IPC_APP_get_sound_detect_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_sound_detect_onoff, IPC_APP_get_sound_detect_onoff},
  {140, "sound_sensitivity", DP_ENUM, "IPC_APP_set_sound_sensitivity", "IPC_APP_get_sound_sensitivity", ABI_STR, ABI_STR, "0|1", 1, IPC_APP_set_sound_sensitivity, IPC_APP_get_sound_sensitivity},
  {145, "battery_percent", DP_SENSOR, 0, "IPC_APP_get_battery_percent", ABI_NONE, ABI_INT, 0, 1, 0, IPC_APP_get_battery_percent},
  {146, "power_mode", DP_SENSOR, 0, "IPC_APP_get_power_mode", ABI_NONE, ABI_STR, 0, 1, 0, IPC_APP_get_power_mode},
  {150, "sd_record_onoff", DP_BOOL, "IPC_APP_set_sd_record_onoff", "IPC_APP_get_sd_record_onoff", ABI_INT, ABI_INT, 0, 0, 0, 0},
  {151, "sd_record_mode", DP_ENUM, "IPC_APP_set_sd_record_mode", "IPC_APP_get_sd_record_mode", ABI_INT, ABI_INT, "0|1", 0, 0, 0},
  {152, "pir_detect_switch", DP_ENUM, "IPC_APP_set_pir_detect_switch", "IPC_APP_get_pir_detect_switch", ABI_STR, ABI_STR, "0|1|2|3", 1, IPC_APP_set_pir_detect_switch, IPC_APP_get_pir_detect_switch},
  {158, "floodlight_lightness", DP_INT, "IPC_APP_set_floodlight_lightness", "IPC_APP_get_floodlight_lightness", ABI_INT, ABI_INT, "1..100", 1, IPC_APP_set_floodlight_lightness, IPC_APP_get_floodlight_lightness},
  {159, "siren_onoff", DP_BOOL, "IPC_APP_set_siren_onoff", "IPC_APP_get_siren_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_siren_onoff, IPC_APP_get_siren_onoff},
  {160, "ipc_lullaby_volume", DP_INT, "IPC_APP_set_ipc_lullaby_volume", "IPC_APP_get_ipc_lullaby_volume", ABI_INT, ABI_INT, "0..10", 1, IPC_APP_set_ipc_lullaby_volume, IPC_APP_get_ipc_lullaby_volume},
  {161, "track_enable", DP_BOOL, "IPC_APP_track_enable", "IPC_APP_get_track_enable", ABI_INT, ABI_INT, 0, 1, IPC_APP_track_enable, IPC_APP_get_track_enable},
  {162, "system_reboot", DP_ACTION, "IPC_APP_set_system_reboot", 0, ABI_INT, ABI_NONE, "1", 1, IPC_APP_set_system_reboot, 0},
  {167, "cry_detect_onoff", DP_BOOL, "IPC_APP_set_cry_detect_onoff", "IPC_APP_get_cry_detect_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_cry_detect_onoff, IPC_APP_get_cry_detect_onoff},
  {168, "alarm_zone_onoff", DP_BOOL, "IPC_APP_set_alarm_zone_onoff", "IPC_APP_get_alarm_zone_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_alarm_zone_onoff, IPC_APP_get_alarm_zone_onoff},
  {169, "alarm_zone_draw", DP_JSON, "IPC_APP_set_alarm_zone_draw", "IPC_APP_get_alarm_zone_draw", ABI_STR, ABI_STR, 0, 1, IPC_APP_set_alarm_zone_draw, IPC_APP_get_alarm_zone_draw},
  {170, "human_filter", DP_BOOL, "IPC_APP_set_human_filter", "IPC_APP_get_human_filter", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_human_filter, IPC_APP_get_human_filter},
  {172, "flight_warn_onoff", DP_BOOL, "IPC_APP_set_flight_warn_onoff", "IPC_APP_get_flight_warn_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_flight_warn_onoff, IPC_APP_get_flight_warn_onoff},
  {173, "flight_warn_time", DP_INT, "IPC_APP_set_flight_warn_time", "IPC_APP_get_flight_warn_time", ABI_INT, ABI_INT, "0..3600", 1, IPC_APP_set_flight_warn_time, IPC_APP_get_flight_warn_time},
  {174, "patrol_switch", DP_BOOL, "IPC_APP_set_patrol_switch", 0, ABI_INT, ABI_NONE, 0, 1, IPC_APP_set_patrol_switch, 0},
  {175, "patrol_mode", DP_ENUM, "IPC_APP_set_patrol_mode", "IPC_APP_get_patrol_mode", ABI_STR, ABI_STR, "0|1", 1, IPC_APP_set_patrol_mode, IPC_APP_get_patrol_mode},
  {176, "patrol_tmode", DP_INT, "IPC_APP_set_patrol_tmode", 0, ABI_INT, ABI_NONE, "0..255", 1, IPC_APP_set_patrol_tmode, 0},
  {177, "patrol_time", DP_INT, "IPC_APP_set_patrol_time", 0, ABI_INT, ABI_NONE, "0..86400", 1, IPC_APP_set_patrol_time, 0},
  {178, "preset", DP_JSON, "IPC_APP_set_preset", 0, ABI_STR, ABI_NONE, 0, 1, IPC_APP_set_preset, 0},
  {179, "patrol_state", DP_SENSOR, 0, "IPC_APP_patrol_state", ABI_NONE, ABI_OUT, 0, 1, 0, IPC_APP_patrol_state},
  {188, "basic_anti_flicker", DP_ENUM, "IPC_APP_set_basic_anti_flicker", "IPC_APP_get_basic_anti_flicker", ABI_STR, ABI_STR, "0|1|2", 1, IPC_APP_set_basic_anti_flicker, IPC_APP_get_basic_anti_flicker},
  {190, "link_move", DP_INT, "IPC_APP_set_link_move", 0, ABI_INT, ABI_NONE, "0..255", 1, IPC_APP_set_link_move, 0},
  {198, "object_outline_onoff", DP_BOOL, "IPC_APP_set_object_outline_onoff", "IPC_APP_get_object_outline_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_object_outline_onoff, IPC_APP_get_object_outline_onoff},
  {199, "link_pos", DP_INT, "IPC_APP_set_link_pos", 0, ABI_INT, ABI_NONE, "0..255", 1, IPC_APP_set_link_pos, 0},
  {232, "ipc_lullaby_mode", DP_ENUM, "IPC_APP_set_ipc_lullaby_mode", "IPC_APP_get_ipc_lullaby_mode", ABI_STR, ABI_STR, "0|1|2", 1, IPC_APP_set_ipc_lullaby_mode, IPC_APP_get_ipc_lullaby_mode},
  {234, "ipc_lullaby_control", DP_ENUM, "IPC_APP_set_ipc_lullaby_control", "IPC_APP_get_ipc_lullaby_control", ABI_STR, ABI_STR, "0|1", 1, IPC_APP_set_ipc_lullaby_control, IPC_APP_get_ipc_lullaby_control},
  {235, "ipc_lullaby_list", DP_ENUM, "IPC_APP_set_ipc_lullaby_list", "IPC_APP_get_ipc_lullaby_list", ABI_STR, ABI_STR, "0|1|2|3|4", 1, IPC_APP_set_ipc_lullaby_list, IPC_APP_get_ipc_lullaby_list},
  {236, "ipc_lullaby_command", DP_ENUM, "IPC_APP_set_ipc_lullaby_command", 0, ABI_STR, ABI_NONE, "0|1", 1, IPC_APP_set_ipc_lullaby_command, 0},
  {238, "temp_report_c", DP_SENSOR, 0, "IPC_APP_get_temp_report_c", ABI_NONE, ABI_OUT, 0, 1, 0, IPC_APP_get_temp_report_c},
  {239, "temp_report_f", DP_SENSOR, 0, "IPC_APP_get_temp_report_f", ABI_NONE, ABI_OUT, 0, 1, 0, IPC_APP_get_temp_report_f},
  {240, "temp_unit_select", DP_ENUM, "IPC_APP_set_temp_unit_select", "IPC_APP_get_temp_unit_select", ABI_STR, ABI_STR, "0|1", 1, IPC_APP_set_temp_unit_select, IPC_APP_get_temp_unit_select},
  {241, "temp_max_onoff", DP_BOOL, "IPC_APP_set_temp_max_onoff", "IPC_APP_get_temp_max_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_temp_max_onoff, IPC_APP_get_temp_max_onoff},
  {242, "temp_min_onoff", DP_BOOL, "IPC_APP_set_temp_min_onoff", "IPC_APP_get_temp_min_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_temp_min_onoff, IPC_APP_get_temp_min_onoff},
  {243, "temp_max_cvalue", DP_INT, "IPC_APP_set_temp_max_cvalue", "IPC_APP_get_temp_max_cvalue", ABI_INT, ABI_INT, "0..500", 1, IPC_APP_set_temp_max_cvalue, IPC_APP_get_temp_max_cvalue},
  {244, "temp_min_cvalue", DP_INT, "IPC_APP_set_temp_min_cvalue", "IPC_APP_get_temp_min_cvalue", ABI_INT, ABI_INT, "0..500", 1, IPC_APP_set_temp_min_cvalue, IPC_APP_get_temp_min_cvalue},
  {245, "temp_max_fvalue", DP_INT, "IPC_APP_set_temp_max_fvalue", "IPC_APP_get_temp_max_fvalue", ABI_INT, ABI_INT, "320..1220", 1, IPC_APP_set_temp_max_fvalue, IPC_APP_get_temp_max_fvalue},
  {246, "temp_min_fvalue", DP_INT, "IPC_APP_set_temp_min_fvalue", "IPC_APP_get_temp_min_fvalue", ABI_INT, ABI_INT, "320..1220", 1, IPC_APP_set_temp_min_fvalue, IPC_APP_get_temp_min_fvalue},
  {247, "audible_alarm_onoff", DP_BOOL, "IPC_APP_set_audible_alarm_onoff", "IPC_APP_get_audible_alarm_onoff", ABI_INT, ABI_INT, 0, 1, IPC_APP_set_audible_alarm_onoff, IPC_APP_get_audible_alarm_onoff},
  {248, "door_control", DP_ENUM, "IPC_APP_set_door_control", "IPC_APP_get_door_control", ABI_STR, ABI_STR, "0|1|2|3|4|5|6", 1, IPC_APP_set_door_control, IPC_APP_get_door_control},
};

#define DP_TABLE_LEN 61
#define DP_EXPOSED_LEN 55

#endif
