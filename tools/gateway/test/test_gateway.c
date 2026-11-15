/*
 * Tests PC de tools/gateway/gateway.c (issue #53). Fonctions IPC_APP_* et ht_* simulées.
 * Lancer : tools/gateway/test/run.sh
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
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
static char zone[160] = "{}", moved[8] = "";
long IPC_APP_set_alarm_zone_draw(const char *v) { strcpy(zone, v); return 0; }
long IPC_APP_get_alarm_zone_draw(void) { return (long)zone; }
void IPC_APP_patrol_state(int *out) { *out = 2; }
long IPC_APP_ptz_start_move(const char *v) { strcpy(moved, v); return 0; }
long IPC_APP_format_sd_card(void) { formats++; return 0; }
long IPC_APP_get_sd_status(void) { return sdst; }
int ht_night_mode_set_irled_mode(int m) { night = m; return m == 2 ? 0 : 0; }
static char sys_all[1024];
int system(const char *c)
{
    FILE *f;
    const char *o = 0;

    strcpy(last_sys, c);
    strcat(sys_all, c);
    strcat(sys_all, "\n");
    if (strstr(c, "get_position"))
        o = "180,98,0\n";
    else if (strstr(c, "get_presets"))
        o = "1=a,10,20,0\n2=b,30,40,0\n";
    else if (strstr(c, "add_preset"))
        o = "3\n";
    if (o && (f = fopen("/tmp/gw_out", "w"))) {
        fputs(o, f);
        fclose(f);
    }
    return 0;
}
void IPC_APP_get_sd_storage(unsigned long *a, unsigned long *b, unsigned long *c) { *a = 4000000000UL; *b = 1000UL; *c = 3999999000UL; }
static int talk_open, talk_bytes;
int ht_audio_codec_talk_open(void) { talk_open++; return 0; }
int ht_audio_codec_talk_close(void) { return 0; }
int ht_audio_codec_play_talk_data(unsigned char *b, int n) { (void)b; talk_bytes += n; return 0; }

static char sdh[8];
static int pub(const char *t, const char *p)
{
    if (!strcmp(t, "camera/sd_health"))
        strcpy(sdh, p);
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
    CHECK(gw_topic("camera/privacy/set", "ON") == 0 && strstr(sys_all, "privacy.sh on") && PUB("camera/privacy", "ON"));
    CHECK(gw_topic("camera/privacy/set", "off") == 0 && strstr(sys_all, "privacy.sh off") && PUB("camera/privacy", "OFF"));
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
    /* sleep_mode est routé vers privacy : le natif n'est jamais appelé */
    CHECK(gw_topic("camera/sleep_mode/set", "ON") == 0 && sleepm == 0 && strstr(sys_all, "privacy.sh on") && PUB("camera/sleep_mode", "ON"));
    CHECK(gw_topic("camera/sleep_mode/set", "3") < 0 && sleepm == 0);
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
    npub = 0;
    gw_publish_sensors();
    CHECK(npub == 5 && PUB("camera/patrol_state", "2")); /* position, santé SD, sd_storage, sd_status, patrol_state */

    /* json : texte libre transmis tel quel ; sensor « out » ; action à valeurs limitées */
    CHECK(gw_topic("camera/alarm_zone_draw/set", "{\"num\":1}") == 0 && !strcmp(zone, "{\"num\":1}") && PUB("camera/alarm_zone_draw", "{\"num\":1}"));
    CHECK(gw_topic("camera/ptz_start_move/set", "4") == 0 && !strcmp(moved, "4"));
    CHECK(gw_topic("camera/ptz_start_move/set", "3") < 0 && !strcmp(moved, "4"));

    /* sd_storage : trois sorties non signées publiées en JSON */
    CHECK(gw_topic("camera/sd_storage/set", "1") < 0);
    npub = 0;
    gw_publish_all();
    CHECK(npub >= 12);

    /* alias PTZ : arguments validés avant tout passage au shell */
    sys_all[0] = 0;
    CHECK(gw_topic("camera/ptz_absolute/set", "0.5,-0.2") == 0 && strstr(sys_all, "ptz_move -j 0.5,-0.2,0\n") && PUB("camera/ptz_position", "180,98,0"));
    CHECK(gw_topic("camera/ptz_absolute/set", "0.5,-0.2,0") == 0 && strstr(sys_all, "ptz_move -j 0.5,-0.2,0\n"));
    sys_all[0] = 0;
    CHECK(gw_topic("camera/ptz_absolute/set", "1;reboot") < 0 && !sys_all[0]);
    CHECK(gw_topic("camera/ptz_preset_goto/set", "2") == 0 && strstr(sys_all, "ptz_move -p 2\n"));
    CHECK(gw_topic("camera/ptz_preset_goto/set", "2 3") < 0 && !strstr(sys_all, "2 3"));
    CHECK(gw_topic("camera/ptz_preset_save/set", "salon") == 0 && strstr(sys_all, "add_preset -m salon") && !strcmp(last_topic, "camera/ptz_presets") && !strcmp(last_payload, "1=a,10,20,0;2=b,30,40,0"));
    CHECK(gw_topic("camera/ptz_preset_save/set", "a b") < 0);
    CHECK(gw_topic("camera/ptz_preset_delete/set", "3") == 0 && strstr(sys_all, "del_preset -n 3"));
    CHECK(gw_topic("camera/ptz_move/set", "left") == 0 && strstr(sys_all, "ptz_move -m left") && strstr(sys_all, "ptz_move -m stop"));
    CHECK(gw_topic("camera/ptz_move/set", "left;reboot") < 0);
    CHECK(gw_topic("camera/ptz_home/set", "ON") == 0 && strstr(sys_all, "ptz_move -h"));
    CHECK(gw_topic("camera/ptz_calibrate/set", "ON") == 0 && strstr(sys_all, "ptz_calibrate.sh"));
    CHECK(gw_topic("camera/ptz_calibrate/set", "x") < 0);

    /* haut-parleur : lecture d'un .ulaw au rythme réel, arrêt à la demande */
    {
        FILE *f = fopen("/tmp/sirene.ulaw", "wb");
        unsigned char z[800] = {0};

        fwrite(z, 1, sizeof z, f);
        fclose(f);
    }
    CHECK(gw_topic("camera/speaker_play/set", "absent") < 0);
    CHECK(gw_topic("camera/speaker_play/set", "../x") < 0);
    CHECK(gw_topic("camera/speaker_siren/set", "ON") == 0 && PUB("camera/speaker_siren", "ON"));
    usleep(150000);
    CHECK(talk_open >= 1 && talk_bytes > 0);
    CHECK(gw_topic("camera/speaker_siren/set", "OFF") == 0 && PUB("camera/speaker_siren", "OFF"));
    usleep(150000);

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
    CHECK(npub == 13); /* position, préréglages, santé SD, sirène + 9 DP lisibles */

    /* santé SD : lecture seule (ou absente) = ON, lecture-écriture = OFF ; les montages « bind » ne comptent pas */
    {
        FILE *m = fopen("/tmp/gw_mounts_test", "w");
        fputs("/dev/mmcblk0p1 /tmp/sd vfat rw,sync,relatime 0 0\n/dev/mmcblk0p1 /mnt vfat ro,relatime 0 0\n", m);
        fclose(m);
        gw_publish_sensors();
        CHECK(!strcmp(sdh, "OFF"));
        m = fopen("/tmp/gw_mounts_test", "w");
        fputs("/dev/mmcblk0p1 /tmp/sd vfat ro,relatime,errors=remount-ro 0 0\n", m);
        fclose(m);
        gw_publish_sensors();
        CHECK(!strcmp(sdh, "ON"));
        m = fopen("/tmp/gw_mounts_test", "w");
        fputs("tmpfs /tmp tmpfs rw 0 0\n", m);
        fclose(m);
        gw_publish_sensors();
        CHECK(!strcmp(sdh, "ON"));
        unlink("/tmp/gw_mounts_test");
    }

    /* sans MQTT : aucun plantage */
    gw_init("camera", 0, 0);
    CHECK(gw_topic("camera/night/set", "day") == 0 && night == 1);

    printf(fails ? "%d échec(s)\n" : "OK\n", fails);
    return fails != 0;
}
