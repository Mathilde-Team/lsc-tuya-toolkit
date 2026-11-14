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
};

#define HA_TABLE_LEN 3

#endif
