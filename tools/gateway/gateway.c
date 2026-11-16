/*
 * Passerelle générique (issue #53) : une entrée de dp_table.h = un topic <base>/<name>.
 *   <base>/<name>/set  -> conversion selon le type -> IPC_APP_set_* (ou action)
 *   <base>/<name>      <- IPC_APP_get_* publié retained après chaque ordre (et au démarrage)
 * Seuls les DP confirmés par docs/firmware/DP_VALUES.md (« Exposé = oui ») sont appelés.
 *
 * Alias historiques, comportement inchangé par rapport à md_inject.c avant #53 :
 *   night   auto|day|night -> ht_night_mode_set_irled_mode(0|1|2), état <base>/night = auto|day|night
 *   privacy ON|on|OFF|off  -> privacy.sh on|off,               état <base>/privacy = ON|OFF
 *
 * Contraintes uClibc (tools/motion/build.sh, -nostdlib) : pas de division ni de modulo (libgcc absente),
 * uniquement les fonctions de libc déjà utilisées par md_inject.so.
 */
#include <string.h>
#include "gateway.h"
#ifdef GW_TABLE
#include GW_TABLE /* tests : table générée depuis une fixture */
#else
#include "dp_table.h"
#endif

extern int ht_night_mode_set_irled_mode(int mode);
extern int system(const char *cmd);

#define GW_MAX 160 /* charge utile maximale (md_inject.c : 160 octets) */

static char gw_base[64];
static char gw_sub[80];
static gw_publish_fn gw_pub;
static gw_log_fn gw_log;

static void say(const char *a, const char *b, const char *c)
{
    char m[160];
    int n = 0;
    const char *p[3];
    int i;

    p[0] = a;
    p[1] = b;
    p[2] = c;
    for (i = 0; i < 3; i++) {
        const char *s = p[i];

        if (!s)
            continue;
        if (n && n < (int)sizeof m - 1)
            m[n++] = ' ';
        while (*s && n < (int)sizeof m - 1)
            m[n++] = *s++;
    }
    m[n] = 0;
    if (gw_log)
        gw_log(m);
}

static void state(const char *name, const char *payload)
{
    char t[128];
    int bl = strlen(gw_base), nl = strlen(name);

    if (!gw_pub || bl + 1 + nl >= (int)sizeof t)
        return;
    memcpy(t, gw_base, bl);
    t[bl] = '/';
    memcpy(t + bl + 1, name, nl + 1);
    gw_pub(t, payload);
}

/* memcmp n'est pas importé par md_inject.so : comparaison locale. */
static int same(const char *a, const char *b, int n)
{
    while (n-- > 0)
        if (*a++ != *b++)
            return 0;
    return 1;
}

/* ---- conversions sans division ---- */

static int parse_int(const char *s, long *out)
{
    long v = 0;
    int neg = 0, digits = 0;

    if (*s == '-' || *s == '+')
        neg = *s++ == '-';
    while (*s >= '0' && *s <= '9') {
        if (v > 100000000L)
            return -1;
        v = v * 10 + (*s++ - '0');
        digits++;
    }
    if (!digits || *s)
        return -1;
    *out = neg ? -v : v;
    return 0;
}

static void fmt_uint(unsigned long u, char *o)
{
    static const unsigned long pw[] = {1000000000UL, 100000000UL, 10000000UL, 1000000UL, 100000UL, 10000UL, 1000UL, 100UL, 10UL, 1UL};
    int i, k, n = 0, started = 0;

    /* 32 bits : jusqu'à 4 294 967 295, soit un chiffre de milliard de plus que fmt_int */
    for (i = 0; i < 10; i++) {
        k = 0;
        while (u >= pw[i]) {
            u -= pw[i];
            k++;
        }
        if (k || started || i == 9) {
            o[n++] = '0' + k;
            started = 1;
        }
    }
    o[n] = 0;
}

