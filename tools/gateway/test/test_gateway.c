/*
 * Tests PC de tools/gateway/gateway.c (issue #53). Fonctions IPC_APP_* et ht_* simulées.
 * Lancer : tools/gateway/test/run.sh
 */
#include <stdio.h>
#include <string.h>
#include "gateway.h"

static char last_topic[128], last_payload[128], last_log[256], last_sys[128];
static int npub, fails;
static long flip = 0, sleepm = 0, sens = 1, sdst = 3, formats, night = -1;
static char wm[8] = "0", recmode[16] = "event";

/* ---- simulations ---- */
long IPC_APP_set_flip_onoff(long v) { flip = v; return 0; }
long IPC_APP_get_flip_onoff(void) { return flip; }
long IPC_APP_set_watermark_onoff(const char *v) { strcpy(wm, v); return 0; }
long IPC_APP_get_watermark_onoff(void) { return (long)wm; }
long IPC_APP_set_sleep_mode(long v) { sleepm = v; return 0; }
long IPC_APP_get_sleep_mode(void) { return sleepm; }
long IPC_APP_set_sd_record_mode(const char *v) { strcpy(recmode, v); return 0; }
long IPC_APP_get_sd_record_mode(void) { return (long)recmode; }
long IPC_APP_set_alarm_sensitivity(long v) { sens = v; return 0; }
long IPC_APP_get_alarm_sensitivity(void) { return sens; }
long IPC_APP_format_sd_card(void) { formats++; return 0; }
long IPC_APP_get_sd_status(void) { return sdst; }
int ht_night_mode_set_irled_mode(int m) { night = m; return m == 2 ? 0 : 0; }
int system(const char *c) { strcpy(last_sys, c); return 0; }

static int pub(const char *t, const char *p)
{
    strcpy(last_topic, t);
    strcpy(last_payload, p);
    npub++;
    return 0;
}

static void lg(const char *m) { strcpy(last_log, m); }

#define CHECK(c) do { if (!(c)) { printf("ÉCHEC %s:%d : %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)
#define PUB(t, p) (!strcmp(last_topic, t) && !strcmp(last_payload, p))

int main(void)
{
    char line[64];

    gw_init("camera", pub, lg);
    CHECK(!strcmp(gw_subscription(), "camera/+/set"));

    /* alias historiques : même effet, mêmes états */
    CHECK(gw_topic("camera/night/set", "day") == 0 && night == 1 && PUB("camera/night", "day"));
    CHECK(!strcmp(last_log, "night mode set"));
    CHECK(gw_topic("camera/night/set", "auto") == 0 && night == 0 && PUB("camera/night", "auto"));
    CHECK(gw_topic("camera/night/set", "Night") < 0 && night == 0);
    CHECK(gw_topic("camera/privacy/set", "ON") == 0 && strstr(last_sys, "privacy.sh on") && PUB("camera/privacy", "ON"));
    CHECK(gw_topic("camera/privacy/set", "off") == 0 && strstr(last_sys, "privacy.sh off") && PUB("camera/privacy", "OFF"));
    strcpy(line, "night night\n");
    CHECK(gw_line(line) == 0 && night == 2 && PUB("camera/night", "night"));
    strcpy(line, "privacy on \r\n");
    CHECK(gw_line(line) == 0 && PUB("camera/privacy", "ON"));
    strcpy(line, "night");
    CHECK(gw_line(line) < 0);

    /* bool int / str */
    CHECK(gw_topic("camera/flip_onoff/set", "ON") == 0 && flip == 1 && PUB("camera/flip_onoff", "ON"));
    CHECK(gw_topic("camera/flip_onoff/set", "off") == 0 && flip == 0 && PUB("camera/flip_onoff", "OFF"));
    CHECK(gw_topic("camera/flip_onoff/set", "2") < 0 && flip == 0);
    CHECK(gw_topic("camera/watermark_onoff/set", "ON") == 0 && !strcmp(wm, "1") && PUB("camera/watermark_onoff", "ON"));

    /* enum */
    CHECK(gw_topic("camera/sleep_mode/set", "1") == 0 && sleepm == 1 && PUB("camera/sleep_mode", "1"));
    CHECK(gw_topic("camera/sleep_mode/set", "3") < 0 && sleepm == 1);
    CHECK(gw_topic("camera/sd_record_mode/set", "always") == 0 && !strcmp(recmode, "always") && PUB("camera/sd_record_mode", "always"));
    CHECK(gw_topic("camera/sd_record_mode/set", "alway") < 0);

    /* int et bornes */
    CHECK(gw_topic("camera/alarm_sensitivity/set", "2") == 0 && sens == 2 && PUB("camera/alarm_sensitivity", "2"));
    CHECK(gw_topic("camera/alarm_sensitivity/set", "3") < 0 && sens == 2);
    CHECK(gw_topic("camera/alarm_sensitivity/set", "-1") < 0);
    CHECK(gw_topic("camera/alarm_sensitivity/set", "1x") < 0);

    /* action : pas d'état publié */
    npub = 0;
    CHECK(gw_topic("camera/format_sd_card/set", "go") == 0 && formats == 1 && npub == 0);

    /* sensor : pas d'écriture, publié par gw_publish_sensors */
    CHECK(gw_topic("camera/sd_status/set", "1") < 0);
    gw_publish_sensors();
    CHECK(PUB("camera/sd_status", "3"));

    /* non exposé / inconnu / mauvais préfixe */
    CHECK(gw_topic("camera/basic_anti_flicker/set", "1") < 0);
    CHECK(!strcmp(last_log, "gateway: unknown or unexposed basic_anti_flicker"));
    CHECK(gw_topic("camera/nope/set", "1") < 0);
    CHECK(gw_topic("other/flip_onoff/set", "ON") < 0);
    CHECK(gw_topic("camera/flip_onoff", "ON") < 0);
    CHECK(gw_topic("camera/x/y/set", "ON") < 0);

    /* état initial : tous les exposés lisibles */
    npub = 0;
    gw_publish_all();
    CHECK(npub == 6); /* 7 exposés dont 1 action sans lecture */

    /* sans MQTT : aucun plantage */
    gw_init("camera", 0, 0);
    CHECK(gw_topic("camera/night/set", "day") == 0 && night == 1);

    printf(fails ? "%d échec(s)\n" : "OK\n", fails);
    return fails != 0;
}
