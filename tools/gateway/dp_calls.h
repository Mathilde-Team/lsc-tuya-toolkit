/* Généré par tools/gateway/gen_gateway.py : ne pas éditer.
   Couche d'appel du runtime (md_inject.c) vers les IPC_APP_* du firmware.
   Convention provisoire des setters : int IPC_APP_set_X(const char *) — la valeur d'un DP
   Tuya est une chaîne (docs/FIRMWARE_COMMANDS.md : « chaîne numérique (atoi) » ;
   IPC_APP_set_night_mode, confirmé au désassemblage, prend « 0 »/« 1 »/« 2 »).
   Chaque lot de décompilation (#54 à #58) confirme la signature réelle dans DP_VALUES.md
   et ajuste CONFIRMED_GETTERS dans gen_gateway.py. */
#ifndef DP_CALLS_H
#define DP_CALLS_H

#include "dp_table.h"

extern int IPC_APP_set_light_onoff(const char *);
extern int IPC_APP_set_flip_onoff(const char *);
extern int IPC_APP_set_watermark_onoff(const char *);
extern int IPC_APP_set_sleep_mode(const char *);
extern int IPC_APP_set_alarm_sensitivity(const char *);
extern int IPC_APP_set_night_mode(const char *);
extern int IPC_APP_format_sd_card(const char *);
extern int IPC_APP_unmount_sd_card(const char *);
extern int IPC_APP_ptz_stop_move(const char *);
extern int IPC_APP_ptz_start_move(const char *);
extern int IPC_APP_set_alarm_siren_onoff(const char *);
extern int IPC_APP_set_night_vision_mode(const char *);
extern int IPC_APP_ptz_check(const char *);
extern int IPC_APP_set_alarm_function_onoff(const char *);
extern int IPC_APP_set_blub_onoff(const char *);
extern int IPC_APP_set_sound_detect_onoff(const char *);
extern int IPC_APP_set_sound_sensitivity(const char *);
extern int IPC_APP_set_sd_record_onoff(const char *);
extern int IPC_APP_set_sd_record_mode(const char *);
extern int IPC_APP_set_pir_detect_switch(const char *);
extern int IPC_APP_set_floodlight_lightness(const char *);
extern int IPC_APP_set_siren_onoff(const char *);
extern int IPC_APP_set_ipc_lullaby_volume(const char *);
extern int IPC_APP_set_system_reboot(const char *);
extern int IPC_APP_set_cry_detect_onoff(const char *);
extern int IPC_APP_set_alarm_zone_onoff(const char *);
extern int IPC_APP_set_alarm_zone_draw(const char *);
extern int IPC_APP_set_human_filter(const char *);
extern int IPC_APP_set_flight_warn_onoff(const char *);
extern int IPC_APP_set_flight_warn_time(const char *);
extern int IPC_APP_set_patrol_switch(const char *);
extern int IPC_APP_set_patrol_mode(const char *);
extern int IPC_APP_set_patrol_tmode(const char *);
extern int IPC_APP_set_patrol_time(const char *);
extern int IPC_APP_set_preset(const char *);
extern int IPC_APP_set_basic_anti_flicker(const char *);
extern int IPC_APP_set_link_move(const char *);
extern int IPC_APP_set_object_outline_onoff(const char *);
extern int IPC_APP_set_link_pos(const char *);
extern int IPC_APP_set_ipc_lullaby_mode(const char *);
extern int IPC_APP_set_ipc_lullaby_control(const char *);
extern int IPC_APP_set_ipc_lullaby_list(const char *);
extern int IPC_APP_set_ipc_lullaby_command(const char *);
extern int IPC_APP_set_temp_unit_select(const char *);
extern int IPC_APP_set_temp_max_onoff(const char *);
extern int IPC_APP_set_temp_min_onoff(const char *);
extern int IPC_APP_set_temp_max_cvalue(const char *);
extern int IPC_APP_set_temp_min_cvalue(const char *);
extern int IPC_APP_set_temp_max_fvalue(const char *);
extern int IPC_APP_set_temp_min_fvalue(const char *);
extern int IPC_APP_set_audible_alarm_onoff(const char *);
extern int IPC_APP_set_door_control(const char *);

