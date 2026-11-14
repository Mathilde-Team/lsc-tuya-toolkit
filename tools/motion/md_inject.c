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
 * Commandes (désassemblage : ht_night_mode_set_irled_mode 0 auto / 1 jour / 2 nuit) :
 *   MQTT <base>/night/set  = auto|day|night   -> état retained <base>/night
 *   MQTT <base>/privacy/set = ON|OFF          -> privacy.sh on|off, état retained <base>/privacy
 *   (<base> = MOTION_TOPIC sans son dernier élément ; ex. camera/motion -> camera)
 *   Fichier /tmp/md_cmd, une ligne "night day" ou "privacy on" : même effet (utilisé par ONVIF).
 *   MOTION_HUMAN      1 = active le filtre humain du firmware (liblvhsr), 0 = le coupe (défaut : inchangé)
 *
 * Passerelle générique (issue #53), pilotée par tools/gateway/dp_table.h :
 *   DP_ENABLE  liste d'entrées de la table à exposer (numéros de DP ou noms séparés par
 *              des virgules, ou "all"). Défaut : vide, aucune entrée exposée (night et
 *              privacy seuls, comportement inchangé).
 *   DP_POLL    secondes entre republications de l'état des entrées actives pourvues d'un
 *              getter (défaut 60 ; 0 = état publié au démarrage et après chaque commande seulement).
 *   Entrée active : commande MQTT <base>/<fonction>/set (ON/OFF, entier ou action) et
 *   ligne "/tmp/md_cmd" "<fonction> <valeur>" ; état publié retained sur <base>/<fonction>.
 *   Convention d'appel assumée int f(int) (voir dp_table.h) : n'activer une ligne qu'après
 *   confirmation par son lot de décompilation (issues #54 à #58) ou pour la tester.
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

#include "dp_table.h"

/* délai avant de toucher au firmware (le laisser s'initialiser) ; surchargeable à la compilation
 * (-DSTARTUP_DELAY_S=1 pour les tests logiciels, voir test_gateway.sh) */
#ifndef STARTUP_DELAY_S
#define STARTUP_DELAY_S 50
#endif

extern int ht_montion_detect_set_active_cb(void (*cb)(long long));
extern long syscall(long n, ...);
extern int ht_montion_detect_open(void);
extern int ht_montion_detect_set_sensitivity(int level);
extern int ht_night_mode_set_irled_mode(int mode);
extern int system(const char *cmd);
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

/* MQTT 3.1.1, QoS 1 + retain (attend le PUBACK), une connexion par message. Longueurs < 128 octets par champ. */
static int publish(const char *topic, const char *payload)
{
    unsigned char pkt[512], ack[8];
    struct sockaddr_in sa;
    struct pollfd pf;
    int s, n = 0, body, r, ok = 0;

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
        n = 0;
        pkt[n++] = 0x33;
        body = n++;
        n += put_str(pkt + n, topic);
        pkt[n++] = 0;
        pkt[n++] = 1;
        memcpy(pkt + n, payload, strlen(payload));
        n += strlen(payload);
        pkt[body] = n - body - 1;
        write(s, pkt, n);
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
static const char *night_name[] = {"auto", "day", "night"};

/* ---------- passerelle générique (dp_table.h, issue #53) ---------- */

static unsigned char dp_active[DP_TABLE_LEN];
static int dp_poll_s = 60;

/* entier décimal sans division (libgcc non liée) ; retourne la fin de chaîne */
static char *fmt_int(char *o, int v)
{
    static const long pw[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
    long t = v;
    int i, k, started = 0;

    if (v < 0) {
        *o++ = '-';
        t = -(long)v;
    }
    for (i = 0; i < 10; i++) {
        k = 0;
        while (t >= pw[i]) {
            t -= pw[i];
            k++;
        }
        if (k || started || i == 9) {
            *o++ = '0' + k;
            started = 1;
        }
    }
    return o;
}

/* DP_ENABLE : "all" ou liste (noms ou numéros de DP) séparés par des virgules */
static void dp_parse_enable(const char *spec)
{
    char buf[256], *tok, *p, m[64];
    int i, n = 0;

    if (!spec || !*spec)
        return;
    strncpy(buf, spec, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
    for (tok = buf; *tok; tok = p) {
        int matched = 0;

        p = tok;
        while (*p && *p != ',')
            p++;
        if (*p)
            *p++ = 0;
        while (*tok == ' ' || *tok == '\t')
            tok++;
        if (!*tok)
            continue;
        if (!strcmp(tok, "all")) {
            for (i = 0; i < DP_TABLE_LEN; i++)
                dp_active[i] = 1;
            matched = 1;
        } else {
            int dp = atoi(tok);

            for (i = 0; i < DP_TABLE_LEN; i++)
                if ((dp && dp_table[i].dp == dp) || !strcmp(dp_table[i].name, tok)) {
                    dp_active[i] = 1;
                    matched = 1;
                }
        }
        if (!matched) {
            int tl = strlen(tok);

            if (tl > (int)sizeof m - 20)
                tl = (int)sizeof m - 20;
            strcpy(m, "dp_enable ignored: ");
            memcpy(m + 19, tok, tl);
            m[19 + tl] = 0;
            logmsg(m);
        }
    }
    for (i = 0; i < DP_TABLE_LEN; i++)
        n += dp_active[i] != 0;
    if (n) {
        char *o;

        strcpy(m, "dp table entries active: ");
        o = fmt_int(m + strlen(m), n);
        *o = 0;
        logmsg(m);
    }
}

static int dp_find(const char *name)
{
    int i;

    for (i = 0; i < DP_TABLE_LEN; i++)
        if (dp_active[i] && !strcmp(dp_table[i].name, name))
            return i;
    return -1;
}

/* état retained : getter appelé avec 0 (convention assumée, voir dp_table.h) */
static void dp_publish_state(int i)
{
    const struct dp_entry *e = dp_table + i;
    char m[80], t[96], s[16];
    char *o;
    int v;

    if (!e->get)
        return;
    v = e->get(0);
    if (e->kind == DP_BOOL)
        strcpy(s, v ? "ON" : "OFF");
    else {
        o = fmt_int(s, v);
        *o = 0;
    }
    strcpy(m, "dp ");
    strcat(m, e->name);
    strcat(m, " = ");
    strcat(m, s);
    logmsg(m);
    if (have_mqtt) {
        strcpy(t, base);
        strcat(t, "/");
        strcat(t, e->name);
        publish(t, s);
    }
}

/* commande <base>/<fonction>/set ou /tmp/md_cmd "<fonction> <valeur>" */
static void dp_apply(int i, const char *val)
{
    const struct dp_entry *e = dp_table + i;
    char m[96], t[96], s[16];
    int v = 0, r;
    char *o;

    if (!e->set || e->kind == DP_SENSOR)
        return;
    if (e->kind == DP_BOOL) {
        if (!strcmp(val, "ON") || !strcmp(val, "on") || !strcmp(val, "1"))
            v = 1;
        else if (!strcmp(val, "OFF") || !strcmp(val, "off") || !strcmp(val, "0"))
            v = 0;
        else {
            logmsg("dp payload ignored: bool attendu (ON/OFF)");
            return;
        }
    } else
        v = atoi(val); /* enum/int/action : entier en texte, valeurs précisées par les lots */
    r = e->set(v);
    strcpy(m, "dp ");
    strcat(m, e->name);
    strcat(m, r == 0 ? " ok" : " refused");
    logmsg(m);
    if (e->get)
        dp_publish_state(i);
    else if (have_mqtt && e->kind != DP_ACTION) {
        /* pas de getter : on publie la valeur appliquée comme état retained */
        if (e->kind == DP_BOOL)
            strcpy(s, v ? "ON" : "OFF");
        else {
            o = fmt_int(s, v);
            *o = 0;
        }
        strcpy(t, base);
        strcat(t, "/");
        strcat(t, e->name);
        publish(t, s);
    }
}

static void apply_night(const char *v)
{
    char t[96];
    int m = !strcmp(v, "auto") ? 0 : !strcmp(v, "day") ? 1 : !strcmp(v, "night") ? 2 : -1;

    if (m < 0)
        return;
    logmsg(ht_night_mode_set_irled_mode(m) == 0 ? "night mode set" : "night mode refused");
    strcpy(t, base);
    strcat(t, "/night");
    if (have_mqtt)
        publish(t, night_name[m]);
}

static void apply_privacy(const char *v)
{
    char t[96];
    int on = !strcmp(v, "ON") || !strcmp(v, "on");

    if (!on && strcmp(v, "OFF") && strcmp(v, "off"))
        return;
    logmsg(on ? "privacy on" : "privacy off");
    system(on ? "/tmp/sd/custom/scripts/privacy.sh on" : "/tmp/sd/custom/scripts/privacy.sh off");
    strcpy(t, base);
    strcat(t, "/privacy");
    if (have_mqtt)
        publish(t, on ? "ON" : "OFF");
}

static void handle(const char *topic, const char *val)
{
    int l = strlen(topic), bl = strlen(base), nl;
    char name[48];
    int i;

    if (l > 10 && !strcmp(topic + l - 10, "/night/set")) {
        apply_night(val);
        return;
    }
    if (l > 12 && !strcmp(topic + l - 12, "/privacy/set")) {
        apply_privacy(val);
        return;
    }
    /* entrée de la table : <base>/<fonction>/set, active seulement */
    nl = l - bl - 5;
    if (nl <= 0 || nl >= (int)sizeof name || strncmp(topic, base, bl) || topic[bl] != '/' || strcmp(topic + l - 4, "/set"))
        return;
    memcpy(name, topic + bl + 1, nl);
    name[nl] = 0;
    i = dp_find(name);
    if (i >= 0)
        dp_apply(i, val);
}

static void poll_file(void)
{
    char b[128];
    int fd = open("/tmp/md_cmd", O_RDONLY), n;
    char *sp;

    if (fd < 0)
        return;
    n = read(fd, b, sizeof b - 1);
    close(fd);
    unlink("/tmp/md_cmd");
    if (n <= 0)
        return;
    b[n] = 0;
    while (n > 0 && (b[n - 1] == '\n' || b[n - 1] == '\r' || b[n - 1] == ' '))
        b[--n] = 0;
    for (sp = b; *sp && *sp != ' '; sp++)
        ;
    if (!*sp)
        return;
    *sp++ = 0;
    if (!strcmp(b, "night"))
        apply_night(sp);
    else if (!strcmp(b, "privacy"))
        apply_privacy(sp);
    else {
        int i = dp_find(b);

        if (i >= 0)
            dp_apply(i, sp);
    }
}

/* Abonné MQTT : QoS 0, PINGREQ toutes les 30 s, reconnexion automatique.
 * Abonnements : alias night/privacy + une commande par entrée active de dp_table. */
static int subscribe_loop(void)
{
    unsigned char pkt[256], rx[256];
    struct sockaddr_in sa;
    struct pollfd pf;
    static char topics[2 + DP_TABLE_LEN][96];
    static const char *sub[2 + DP_TABLE_LEN];
    int nsubs = 2, s, n = 0, body, r, idle = 0, secs = 0, i;

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
    strcpy(topics[0], base);
    strcat(topics[0], "/night/set");
    strcpy(topics[1], base);
    strcat(topics[1], "/privacy/set");
    for (i = 0; i < DP_TABLE_LEN && nsubs < (int)(sizeof sub / sizeof sub[0]); i++) {
        if (!dp_active[i] || dp_table[i].kind == DP_SENSOR)
            continue;
        strcpy(topics[nsubs], base);
        strcat(topics[nsubs], "/");
        strcat(topics[nsubs], dp_table[i].name);
        strcat(topics[nsubs], "/set");
        nsubs++;
    }
    for (i = 0; i < nsubs; i++) {
        sub[i] = topics[i];
        n = 0;
        pkt[n++] = 0x82;
        body = n++;
        pkt[n++] = 0;
        pkt[n++] = i + 1;
        n += put_str(pkt + n, sub[i]);
        pkt[n++] = 0;
        pkt[body] = n - body - 1;
        write(s, pkt, n);
    }
    logmsg("command subscriber ready");
    /* état retained des entrées actives au démarrage */
    for (i = 0; i < DP_TABLE_LEN; i++)
        if (dp_active[i] && dp_table[i].get)
            dp_publish_state(i);
    for (;;) {
        poll_file();
        r = poll(&pf, 1, 1000);
        if (r > 0) {
            r = read(s, rx, sizeof rx - 1);
            if (r <= 0)
                break;
            idle = 0;
            for (n = 0; n + 2 <= r;) {
                int len = rx[n + 1], tl;

                if (n + 2 + len > r)
                    break;
                if ((rx[n] & 0xF0) == 0x30 && len > 2) {
                    char topic[96], val[32];
                    int pl;

                    tl = (rx[n + 2] << 8) | rx[n + 3];
                    pl = len - 2 - tl;
                    if (tl < (int)sizeof topic && pl > 0 && pl < (int)sizeof val) {
                        memcpy(topic, rx + n + 4, tl);
                        topic[tl] = 0;
                        memcpy(val, rx + n + 4 + tl, pl);
                        val[pl] = 0;
                        handle(topic, val);
                    }
                }
                n += 2 + len;
            }
        } else {
            idle++;
            if (idle >= 30) {
                idle = 0;
                pkt[0] = 0xC0;
                pkt[1] = 0;
                if (write(s, pkt, 2) < 2)
                    break;
            }
            /* republication périodique de l'état (approximative : 1 s par silence) */
            if (dp_poll_s && ++secs >= dp_poll_s) {
                secs = 0;
                for (i = 0; i < DP_TABLE_LEN; i++)
                    if (dp_active[i] && dp_table[i].get)
                        dp_publish_state(i);
            }
        }
    }
    close(s);
    return 0;
}

static void *cmd_worker(void *arg)
{
    const char *t = getenv("MOTION_TOPIC");
    const char *en = getenv("DP_ENABLE");
    const char *po = getenv("DP_POLL");
    char *sl;

    (void)arg;
    strncpy(base, t ? t : "camera/motion", sizeof base - 1);
    base[sizeof base - 1] = 0;
    sl = strrchr(base, '/');
    if (sl)
        *sl = 0;
    dp_parse_enable(en);
    if (po) {
        dp_poll_s = atoi(po);
        if (dp_poll_s < 0)
            dp_poll_s = 0;
    }
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
