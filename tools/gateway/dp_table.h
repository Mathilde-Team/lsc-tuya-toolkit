/* Généré par tools/gateway/gen_gateway.py : ne pas éditer. */
#ifndef DP_TABLE_H
#define DP_TABLE_H

/* Convention d'appel ASSUMÉE, à confirmer lot par lot (issues #54 à #58,
 * livrable docs/firmware/DP_VALUES.md) : setter int f(int valeur) ; getter
 * int f(int), appelé avec 0. Le runtime (tools/motion/md_inject.c) n'appelle
 * une entrée que si elle est activée explicitement (DP_ENABLE), donc rien
 * n'est exposé tant que le lot correspondant n'a pas confirmé la ligne. */

enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR };

struct dp_entry {
  int dp;
  const char *name;      /* topic : <base>/<name>[/set] */
  enum dp_kind kind;     /* heuristique, voir DP_VALUES.md */
  int (*set)(int);       /* IPC_APP_set_* ou action ; NULL = lecture seule */
  int (*get)(int);       /* IPC_APP_get_* ; NULL = pas d'état lisible */
};

extern int IPC_APP_format_sd_card(int);
extern int IPC_APP_get_alarm_function_onoff(int);
extern int IPC_APP_get_alarm_sensitivity(int);
extern int IPC_APP_get_alarm_siren_onoff(int);
extern int IPC_APP_get_alarm_zone_draw(int);
extern int IPC_APP_get_alarm_zone_onoff(int);
extern int IPC_APP_get_audible_alarm_onoff(int);
extern int IPC_APP_get_basic_anti_flicker(int);
extern int IPC_APP_get_battery_percent(int);
extern int IPC_APP_get_blub_onoff(int);
extern int IPC_APP_get_cry_detect_onoff(int);
extern int IPC_APP_get_door_control(int);
extern int IPC_APP_get_flight_warn_onoff(int);
extern int IPC_APP_get_flight_warn_time(int);
extern int IPC_APP_get_flip_onoff(int);
extern int IPC_APP_get_floodlight_lightness(int);
extern int IPC_APP_get_human_filter(int);
extern int IPC_APP_get_ipc_lullaby_control(int);
extern int IPC_APP_get_ipc_lullaby_list(int);
extern int IPC_APP_get_ipc_lullaby_mode(int);
extern int IPC_APP_get_ipc_lullaby_volume(int);
extern int IPC_APP_get_light_onoff(int);
extern int IPC_APP_get_night_mode(int);
extern int IPC_APP_get_night_vision_mode(int);
extern int IPC_APP_get_object_outline_onoff(int);
extern int IPC_APP_get_patrol_mode(int);
extern int IPC_APP_get_pir_detect_switch(int);
extern int IPC_APP_get_power_mode(int);
extern int IPC_APP_get_sd_format_status(int);
extern int IPC_APP_get_sd_record_mode(int);
extern int IPC_APP_get_sd_record_onoff(int);
extern int IPC_APP_get_sd_status(int);
extern int IPC_APP_get_sd_storage(int);
extern int IPC_APP_get_siren_onoff(int);
extern int IPC_APP_get_sleep_mode(int);
extern int IPC_APP_get_sound_detect_onoff(int);
extern int IPC_APP_get_sound_sensitivity(int);
extern int IPC_APP_get_temp_max_cvalue(int);
extern int IPC_APP_get_temp_max_fvalue(int);
extern int IPC_APP_get_temp_max_onoff(int);
extern int IPC_APP_get_temp_min_cvalue(int);
extern int IPC_APP_get_temp_min_fvalue(int);
extern int IPC_APP_get_temp_min_onoff(int);
extern int IPC_APP_get_temp_report_c(int);
extern int IPC_APP_get_temp_report_f(int);
extern int IPC_APP_get_temp_unit_select(int);
extern int IPC_APP_get_track_enable(int);
extern int IPC_APP_get_watermark_onoff(int);
extern int IPC_APP_ptz_check(int);
extern int IPC_APP_ptz_start_move(int);
extern int IPC_APP_ptz_stop_move(int);
extern int IPC_APP_set_alarm_function_onoff(int);
extern int IPC_APP_set_alarm_sensitivity(int);
extern int IPC_APP_set_alarm_siren_onoff(int);
extern int IPC_APP_set_alarm_zone_draw(int);
extern int IPC_APP_set_alarm_zone_onoff(int);
extern int IPC_APP_set_audible_alarm_onoff(int);
extern int IPC_APP_set_basic_anti_flicker(int);
extern int IPC_APP_set_blub_onoff(int);
extern int IPC_APP_set_cry_detect_onoff(int);
extern int IPC_APP_set_door_control(int);
extern int IPC_APP_set_flight_warn_onoff(int);
extern int IPC_APP_set_flight_warn_time(int);
extern int IPC_APP_set_flip_onoff(int);
extern int IPC_APP_set_floodlight_lightness(int);
extern int IPC_APP_set_human_filter(int);
extern int IPC_APP_set_ipc_lullaby_command(int);
extern int IPC_APP_set_ipc_lullaby_control(int);
extern int IPC_APP_set_ipc_lullaby_list(int);
extern int IPC_APP_set_ipc_lullaby_mode(int);
extern int IPC_APP_set_ipc_lullaby_volume(int);
extern int IPC_APP_set_light_onoff(int);
extern int IPC_APP_set_link_move(int);
extern int IPC_APP_set_link_pos(int);
extern int IPC_APP_set_night_mode(int);
extern int IPC_APP_set_night_vision_mode(int);
extern int IPC_APP_set_object_outline_onoff(int);
extern int IPC_APP_set_patrol_mode(int);
extern int IPC_APP_set_patrol_switch(int);
extern int IPC_APP_set_patrol_time(int);
extern int IPC_APP_set_patrol_tmode(int);
extern int IPC_APP_set_pir_detect_switch(int);
extern int IPC_APP_set_preset(int);
extern int IPC_APP_set_sd_record_mode(int);
extern int IPC_APP_set_sd_record_onoff(int);
extern int IPC_APP_set_siren_onoff(int);
extern int IPC_APP_set_sleep_mode(int);
extern int IPC_APP_set_sound_detect_onoff(int);
extern int IPC_APP_set_sound_sensitivity(int);
extern int IPC_APP_set_system_reboot(int);
extern int IPC_APP_set_temp_max_cvalue(int);
extern int IPC_APP_set_temp_max_fvalue(int);
extern int IPC_APP_set_temp_max_onoff(int);
extern int IPC_APP_set_temp_min_cvalue(int);
extern int IPC_APP_set_temp_min_fvalue(int);
extern int IPC_APP_set_temp_min_onoff(int);
extern int IPC_APP_set_temp_unit_select(int);
extern int IPC_APP_set_watermark_onoff(int);
extern int IPC_APP_unmount_sd_card(int);