/* Commande : renvoie 0 si le firmware a accepté, -1 si inconnu/refusé. */
static int dp_call_set(const struct dp_entry *e, const char *v)
{
  switch (e->dp) {
  case 101: return IPC_APP_set_light_onoff(v);
  case 103: return IPC_APP_set_flip_onoff(v);
  case 104: return IPC_APP_set_watermark_onoff(v);
  case 105: return IPC_APP_set_sleep_mode(v);
  case 106: return IPC_APP_set_alarm_sensitivity(v);
  case 108: return IPC_APP_set_night_mode(v);
  case 111: return IPC_APP_format_sd_card(v);
  case 112: return IPC_APP_unmount_sd_card(v);
  case 116: return IPC_APP_ptz_stop_move(v);
  case 119: return IPC_APP_ptz_start_move(v);
  case 120: return IPC_APP_set_alarm_siren_onoff(v);
  case 124: return IPC_APP_set_night_vision_mode(v);
  case 132: return IPC_APP_ptz_check(v);
  case 134: return IPC_APP_set_alarm_function_onoff(v);
  case 138: return IPC_APP_set_blub_onoff(v);
  case 139: return IPC_APP_set_sound_detect_onoff(v);
  case 140: return IPC_APP_set_sound_sensitivity(v);
  case 150: return IPC_APP_set_sd_record_onoff(v);
  case 151: return IPC_APP_set_sd_record_mode(v);
  case 152: return IPC_APP_set_pir_detect_switch(v);
  case 158: return IPC_APP_set_floodlight_lightness(v);
  case 159: return IPC_APP_set_siren_onoff(v);
  case 160: return IPC_APP_set_ipc_lullaby_volume(v);
  case 162: return IPC_APP_set_system_reboot(v);
  case 167: return IPC_APP_set_cry_detect_onoff(v);
  case 168: return IPC_APP_set_alarm_zone_onoff(v);
  case 169: return IPC_APP_set_alarm_zone_draw(v);
  case 170: return IPC_APP_set_human_filter(v);
  case 172: return IPC_APP_set_flight_warn_onoff(v);
  case 173: return IPC_APP_set_flight_warn_time(v);
  case 174: return IPC_APP_set_patrol_switch(v);
  case 175: return IPC_APP_set_patrol_mode(v);
  case 176: return IPC_APP_set_patrol_tmode(v);
  case 177: return IPC_APP_set_patrol_time(v);
  case 178: return IPC_APP_set_preset(v);
  case 188: return IPC_APP_set_basic_anti_flicker(v);
  case 190: return IPC_APP_set_link_move(v);
  case 198: return IPC_APP_set_object_outline_onoff(v);
  case 199: return IPC_APP_set_link_pos(v);
  case 232: return IPC_APP_set_ipc_lullaby_mode(v);
  case 234: return IPC_APP_set_ipc_lullaby_control(v);
  case 235: return IPC_APP_set_ipc_lullaby_list(v);
  case 236: return IPC_APP_set_ipc_lullaby_command(v);
  case 240: return IPC_APP_set_temp_unit_select(v);
  case 241: return IPC_APP_set_temp_max_onoff(v);
  case 242: return IPC_APP_set_temp_min_onoff(v);
  case 243: return IPC_APP_set_temp_max_cvalue(v);
  case 244: return IPC_APP_set_temp_min_cvalue(v);
  case 245: return IPC_APP_set_temp_max_fvalue(v);
  case 246: return IPC_APP_set_temp_min_fvalue(v);
  case 247: return IPC_APP_set_audible_alarm_onoff(v);
  case 248: return IPC_APP_set_door_control(v);
  default: return -1;
  }
}

/* État : appelé seulement pour les getters confirmés par un lot (sinon -1, jamais appelé
   sur la caméra : signature inconnue). Renvoie la valeur, -1 si non confirmé. */
