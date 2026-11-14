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

static int apply_night(const char *v)
{
    int m = !strcmp(v, "auto") ? 0 : !strcmp(v, "day") ? 1 : !strcmp(v, "night") ? 2 : -1;

    if (m < 0)
        return -1;
    say(ht_night_mode_set_irled_mode(m) == 0 ? "night mode set" : "night mode refused", 0, 0);
    state("night", night_name[m]);
    return 0;
}

static int apply_privacy(const char *v)
{
    int on = is_on(v);

    if (!on && !is_off(v))
        return -1;
    say(on ? "privacy on" : "privacy off", 0, 0);
    system(on ? "/tmp/sd/custom/scripts/privacy.sh on" : "/tmp/sd/custom/scripts/privacy.sh off");
    state("privacy", on ? "ON" : "OFF");
    return 0;
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
    if (e->kind != DP_ACTION)
        publish_entry(e);
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

    for (i = 0; i < DP_TABLE_LEN; i++)
        publish_entry(&dp_table[i]);
}

void gw_publish_sensors(void)
{
    int i;

    for (i = 0; i < DP_TABLE_LEN; i++)
        if (dp_table[i].kind == DP_SENSOR)
            publish_entry(&dp_table[i]);
}