static const struct dp_entry dp_table[] = {
  {101, "light_onoff", DP_BOOL, IPC_APP_set_light_onoff, IPC_APP_get_light_onoff},
  {103, "flip_onoff", DP_BOOL, IPC_APP_set_flip_onoff, IPC_APP_get_flip_onoff},
  {104, "watermark_onoff", DP_BOOL, IPC_APP_set_watermark_onoff, IPC_APP_get_watermark_onoff},
  {105, "sleep_mode", DP_ENUM, IPC_APP_set_sleep_mode, IPC_APP_get_sleep_mode},
  {106, "alarm_sensitivity", DP_INT, IPC_APP_set_alarm_sensitivity, IPC_APP_get_alarm_sensitivity},
  {108, "night_mode", DP_ENUM, IPC_APP_set_night_mode, IPC_APP_get_night_mode},
  {109, "sd_storage", DP_SENSOR, 0, IPC_APP_get_sd_storage},
  {110, "sd_status", DP_SENSOR, 0, IPC_APP_get_sd_status},
  {111, "format_sd_card", DP_ACTION, IPC_APP_format_sd_card, 0},
  {112, "unmount_sd_card", DP_ACTION, IPC_APP_unmount_sd_card, 0},
  {116, "ptz_stop_move", DP_ACTION, IPC_APP_ptz_stop_move, 0},
  {117, "sd_format_status", DP_SENSOR, 0, IPC_APP_get_sd_format_status},
  {119, "ptz_start_move", DP_ACTION, IPC_APP_ptz_start_move, 0},
  {120, "alarm_siren_onoff", DP_BOOL, IPC_APP_set_alarm_siren_onoff, IPC_APP_get_alarm_siren_onoff},
  {124, "night_vision_mode", DP_ENUM, IPC_APP_set_night_vision_mode, IPC_APP_get_night_vision_mode},
  {132, "ptz_check", DP_ACTION, IPC_APP_ptz_check, 0},
  {134, "alarm_function_onoff", DP_BOOL, IPC_APP_set_alarm_function_onoff, IPC_APP_get_alarm_function_onoff},
  {138, "blub_onoff", DP_BOOL, IPC_APP_set_blub_onoff, IPC_APP_get_blub_onoff},
  {139, "sound_detect_onoff", DP_BOOL, IPC_APP_set_sound_detect_onoff, IPC_APP_get_sound_detect_onoff},
  {140, "sound_sensitivity", DP_INT, IPC_APP_set_sound_sensitivity, IPC_APP_get_sound_sensitivity},
  {145, "battery_percent", DP_SENSOR, 0, IPC_APP_get_battery_percent},
  {146, "power_mode", DP_SENSOR, 0, IPC_APP_get_power_mode},
  {150, "sd_record_onoff", DP_BOOL, IPC_APP_set_sd_record_onoff, IPC_APP_get_sd_record_onoff},
  {151, "sd_record_mode", DP_ENUM, IPC_APP_set_sd_record_mode, IPC_APP_get_sd_record_mode},
  {152, "pir_detect_switch", DP_BOOL, IPC_APP_set_pir_detect_switch, IPC_APP_get_pir_detect_switch},
  {158, "floodlight_lightness", DP_INT, IPC_APP_set_floodlight_lightness, IPC_APP_get_floodlight_lightness},
  {159, "siren_onoff", DP_BOOL, IPC_APP_set_siren_onoff, IPC_APP_get_siren_onoff},
  {160, "ipc_lullaby_volume", DP_INT, IPC_APP_set_ipc_lullaby_volume, IPC_APP_get_ipc_lullaby_volume},
  {161, "track_enable", DP_SENSOR, 0, IPC_APP_get_track_enable},
  {162, "system_reboot", DP_INT, IPC_APP_set_system_reboot, 0},
  {167, "cry_detect_onoff", DP_BOOL, IPC_APP_set_cry_detect_onoff, IPC_APP_get_cry_detect_onoff},
  {168, "alarm_zone_onoff", DP_BOOL, IPC_APP_set_alarm_zone_onoff, IPC_APP_get_alarm_zone_onoff},
  {169, "alarm_zone_draw", DP_ENUM, IPC_APP_set_alarm_zone_draw, IPC_APP_get_alarm_zone_draw},
  {170, "human_filter", DP_INT, IPC_APP_set_human_filter, IPC_APP_get_human_filter},
  {172, "flight_warn_onoff", DP_BOOL, IPC_APP_set_flight_warn_onoff, IPC_APP_get_flight_warn_onoff},
  {173, "flight_warn_time", DP_INT, IPC_APP_set_flight_warn_time, IPC_APP_get_flight_warn_time},
  {174, "patrol_switch", DP_BOOL, IPC_APP_set_patrol_switch, 0},
  {175, "patrol_mode", DP_ENUM, IPC_APP_set_patrol_mode, IPC_APP_get_patrol_mode},
  {176, "patrol_tmode", DP_ENUM, IPC_APP_set_patrol_tmode, 0},
  {177, "patrol_time", DP_INT, IPC_APP_set_patrol_time, 0},
  {178, "preset", DP_INT, IPC_APP_set_preset, 0},
  {179, "patrol_state", DP_SENSOR, 0, 0},
  {188, "basic_anti_flicker", DP_INT, IPC_APP_set_basic_anti_flicker, IPC_APP_get_basic_anti_flicker},
  {190, "link_move", DP_INT, IPC_APP_set_link_move, 0},
  {198, "object_outline_onoff", DP_BOOL, IPC_APP_set_object_outline_onoff, IPC_APP_get_object_outline_onoff},
  {199, "link_pos", DP_INT, IPC_APP_set_link_pos, 0},
  {232, "ipc_lullaby_mode", DP_ENUM, IPC_APP_set_ipc_lullaby_mode, IPC_APP_get_ipc_lullaby_mode},
  {234, "ipc_lullaby_control", DP_ENUM, IPC_APP_set_ipc_lullaby_control, IPC_APP_get_ipc_lullaby_control},
  {235, "ipc_lullaby_list", DP_ENUM, IPC_APP_set_ipc_lullaby_list, IPC_APP_get_ipc_lullaby_list},
  {236, "ipc_lullaby_command", DP_ENUM, IPC_APP_set_ipc_lullaby_command, 0},
  {238, "temp_report_c", DP_SENSOR, 0, IPC_APP_get_temp_report_c},
  {239, "temp_report_f", DP_SENSOR, 0, IPC_APP_get_temp_report_f},
  {240, "temp_unit_select", DP_ENUM, IPC_APP_set_temp_unit_select, IPC_APP_get_temp_unit_select},
  {241, "temp_max_onoff", DP_BOOL, IPC_APP_set_temp_max_onoff, IPC_APP_get_temp_max_onoff},
  {242, "temp_min_onoff", DP_BOOL, IPC_APP_set_temp_min_onoff, IPC_APP_get_temp_min_onoff},
  {243, "temp_max_cvalue", DP_INT, IPC_APP_set_temp_max_cvalue, IPC_APP_get_temp_max_cvalue},
  {244, "temp_min_cvalue", DP_INT, IPC_APP_set_temp_min_cvalue, IPC_APP_get_temp_min_cvalue},
  {245, "temp_max_fvalue", DP_INT, IPC_APP_set_temp_max_fvalue, IPC_APP_get_temp_max_fvalue},
  {246, "temp_min_fvalue", DP_INT, IPC_APP_set_temp_min_fvalue, IPC_APP_get_temp_min_fvalue},
  {247, "audible_alarm_onoff", DP_BOOL, IPC_APP_set_audible_alarm_onoff, IPC_APP_get_audible_alarm_onoff},
  {248, "door_control", DP_ENUM, IPC_APP_set_door_control, IPC_APP_get_door_control},
};

#define DP_TABLE_LEN 61

#endif