static int dp_call_get(const struct dp_entry *e)
{
  switch (e->dp) {
  case 101: break; /* IPC_APP_get_light_onoff : signature à confirmer (lot #58) */
  case 103: break; /* IPC_APP_get_flip_onoff : signature à confirmer (lot #54) */
  case 104: break; /* IPC_APP_get_watermark_onoff : signature à confirmer (lot #54) */
  case 105: break; /* IPC_APP_get_sleep_mode : signature à confirmer (lot #58) */
  case 106: break; /* IPC_APP_get_alarm_sensitivity : signature à confirmer (lot #55) */
  case 108: break; /* IPC_APP_get_night_mode : signature à confirmer (lot #58) */
  case 109: break; /* IPC_APP_get_sd_storage : signature à confirmer (lot #56) */
  case 110: break; /* IPC_APP_get_sd_status : signature à confirmer (lot #56) */
  case 117: break; /* IPC_APP_get_sd_format_status : signature à confirmer (lot #56) */
  case 120: break; /* IPC_APP_get_alarm_siren_onoff : signature à confirmer (lot #55) */
  case 124: break; /* IPC_APP_get_night_vision_mode : signature à confirmer (lot #58) */
  case 134: break; /* IPC_APP_get_alarm_function_onoff : signature à confirmer (lot #55) */
  case 138: break; /* IPC_APP_get_blub_onoff : signature à confirmer (lot #58) */
  case 139: break; /* IPC_APP_get_sound_detect_onoff : signature à confirmer (lot #55) */
  case 140: break; /* IPC_APP_get_sound_sensitivity : signature à confirmer (lot #55) */
  case 145: break; /* IPC_APP_get_battery_percent : signature à confirmer (lot #58) */
  case 146: break; /* IPC_APP_get_power_mode : signature à confirmer (lot #58) */
  case 150: break; /* IPC_APP_get_sd_record_onoff : signature à confirmer (lot #56) */
  case 151: break; /* IPC_APP_get_sd_record_mode : signature à confirmer (lot #56) */
  case 152: break; /* IPC_APP_get_pir_detect_switch : signature à confirmer (lot #58) */
  case 158: break; /* IPC_APP_get_floodlight_lightness : signature à confirmer (lot #58) */
  case 159: break; /* IPC_APP_get_siren_onoff : signature à confirmer (lot #55) */
  case 160: break; /* IPC_APP_get_ipc_lullaby_volume : signature à confirmer (lot #58) */
  case 161: break; /* IPC_APP_get_track_enable : signature à confirmer (lot #57) */
  case 167: break; /* IPC_APP_get_cry_detect_onoff : signature à confirmer (lot #55) */
  case 168: break; /* IPC_APP_get_alarm_zone_onoff : signature à confirmer (lot #55) */
  case 169: break; /* IPC_APP_get_alarm_zone_draw : signature à confirmer (lot #55) */
  case 170: break; /* IPC_APP_get_human_filter : signature à confirmer (lot #55) */
  case 172: break; /* IPC_APP_get_flight_warn_onoff : signature à confirmer (lot #58) */
  case 173: break; /* IPC_APP_get_flight_warn_time : signature à confirmer (lot #58) */
  case 175: break; /* IPC_APP_get_patrol_mode : signature à confirmer (lot #57) */
  case 188: break; /* IPC_APP_get_basic_anti_flicker : signature à confirmer (lot #54) */
  case 198: break; /* IPC_APP_get_object_outline_onoff : signature à confirmer (lot #54) */
  case 232: break; /* IPC_APP_get_ipc_lullaby_mode : signature à confirmer (lot #58) */
  case 234: break; /* IPC_APP_get_ipc_lullaby_control : signature à confirmer (lot #58) */
  case 235: break; /* IPC_APP_get_ipc_lullaby_list : signature à confirmer (lot #58) */
  case 238: break; /* IPC_APP_get_temp_report_c : signature à confirmer (lot #58) */
  case 239: break; /* IPC_APP_get_temp_report_f : signature à confirmer (lot #58) */
  case 240: break; /* IPC_APP_get_temp_unit_select : signature à confirmer (lot #58) */
  case 241: break; /* IPC_APP_get_temp_max_onoff : signature à confirmer (lot #58) */
  case 242: break; /* IPC_APP_get_temp_min_onoff : signature à confirmer (lot #58) */
  case 243: break; /* IPC_APP_get_temp_max_cvalue : signature à confirmer (lot #58) */
  case 244: break; /* IPC_APP_get_temp_min_cvalue : signature à confirmer (lot #58) */
  case 245: break; /* IPC_APP_get_temp_max_fvalue : signature à confirmer (lot #58) */
  case 246: break; /* IPC_APP_get_temp_min_fvalue : signature à confirmer (lot #58) */
  case 247: break; /* IPC_APP_get_audible_alarm_onoff : signature à confirmer (lot #55) */
  case 248: break; /* IPC_APP_get_door_control : signature à confirmer (lot #58) */
  default: break;
  }
  return -1;
}

#endif
