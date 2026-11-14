/*
 * md_inject.so : détection de mouvement locale pour anyka_ipc (sans cloud Tuya).
 * Chargée par LD_PRELOAD. Le firmware initialise la détection (ak_md) mais ne l'active
 * et n'enregistre son callback que depuis le service Tuya. On le fait ici :
 * ht_montion_detect_set_active_cb() puis ht_montion_detect_open().
 * Chaque mouvement est écrit dans le journal et, si MOTION_MQTT_FILE est défini,
 * publié en MQTT (ON au début, OFF après MOTION_HOLD secondes sans nouveau mouvement).
 * Variables :
 *   MOTION_MQTT_FILE  fichier de 4 lignes : IP, port, utilisateur, mot de passe (mode 600). Absent = journal seul.
 *   MOTION_TOPIC      topic MQTT (défaut camera/motion)
 *   MOTION_HOLD       secondes avant OFF (défaut 10)
 *   MOTION_SENS       sensibilité du firmware 0..2 (défaut : celle du firmware)
 * Commandes : passerelle générique tools/gateway/gateway.c pilotée par dp_table.h (issue #53).
 *   Abonnement MQTT unique <base>/+/set ; <base>/<fonction>/set -> état retained <base>/<fonction>.
 *   Alias historiques inchangés (désassemblage : ht_night_mode_set_irled_mode 0 auto / 1 jour / 2 nuit) :
 *   MQTT <base>/night/set  = auto|day|night   -> état retained <base>/night
 *   MQTT <base>/privacy/set = ON|OFF          -> privacy.sh on|off, état retained <base>/privacy
 *   Autres fonctions : seulement les DP « Exposé = oui » de docs/firmware/DP_VALUES.md (voir docs/TOPICS.md).
 *   (<base> = MOTION_TOPIC sans son dernier élément ; ex. camera/motion -> camera)
 *   Fichier /tmp/md_cmd, une ligne "<fonction> <valeur>" ("night day", "privacy on") : même effet (utilisé par ONVIF).
 *   MOTION_HUMAN      1 = active le filtre humain du firmware (liblvhsr), 0 = le coupe (défaut : inchangé)
 *   HA_DISCOVERY      0 = pas de découverte Home Assistant (défaut 1 : publiée retained au premier abonnement, #59)
 *   HA_PREFIX         préfixe de découverte HA (défaut homeassistant)
 */
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "gateway.h"

#ifndef STARTUP_DELAY_S
#define STARTUP_DELAY_S 50 /* surchargé à 0 par les tests hôte (tools/gateway/test) */
#endif

extern int ht_montion_detect_set_active_cb(void (*cb)(long long));
extern long syscall(long n, ...);
extern int ht_montion_detect_open(void);
extern int ht_montion_detect_set_sensitivity(int level);
extern int ht_montion_detect_set_human_filter(int enable);

static volatile int ev_count;