static void fmt_int(long v, char *o)
{
    static const long pw[] = {1000000000L, 100000000L, 10000000L, 1000000L, 100000L, 10000L, 1000L, 100L, 10L, 1L};
    unsigned long u;
    int i, k, n = 0, started = 0;

    if (v < 0) {
        o[n++] = '-';
        u = (unsigned long)(-(v + 1)) + 1;
    } else {
        u = (unsigned long)v;
    }
    for (i = 0; i < 10; i++) {
        k = 0;
        while (u >= (unsigned long)pw[i]) {
            u -= pw[i];
            k++;
        }
        if (k || started || i == 9) {
            o[n++] = '0' + k;
            started = 1;
        }
    }
    o[n] = 0;
}

/* "min..max" */
static int in_range(const char *r, long v)
{
    char a[16];
    const char *d;
    long lo, hi;
    int l;

    if (!r)
        return 0;
    for (d = r; *d && !(d[0] == '.' && d[1] == '.'); d++)
        ;
    l = d - r;
    if (!*d || l <= 0 || l >= (int)sizeof a)
        return 0;
    memcpy(a, r, l);
    a[l] = 0;
    if (parse_int(a, &lo) || parse_int(d + 2, &hi))
        return 0;
    return v >= lo && v <= hi;
}

/* "a|b|c" contient val ? */
static int in_list(const char *list, const char *val)
{
    int vl = strlen(val);
    const char *p = list;

    if (!list || !vl)
        return 0;
    while (*p) {
        const char *e = p;

        while (*e && *e != '|')
            e++;
        if (e - p == vl && same(p, val, vl))
            return 1;
        p = *e ? e + 1 : e;
    }
    return 0;
}

static int is_on(const char *v)
{
    return !strcmp(v, "ON") || !strcmp(v, "on");
}

static int is_off(const char *v)
{
    return !strcmp(v, "OFF") || !strcmp(v, "off");
}

/* ---- alias historiques (identiques à md_inject.c avant #53) ---- */

static const char *night_name[] = {"auto", "day", "night"};

static void persist(const char *name, const char *v);

static int apply_night(const char *v)
{
    int m = !strcmp(v, "auto") ? 0 : !strcmp(v, "day") ? 1 : !strcmp(v, "night") ? 2 : -1;

    if (m < 0)
        return -1;
    say(ht_night_mode_set_irled_mode(m) == 0 ? "night mode set" : "night mode refused", 0, 0);
    state("night", night_name[m]);
    persist("night", night_name[m]);
    return 0;
}

static void publish_position(void);

static int apply_privacy(const char *v)
{
    int on = is_on(v);

    if (!on && !is_off(v))
        return -1;
    say(on ? "privacy on" : "privacy off", 0, 0);
    system(on ? "/tmp/sd/custom/scripts/privacy.sh on" : "/tmp/sd/custom/scripts/privacy.sh off");
    publish_position();
    state("privacy", on ? "ON" : "OFF");
    persist("privacy", on ? "ON" : "OFF");
    return 0;
}

/* ---- PTZ (scripts du toolkit) et haut-parleur ---- */

#define SCRIPTS "/tmp/sd/custom/scripts/"
#ifndef GW_AUDIO_DIR
#define GW_AUDIO_DIR "/tmp/sd/custom/audio/"
#endif
#define GW_OUT "/tmp/gw_out"

extern int ht_audio_codec_talk_open(void) __attribute__((weak));
extern int ht_audio_codec_talk_close(void) __attribute__((weak));
extern int ht_audio_codec_play_talk_data(unsigned char *buf, int len) __attribute__((weak));
extern int open(const char *p, int flags, ...);
extern int close(int fd);
extern long read(int fd, void *b, unsigned long n);
extern int usleep(unsigned int us);
extern int pthread_create(unsigned long *t, const void *a, void *(*f)(void *), void *arg);

