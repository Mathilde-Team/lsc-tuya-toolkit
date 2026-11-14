/*
 * Découverte MQTT Home Assistant (issue #59) : publie au démarrage, en retained, les messages de
 * ha_discovery.h (générés depuis dp_table.h par gen_ha_discovery.py).
 * Mêmes contraintes uClibc que gateway.c : pas de division, pas de nouveau symbole libc.
 */
#include <string.h>
#include "gateway.h"
#ifdef GW_HA_TABLE
#include GW_HA_TABLE /* tests */
#else
#include "ha_discovery.h"
#endif

/* Identifiant de nœud HA : base avec tout caractère hors [A-Za-z0-9_-] remplacé par '_'. */
static void node_id(const char *b, char *o, int max)
{
    int n = 0;

    for (; *b && n < max - 1; b++) {
        char c = *b;

        o[n++] = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' ? c : '_';
    }
    o[n] = 0;
}

/* Une valeur insérée dans du JSON ne doit contenir ni guillemet, ni antislash, ni '%'. */
static int safe(const char *s)
{
    for (; *s; s++)
        if (*s == '"' || *s == '\\' || *s == '%' || (unsigned char)*s < 32)
            return 0;
    return 1;
}

static int append(char *o, int n, int max, const char *s)
{
    while (*s && n < max - 1)
        o[n++] = *s++;
    if (*s)
        return -1;
    o[n] = 0;
    return n;
}

int gw_ha_expand(const char *tpl, const char *base, const char *node, const char *motion, char *o, int max)
{
    int n = 0;

    o[0] = 0;
    while (*tpl) {
        if (tpl[0] == '%' && (tpl[1] == 'B' || tpl[1] == 'N' || tpl[1] == 'M')) {
            n = append(o, n, max, tpl[1] == 'B' ? base : tpl[1] == 'N' ? node : motion);
            tpl += 2;
        } else {
            char c[2];

            c[0] = *tpl++;
            c[1] = 0;
            n = append(o, n, max, c);
        }
        if (n < 0)
            return -1;
    }
    return n;
}

int gw_ha_announce(const char *prefix, const char *base, const char *motion, gw_publish_fn pub, gw_log_fn log)
{
    char node[64], topic[128], payload[900];
    int i, n, ok = 0;

    if (!pub)
        return -1;
    if (!safe(prefix) || !safe(base) || !safe(motion)) {
        if (log)
            log("ha discovery skipped: invalid character in topic");
        return -1;
    }
    node_id(base, node, sizeof node);
    for (i = 0; i < HA_TABLE_LEN; i++) {
        n = 0;
        topic[0] = 0;
        n = append(topic, n, sizeof topic, prefix);
        if (n >= 0)
            n = append(topic, n, sizeof topic, "/");
        if (n >= 0)
            n = append(topic, n, sizeof topic, ha_table[i].component);
        if (n >= 0)
            n = append(topic, n, sizeof topic, "/");
        if (n >= 0)
            n = append(topic, n, sizeof topic, node);
        if (n >= 0)
            n = append(topic, n, sizeof topic, "/");
        if (n >= 0)
            n = append(topic, n, sizeof topic, ha_table[i].object);
        if (n >= 0)
            n = append(topic, n, sizeof topic, "/config");
        if (n < 0 || gw_ha_expand(ha_table[i].payload, base, node, motion, payload, sizeof payload) < 0)
            continue;
        if (pub(topic, payload) == 0)
            ok++;
    }
    if (log)
        log(ok == HA_TABLE_LEN ? "ha discovery published" : "ha discovery incomplete");
    return ok == HA_TABLE_LEN ? 0 : -1;
}
