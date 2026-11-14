/* Généré par tools/gateway/gen_ha_discovery.py depuis dp_table.h : ne pas éditer. */
#ifndef HA_DISCOVERY_H
#define HA_DISCOVERY_H

/* topic : <prefix>/<component>/<node>/<object>/config ; %B base, %N nœud, %M mouvement */
struct ha_entry {
  const char *component;
  const char *object;
  const char *payload;
};

static const struct ha_entry ha_table[] = {
  {"binary_sensor", "motion",
   "{\"name\":\"Mouvement\",\"unique_id\":\"%N_motion\",\"state_topic\":\"%M\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"device_class\":\"motion\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"select", "night",
   "{\"name\":\"Jour/nuit\",\"unique_id\":\"%N_night\",\"state_topic\":\"%B/night\",\"command_topic\":\"%B/night/set\",\"options\":[\"auto\",\"day\",\"night\"],\"icon\":\"mdi:weather-night\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"switch", "privacy",
   "{\"name\":\"Mode privé\",\"unique_id\":\"%N_privacy\",\"state_topic\":\"%B/privacy\",\"command_topic\":\"%B/privacy/set\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\",\"icon\":\"mdi:eye-off\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"switch", "flip_onoff",
   "{\"name\":\"Flip onoff\",\"unique_id\":\"%N_flip_onoff\",\"command_topic\":\"%B/flip_onoff/set\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\",\"state_topic\":\"%B/flip_onoff\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"switch", "watermark_onoff",
   "{\"name\":\"Watermark onoff\",\"unique_id\":\"%N_watermark_onoff\",\"command_topic\":\"%B/watermark_onoff/set\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"state_on\":\"ON\",\"state_off\":\"OFF\",\"state_topic\":\"%B/watermark_onoff\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"text", "system_reboot",
   "{\"name\":\"System reboot\",\"unique_id\":\"%N_system_reboot\",\"command_topic\":\"%B/system_reboot/set\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
  {"select", "basic_anti_flicker",
   "{\"name\":\"Basic anti flicker\",\"unique_id\":\"%N_basic_anti_flicker\",\"command_topic\":\"%B/basic_anti_flicker/set\",\"options\":[\"0\",\"1\",\"2\"],\"state_topic\":\"%B/basic_anti_flicker\",\"device\":{\"identifiers\":[\"%N\"],\"name\":\"%N\",\"manufacturer\":\"LSC Smart Connect\",\"model\":\"Anyka AK3918 (anyka_ipc 3.10.78)\",\"sw_version\":\"lsc-tuya-toolkit\"}}"},
};

#define HA_TABLE_LEN 7

#endif