static void logmsg(const char *m)
{
    static const long pw[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
    char ts[12];
    long tv[2] = {0, 0}; /* gettimeofday direct : time() est renommé __time64 par la glibc, absent d'uClibc */
    long t;
    int i, k, n = 0;

    syscall(78, tv, 0);
    t = tv[0];

    /* pas de division : libgcc n'est pas liée */
    for (i = 0; i < 10; i++) {
        k = 0;
        while (t >= pw[i]) {
            t -= pw[i];
            k++;
        }
        ts[n++] = '0' + k;
    }
    write(1, "[md_inject] t=", 14);
    write(1, ts, n);
    write(1, " ", 1);
    write(1, m, strlen(m));
    write(1, "\n", 1);
}

static void on_motion(long long ms)
{
    (void)ms;
    ev_count++;
}

static char host[64], port[8], user[64], pass[96];
static int have_mqtt;

static void load_cfg(const char *path)
{
    char buf[512];
    char *line[4], *p = buf;
    int fd, n, i;

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return;
    n = read(fd, buf, sizeof buf - 1);
    close(fd);
    if (n <= 0)
        return;
    buf[n] = 0;
    for (i = 0; i < 4; i++) {
        line[i] = p;
        while (*p && *p != '\n' && *p != '\r')
            p++;
        if (*p) {
            *p++ = 0;
            while (*p == '\n' || *p == '\r')
                p++;
        }
    }
    if (!*line[0] || !*line[1])
        return;
    strncpy(host, line[0], sizeof host - 1);
    strncpy(port, line[1], sizeof port - 1);
    strncpy(user, line[2], sizeof user - 1);
    strncpy(pass, line[3], sizeof pass - 1);
    have_mqtt = 1;
}

static int put_str(unsigned char *o, const char *s)
{
    int l = strlen(s);
    o[0] = l >> 8;
    o[1] = l & 255;
    memcpy(o + 2, s, l);
    return l + 2;
}

/* Longueur restante MQTT (1 à 2 octets, < 16384) ; sans division : décalages. */
static int put_len(unsigned char *o, int l)
{
    if (l < 128) {
        o[0] = l;
        return 1;
    }
    o[0] = (l & 127) | 128;
    o[1] = l >> 7;
    return 2;
}

/* Écrit en-tête + longueur variable devant le corps construit à partir de pkt + 3. */
static int frame(unsigned char *pkt, unsigned char type, int body_len)
{
    unsigned char h[3];
    int hl = 1 + put_len(h + 1, body_len), i;

    h[0] = type;
    /* le corps commence en pkt + 3 : on recolle l'en-tête juste avant */
    for (i = 0; i < hl; i++)
        pkt[3 - hl + i] = h[i];
    return 3 - hl;
}

/* MQTT 3.1.1, QoS 1 + retain (attend le PUBACK), une connexion par message. Topic < 128, charge < 900 octets. */
static int publish(const char *topic, const char *payload)
{
    unsigned char pkt[1040], ack[8]; /* 3 + 2 + 127 + 2 + 900 */
    int tl = strlen(topic), pl = strlen(payload), off;
    struct sockaddr_in sa;
    struct pollfd pf;
    int s, n = 0, body, r, ok = 0;

    if (tl > 127 || pl > 900)
        return -1;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        return -1;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(port));
    sa.sin_addr.s_addr = inet_addr(host);
    if (connect(s, (struct sockaddr *)&sa, sizeof sa) < 0) {
        close(s);
        return -1;
    }
    pkt[n++] = 0x10;
    body = n++;
    n += put_str(pkt + n, "MQTT");
    pkt[n++] = 4;
    pkt[n++] = user[0] ? 0xC2 : 0x02;
    pkt[n++] = 0;
    pkt[n++] = 30;
    n += put_str(pkt + n, "anyka-md");
    if (user[0]) {
        n += put_str(pkt + n, user);
        n += put_str(pkt + n, pass);
    }
    pkt[body] = n - body - 1;
    write(s, pkt, n);
    pf.fd = s;
    pf.events = POLLIN;
    if (poll(&pf, 1, 3000) > 0 && (r = read(s, ack, sizeof ack)) >= 4 && ack[0] == 0x20 && ack[3] == 0) {
        n = 3;
        n += put_str(pkt + n, topic);
        pkt[n++] = 0;
        pkt[n++] = 1;
        memcpy(pkt + n, payload, pl);
        n += pl;
        off = frame(pkt, 0x33, n - 3);
        write(s, pkt + off, n - off);
        if (poll(&pf, 1, 3000) > 0 && read(s, ack, sizeof ack) >= 4 && ack[0] == 0x40)
            ok = 1;
    }
    pkt[0] = 0xE0;
    pkt[1] = 0;
    write(s, pkt, 2);
    close(s);
    return ok ? 0 : -1;
}

static void announce(const char *topic, const char *state)
{
    int i;

    logmsg(state);
    if (!have_mqtt)
        return;
    for (i = 0; i < 3; i++) {
        if (publish(topic, state) == 0)
            return;
        sleep(1);
    }
    logmsg("mqtt publish failed");
}


static char base[64];
static char motion_topic[96];

static int pub_state(const char *topic, const char *payload)
{
    return have_mqtt ? publish(topic, payload) : -1;
}

static void poll_file(void)
{
    char b[160];
    int fd = open("/tmp/md_cmd", O_RDONLY), n;

    if (fd < 0)
        return;
    n = read(fd, b, sizeof b - 1);
    close(fd);
    unlink("/tmp/md_cmd");
    if (n <= 0)
        return;
    b[n] = 0;
    gw_line(b);
}

/* Un PUBLISH reçu (QoS 0 ou 1) : topic + charge utile -> passerelle. */
static void on_publish(const unsigned char *p, int len, int qos)
{
    char topic[96], val[160];
    int tl, off, pl;

    if (len < 2)
        return;
    tl = (p[0] << 8) | p[1];
    off = 2 + tl + (qos ? 2 : 0);
    pl = len - off;
    if (tl <= 0 || tl >= (int)sizeof topic || pl <= 0 || pl >= (int)sizeof val)
        return;
    memcpy(topic, p + 2, tl);
    topic[tl] = 0;
    memcpy(val, p + off, pl);
    val[pl] = 0;
    gw_topic(topic, val);
}

/* Abonné MQTT : filtre unique <base>/+/set (QoS 0), PINGREQ toutes les 30 s, reconnexion automatique,
 * capteurs republiés toutes les 5 min. Paquets découpés sur plusieurs lectures : tampon cumulatif. */
