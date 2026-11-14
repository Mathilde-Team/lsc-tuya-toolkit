/*
 * Passerelle locale MQTT/ONVIF -> IPC_APP_* (issue #53), pilotée par dp_table.h.
 * Transport (MQTT, /tmp/md_cmd) fourni par l'appelant (tools/motion/md_inject.c).
 */
#ifndef GATEWAY_H
#define GATEWAY_H

/* Publie un état retained ; renvoie 0 si succès. NULL = pas de MQTT (journal seul). */
typedef int (*gw_publish_fn)(const char *topic, const char *payload);
typedef void (*gw_log_fn)(const char *msg);

void gw_init(const char *base, gw_publish_fn pub, gw_log_fn log);

/* Ordre <fonction> <valeur> (MQTT <base>/<fonction>/set ou ligne de /tmp/md_cmd).
 * Renvoie 0 si traité, -1 si fonction inconnue, non exposée ou valeur refusée. */
int gw_command(const char *name, const char *val);

/* Topic MQTT complet : extrait <fonction> de <base>/<fonction>/set puis gw_command. */
int gw_topic(const char *topic, const char *val);

/* Ligne de /tmp/md_cmd ("night day", "flip_onoff ON") ; espaces finaux ignorés. */
int gw_line(char *line);

/* Filtre d'abonnement unique : <base>/+/set. */
const char *gw_subscription(void);

/* Publie l'état retained de tous les DP exposés ayant une lecture. */
void gw_publish_all(void);

/* Publie les DP exposés de type sensor (appelé périodiquement). */
void gw_publish_sensors(void);

#endif