/* Caractères autorisés dans les arguments passés au shell : jamais autre chose. */
static int has(const char *set, char c)
{
    for (; *set; set++)
        if (*set == c)
            return 1;
    return 0;
}

static int only(const char *s, const char *ok, int max)
{
    int n = 0;

    for (; *s; s++, n++)
        if (n >= max || !has(ok, *s))
            return 0;
    return n > 0;
}

/* Mémorise un réglage sur la SD (scripts/persist.sh, écrit seulement si la valeur change) ; rejoué au démarrage. */
static void persist(const char *name, const char *v)
{
    char c[160];

    if (!only(name, "abcdefghijklmnopqrstuvwxyz0123456789_", 32) || !only(v, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.,-", 40))
        return;
    strcpy(c, SCRIPTS "persist.sh save ");
    strcat(c, name);
    strcat(c, " ");
    strcat(c, v);
    strcat(c, " >/dev/null 2>&1 &");
    system(c);
}

/* Lance « cmd », lit sa sortie (une ligne par ligne, « ; » à la place des retours à la ligne). */
static int run_out(const char *cmd, char *o, int max)
{
    char c[200];
    int fd, n, i;

    if (strlen(cmd) + sizeof " > " GW_OUT " 2>&1" >= sizeof c)
        return -1;
    strcpy(c, cmd);
    strcat(c, " > " GW_OUT " 2>&1");
    system(c);
    o[0] = 0;
    fd = open(GW_OUT, 0);
    if (fd < 0)
        return -1;
    n = read(fd, o, max - 1);
    close(fd);
    if (n < 0)
        n = 0;
    o[n] = 0;
    for (i = n - 1; i >= 0 && (o[i] == '\n' || o[i] == '\r'); i--)
        o[i] = 0;
    for (i = 0; o[i]; i++)
        if (o[i] == '\n' || o[i] == '\r')
            o[i] = ';';
    return 0;
}

/* Santé de la SD : ON = problème (absente ou montée en lecture seule après une FAT corrompue). */
#ifndef GW_MOUNTS
#define GW_MOUNTS "/proc/mounts"
#endif
static void publish_sd_health(void)
{
    static const char sd[] = "/tmp/sd";
    char b[4096];
    int fd, n, i, j, k, s, q, field, ms, me, os, oe, found = 0, ro = 0;

    fd = open(GW_MOUNTS, 0);
    n = fd >= 0 ? read(fd, b, sizeof b - 1) : 0;
    if (fd >= 0)
        close(fd);
    if (n < 0)
        n = 0;
    b[n] = 0;
    for (i = 0; i < n; i = j + 1) {
        for (j = i; j < n && b[j] != '\n'; j++)
            ;
        /* champs : périphérique, point de montage, type, options */
        field = 0; ms = me = os = oe = -1;
        for (k = i; k < j; k++) {
            for (s = k; k < j && b[k] != ' '; k++)
                ;
            if (field == 1) { ms = s; me = k; }
            if (field == 3) { os = s; oe = k; }
            field++;
        }
        if (ms < 0 || os < 0 || me - ms != 7)
            continue;
        for (q = 0; q < 7 && b[ms + q] == sd[q]; q++)
            ;
        if (q < 7)
            continue;
        found = 1;
        for (q = os; q + 1 < oe; q++)
            if (b[q] == 'r' && b[q + 1] == 'o' && (q == os || b[q - 1] == ',') && (q + 2 == oe || b[q + 2] == ','))
                ro = 1;
    }
    state("sd_health", (!found || ro) ? "ON" : "OFF");
}

static void publish_position(void)
{
    char o[GW_MAX];

    if (!run_out(SCRIPTS "get_position", o, sizeof o) && only(o, "0123456789,-", 40))
        state("ptz_position", o);
}

static void publish_presets(void)
{
    char o[GW_MAX];

    if (!run_out(SCRIPTS "ptz_presets.sh -a get_presets", o, sizeof o))
        state("ptz_presets", o);
}

static volatile int sp_stop, sp_run, sp_loop;
static int sp_max;
static char sp_file[96], sp_state[24], sp_end[8];

/* Joue un fichier G.711 µ-law 8 kHz par ht_audio_codec_play_talk_data, au rythme réel (160 octets / 20 ms). */
static void *sp_worker(void *arg)
{
    unsigned char b[160];
    int fd, n, ticks = 0;

    (void)arg;
    ht_audio_codec_talk_open();
    while (!sp_stop && ticks < sp_max) {
        fd = open(sp_file, 0);
        if (fd < 0)
            break;
        while (!sp_stop && ticks < sp_max && (n = read(fd, b, sizeof b)) > 0) {
            ht_audio_codec_play_talk_data(b, n);
            usleep(20000);
            ticks++;
        }
        close(fd);
        if (!sp_loop)
            break;
    }
    ht_audio_codec_talk_close();
    sp_run = 0;
    if (sp_state[0])
        state(sp_state, sp_end);
    return 0;
}

static int speaker_start(const char *name, int loop, const char *st, const char *end, int max)
{
    unsigned long t;
    int w, fd;

    if (!ht_audio_codec_talk_open || !ht_audio_codec_play_talk_data || !ht_audio_codec_talk_close || !only(name, "abcdefghijklmnopqrstuvwxyz0123456789_", 24))
        return -1;
    sp_stop = 1;
    for (w = 0; sp_run && w < 100; w++)
        usleep(20000);
    if (sp_run)
        return -1;
    strcpy(sp_file, GW_AUDIO_DIR);
    strcat(sp_file, name);
    strcat(sp_file, ".ulaw");
    fd = open(sp_file, 0);
    if (fd < 0)
        return -1;
    close(fd);
    sp_stop = 0;
    sp_loop = loop;
    strcpy(sp_state, st);
    strcpy(sp_end, end);
    sp_max = max;
    sp_run = 1;
    if (pthread_create(&t, 0, sp_worker, 0)) {
        sp_run = 0;
        return -1;
    }
    return 0;
}

static void speaker_stop(void)
{
    sp_stop = 1;
}

/* Alias PTZ et haut-parleur ; renvoie 1 si le nom est un alias (résultat dans *rc). */
static int apply_alias(const char *name, const char *v, int *rc)
{
    char c[160], o[GW_MAX];
    const char *w;
    int n;

    *rc = -1;
    if (!strcmp(name, "ptz_absolute")) {
        /* « x,y » normalisés -1..1 (ptz_move -j x,y,0) */
        if (!only(v, "0123456789.,-", 24))
            return 1;
        strcpy(c, SCRIPTS "ptz_move -j ");
        strcat(c, v);
        for (n = 0, w = v; *w; w++)
            n += *w == ',';
        if (n == 1)
            strcat(c, ",0");
        say("ptz_absolute", v, 0);
        system(c);
        publish_position();
        *rc = 0;
    } else if (!strcmp(name, "ptz_preset_goto")) {
        if (!only(v, "0123456789", 3))
            return 1;
        strcpy(c, SCRIPTS "ptz_move -p ");
        strcat(c, v);
        say("ptz_preset_goto", v, 0);
        system(c);
        publish_position();
        *rc = 0;
    } else if (!strcmp(name, "ptz_preset_save")) {
        if (!only(v, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_", 24))
            return 1;
        strcpy(c, SCRIPTS "ptz_presets.sh -a add_preset -m ");
        strcat(c, v);
        if (run_out(c, o, sizeof o) || !only(o, "0123456789", 6))
            return 1;
        state("ptz_preset_saved", o);
        publish_presets();
        *rc = 0;
    } else if (!strcmp(name, "ptz_preset_delete")) {
        if (!only(v, "0123456789", 3))
            return 1;
        strcpy(c, SCRIPTS "ptz_presets.sh -a del_preset -n ");
        strcat(c, v);
        system(c);
        publish_presets();
        *rc = 0;
    } else if (!strcmp(name, "ptz_move")) {
        /* déplacement manuel : 1 s puis arrêt automatique (en arrière-plan, ne bloque pas le thread MQTT) */
        if (!strcmp(v, "stop")) {
            say("ptz_move", v, 0);
            system(SCRIPTS "ptz_move -m stop >/dev/null 2>&1 &");
            *rc = 0;
        } else if (!strcmp(v, "up") || !strcmp(v, "down") || !strcmp(v, "left") || !strcmp(v, "right")) {
            say("ptz_move", v, 0);
            strcpy(c, "(" SCRIPTS "ptz_move -m ");
            strcat(c, v);
            strcat(c, "; sleep 1; " SCRIPTS "ptz_move -m stop) >/dev/null 2>&1 &");
            system(c);
            *rc = 0;
        }
    } else if (!strcmp(name, "ptz_home")) {
        /* recalage sur les butées (ptz_move -h), comme PTZ_HOME_ON_BOOT ; long : en arrière-plan */
        say("ptz_home", 0, 0);
        system(SCRIPTS "ptz_move -h >/dev/null 2>&1 &");
        *rc = 0;
    } else if (!strcmp(name, "ptz_calibrate")) {
        /* séquence de test des deux axes (ptz_calibrate.sh), très longue : en arrière-plan */
        if (!is_on(v))
            return 1;
        say("ptz_calibrate", 0, 0);
        system(SCRIPTS "ptz_calibrate.sh >/tmp/gw_calibrate.log 2>&1 &");
        *rc = 0;
    } else if (!strcmp(name, "speaker_siren")) {
        if (is_on(v)) {
            if (speaker_start("sirene", 1, "speaker_siren", "OFF", 1500))
                return 1;
            state("speaker_siren", "ON");
            *rc = 0;
        } else if (is_off(v)) {
            speaker_stop();
            state("speaker_siren", "OFF");
            *rc = 0;
        }
    } else if (!strcmp(name, "reboot")) {
        /* redémarrage propre (sync) ; « reboot -f » si le premier échoue */
        say("reboot", 0, 0);
        system("(sync; sleep 1; reboot; sleep 20; reboot -f) >/dev/null 2>&1 &");
        *rc = 0;
    } else if (!strcmp(name, "lullaby")) {
        /* berceuse 1..5 en boucle (15 min max) depuis audio/berceuse<N>.ulaw ; « stop » l'arrête.
           Les DP ipc_lullaby_* du firmware ne sont pas utilisés : sans cloud ils font planter anyka_ipc. */
        if (!strcmp(v, "stop")) {
            speaker_stop();
            state("lullaby", "stop");
            *rc = 0;
        } else if (v[0] >= '1' && v[0] <= '5' && !v[1]) {
            char f[12];

            strcpy(f, "berceuse");
            f[8] = v[0];
            f[9] = 0;
            if (speaker_start(f, 1, "lullaby", "stop", 45000))
                return 1;
            state("lullaby", v);
            say("lullaby", v, 0);
            *rc = 0;
        }
    } else if (!strcmp(name, "speaker_play")) {
        /* nom d'un fichier <nom>.ulaw du dossier audio ; « stop » interrompt la lecture */
        if (!strcmp(v, "stop")) {
            speaker_stop();
            *rc = 0;
        } else if (!speaker_start(v, 0, "", "", 1500)) {
            say("speaker_play", v, 0);
            *rc = 0;
        }
    } else {
        return 0;
    }
    return 1;
}

/* ---- table ---- */

static const struct dp_entry *find(const char *name)
{
    int i;

    for (i = 0; i < DP_TABLE_LEN; i++)
        if (!strcmp(dp_table[i].name, name))
            return &dp_table[i];
    return 0;
}

/* « df -k /tmp/sd » : total, utilisé, libre en Kio (les 5e, 4e et 3e colonnes avant la fin, le nom du volume pouvant passer à la ligne). */
static void df_kib(unsigned long *t, unsigned long *u, unsigned long *f)
{
    char o[GW_MAX], tok[8][16];
    const char *p;
    long x[3];
    int n, k = 0, i;

    if (run_out("df -k /tmp/sd", o, sizeof o))
        return;
    p = o;
    for (n = 0; o[n]; n++)
        if (o[n] == ';')
            p = o + n + 1;
    while (*p && k < 8) {
        while (*p == ' ')
            p++;
        for (n = 0; *p && *p != ' ' && n < 15; n++)
            tok[k][n] = *p++;
        tok[k][n] = 0;
        if (n)
            k++;
    }
    if (k < 5)
        return;
    for (i = 0; i < 3; i++)
        if (parse_int(tok[k - 5 + i], &x[i]))
            return;
    *t = x[0];
    *u = x[1];
    *f = x[2];
}

static void publish_entry(const struct dp_entry *e)
{
    char out[GW_MAX];
    long r;

    if (!e->exposed || !e->get_fn || e->get_abi == ABI_NONE)
        return;
    if (e->get_abi == ABI_OUT) {
        int v = 0;

        ((void (*)(int *))e->get_fn)(&v);
        fmt_int(v, out);
        state(e->name, out);
        return;
    }
    if (e->get_abi == ABI_OUT3) {
        unsigned long a = 0, b = 0, c = 0;
        char n1[16], n2[16], n3[16];

        ((void (*)(unsigned long *, unsigned long *, unsigned long *))e->get_fn)(&a, &b, &c);
        a >>= 10; /* le firmware donne des octets (32 bits) : on publie en Kio, comme df */
        b >>= 10;
        c >>= 10;
        if (!a)
            df_kib(&a, &b, &c); /* sans cloud le service de stockage Tuya n'est pas initialisé : tout vaut 0 */
        fmt_uint(a, n1);
        fmt_uint(b, n2);
        fmt_uint(c, n3);
        strcpy(out, "{\"total\":");
        strcat(out, n1);
        strcat(out, ",\"used\":");
        strcat(out, n2);
        strcat(out, ",\"free\":");
        strcat(out, n3);
        strcat(out, "}");
        state(e->name, out);
        return;
    }
    r = e->get_fn();
    if (e->get_abi == ABI_STR) {
        const char *s = (const char *)r;
        int n = 0;

        if (!s)
            return;
        while (s[n] && n < (int)sizeof out - 1) {
            out[n] = s[n];
            n++;
        }
        out[n] = 0;
        if (e->kind == DP_BOOL)
            strcpy(out, strcmp(out, "0") && strcmp(out, "false") ? "ON" : "OFF");
    } else if (e->kind == DP_BOOL) {
        strcpy(out, r ? "ON" : "OFF");
    } else {
        fmt_int(r, out);
    }
    state(e->name, out);
}

static int apply_entry(const struct dp_entry *e, const char *v)
{
    static char sarg[GW_MAX];
    long iv = 0;
    const char *sv = v;
    int vl = strlen(v);

    if (vl >= (int)sizeof sarg)
        return -1;
    switch (e->kind) {
    case DP_BOOL:
        if (is_on(v))
            iv = 1;
        else if (!is_off(v))
            return -1;
        sv = iv ? "1" : "0";
        break;
    case DP_ENUM:
        if (!in_list(e->values, v))
            return -1;
        if (e->set_abi == ABI_INT && parse_int(v, &iv))
            return -1;
        break;
    case DP_INT:
        if (parse_int(v, &iv) || !in_range(e->values, iv))
            return -1;
        break;
    case DP_ACTION:
        if (e->values && !in_list(e->values, v))
            return -1;
        if (e->set_abi == ABI_INT && parse_int(v, &iv))
            return -1;
        break;
    case DP_JSON: /* texte libre transmis tel quel (JSON validé par le firmware) */
        if (!vl || e->set_abi != ABI_STR)
            return -1;
        break;
    default: /* sensor : lecture seule */
        return -1;
    }
    if (!e->set_fn)
        return -1;
    memcpy(sarg, sv, strlen(sv) + 1);
    if (e->set_abi == ABI_STR)
        ((long (*)(const char *))e->set_fn)(sarg);
    else if (e->set_abi == ABI_INT)
        ((long (*)(long))e->set_fn)(iv);
    else
        e->set_fn();
    say("gateway:", e->name, v);
    if (e->kind != DP_ACTION && e->kind != DP_SENSOR)
        persist(e->name, v);
    if (e->kind != DP_ACTION) {
        if (!e->get_fn || e->get_abi == ABI_NONE)
            state(e->name, e->kind == DP_BOOL ? (iv ? "ON" : "OFF") : v); /* pas de lecture : on republie la valeur commandée */
        else
            publish_entry(e);
    }
    return 0;
}

/* ---- API ---- */

void gw_init(const char *base, gw_publish_fn pub, gw_log_fn log)
{
    int l = strlen(base);

    if (l > (int)sizeof gw_base - 1)
        l = sizeof gw_base - 1;
    memcpy(gw_base, base, l);
    gw_base[l] = 0;
    memcpy(gw_sub, gw_base, l);
    memcpy(gw_sub + l, "/+/set", 7);
    gw_pub = pub;
    gw_log = log;
}

const char *gw_subscription(void)
{
    return gw_sub;
}

int gw_command(const char *name, const char *val)
{
    const struct dp_entry *e;

    if (!strcmp(name, "night"))
        return apply_night(val);
    if (!strcmp(name, "privacy"))
        return apply_privacy(val);
    {
        int rc;

        if (apply_alias(name, val, &rc)) {
            if (rc < 0)
                say("gateway: refused", name, val);
            return rc;
        }
    }
    if (!strcmp(name, "sleep_mode")) {
        /* DP 105 natif met en pause le service média Tuya et fait planter anyka_ipc sans cloud */
        if (apply_privacy(val) < 0)
            return -1;
        state("sleep_mode", is_on(val) ? "ON" : "OFF");
        return 0;
    }
    e = find(name);
    if (!e || !e->exposed) {
        say("gateway: unknown or unexposed", name, 0);
        return -1;
    }
    if (apply_entry(e, val) < 0) {
        say("gateway: refused", name, val);
        return -1;
    }
    return 0;
}

int gw_topic(const char *topic, const char *val)
{
    char name[48];
    int bl = strlen(gw_base), tl = strlen(topic), nl;

    if (tl <= bl + 5 || !same(topic, gw_base, bl) || topic[bl] != '/' || strcmp(topic + tl - 4, "/set"))
        return -1;
    nl = tl - bl - 5;
    if (nl <= 0 || nl >= (int)sizeof name)
        return -1;
    memcpy(name, topic + bl + 1, nl);
    name[nl] = 0;
    return gw_command(name, val);
}

int gw_line(char *b)
{
    int n = strlen(b);
    char *sp;

    while (n > 0 && (b[n - 1] == '\n' || b[n - 1] == '\r' || b[n - 1] == ' '))
        b[--n] = 0;
    for (sp = b; *sp && *sp != ' '; sp++)
        ;
    if (!*sp)
        return -1;
    *sp++ = 0;
    return gw_command(b, sp);
}

void gw_publish_all(void)
{
    int i;

    publish_position();
    publish_presets();
    publish_sd_health();
    state("speaker_siren", "OFF");
    state("lullaby", "stop");
    for (i = 0; i < DP_TABLE_LEN; i++)
        publish_entry(&dp_table[i]);
}

void gw_publish_sensors(void)
{
    int i;

    publish_position();
    publish_sd_health();
    for (i = 0; i < DP_TABLE_LEN; i++)
        if (dp_table[i].kind == DP_SENSOR)
            publish_entry(&dp_table[i]);
}