static int subscribe_loop(void)
{
    static int first = 1;
    unsigned char pkt[256], rx[1024];
    struct sockaddr_in sa;
    struct pollfd pf;
    int s, n = 0, body, r, idle = 0, have = 0, sensors = 0, off;
    const char *sub = gw_subscription();

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
        return -1;
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(port));
    sa.sin_addr.s_addr = inet_addr(host);
    if (connect(s, (struct sockaddr *)&sa, sizeof sa) < 0) {
        close(s);
        return -1;
    }
    pkt[n++] = 0x10;
    body = n++;
    n += put_str(pkt + n, "MQTT");
    pkt[n++] = 4;
    pkt[n++] = user[0] ? 0xC2 : 0x02;
    pkt[n++] = 0;
    pkt[n++] = 60;
    n += put_str(pkt + n, "anyka-cmd");
    if (user[0]) {
        n += put_str(pkt + n, user);
        n += put_str(pkt + n, pass);
    }
    pkt[body] = n - body - 1;
    write(s, pkt, n);
    pf.fd = s;
    pf.events = POLLIN;
    if (poll(&pf, 1, 3000) <= 0 || read(s, rx, sizeof rx) < 4 || rx[0] != 0x20 || rx[3] != 0) {
        close(s);
        return -1;
    }
    n = 3;
    pkt[n++] = 0;
    pkt[n++] = 1;
    n += put_str(pkt + n, sub);
    pkt[n++] = 0;
    off = frame(pkt, 0x82, n - 3);
    write(s, pkt + off, n - off);
    logmsg("command subscriber ready");
    if (first) {
        const char *ha = getenv("HA_DISCOVERY"), *pre = getenv("HA_PREFIX");

        first = 0;
        if (!ha || strcmp(ha, "0"))
            gw_ha_announce(pre && *pre ? pre : "homeassistant", base, motion_topic, pub_state, logmsg);
        gw_publish_all();
    }
    for (;;) {
        poll_file();
        if (++sensors >= 300) {
            sensors = 0;
            gw_publish_sensors();
        }
        r = poll(&pf, 1, 1000);
        if (r > 0) {
            r = read(s, rx + have, sizeof rx - have);
            if (r <= 0)
                break;
            have += r;
            idle = 0;
            for (;;) {
                int len, lb, i, mult;

                if (have < 2)
                    break;
                /* longueur restante variable (jusqu'à 4 octets) */
                len = 0;
                mult = 0;
                for (lb = 1; lb <= 4 && lb < have; lb++) {
                    len |= (rx[lb] & 127) << mult;
                    mult += 7;
                    if (!(rx[lb] & 128))
                        break;
                }
                if (lb > 4) {
                    have = 0; /* trame invalide : on jette le tampon */
                    break;
                }
                if (lb >= have || 1 + lb + len > have) {
                    if (1 + lb + len > (int)sizeof rx)
                        have = 0; /* trop grand pour le tampon : ignoré */
                    break;
                }
                if ((rx[0] & 0xF0) == 0x30)
                    on_publish(rx + 1 + lb, len, (rx[0] >> 1) & 3);
                n = 1 + lb + len;
                for (i = n; i < have; i++)
                    rx[i - n] = rx[i];
                have -= n;
            }
        } else if (++idle >= 30) {
            idle = 0;
            pkt[0] = 0xC0;
            pkt[1] = 0;
            if (write(s, pkt, 2) < 2)
                break;
        }
    }
    close(s);
    return 0;
}

static void *cmd_worker(void *arg)
{
    const char *t = getenv("MOTION_TOPIC");
    char *sl;

    (void)arg;
    strncpy(base, t ? t : "camera/motion", sizeof base - 1);
    strncpy(motion_topic, t ? t : "camera/motion", sizeof motion_topic - 1);
    sl = strrchr(base, '/');
    if (sl)
        *sl = 0;
    gw_init(base, pub_state, logmsg);
    sleep(STARTUP_DELAY_S + 5);
    for (;;) {
        if (have_mqtt) {
            if (subscribe_loop() < 0)
                sleep(10);
        } else {
            poll_file();
            sleep(1);
        }
    }
    return 0;
}

static void *worker(void *arg)
{
    const char *f = getenv("MOTION_MQTT_FILE");
    const char *t = getenv("MOTION_TOPIC");
    const char *h = getenv("MOTION_HOLD");
    int hold = (h ? atoi(h) : 10) * 10;
    int seen = 0, on = 0, quiet = 0;

    (void)arg;
    if (!t)
        t = "camera/motion";
    if (f)
        load_cfg(f);
    sleep(STARTUP_DELAY_S);
    ht_montion_detect_set_active_cb(on_motion);
    if (getenv("MOTION_SENS"))
        ht_montion_detect_set_sensitivity(atoi(getenv("MOTION_SENS")));
    if (getenv("MOTION_HUMAN")) {
        int hr = ht_montion_detect_set_human_filter(atoi(getenv("MOTION_HUMAN")));
        logmsg(hr == 0 ? "human filter set" : "human filter set failed");
    }
    logmsg(ht_montion_detect_open() == 0 ? "detection active" : "detection open failed");
    for (;;) {
        usleep(100000);
        if (ev_count != seen) {
            seen = ev_count;
            quiet = 0;
            if (!on) {
                on = 1;
                announce(t, "ON");
            }
        } else if (on && ++quiet >= hold) {
            on = 0;
            announce(t, "OFF");
        }
    }
    return 0;
}

__attribute__((constructor)) static void md_init(void)
{
    pthread_t t;
    pthread_create(&t, 0, worker, 0);
    pthread_create(&t, 0, cmd_worker, 0);
}
