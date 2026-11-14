#!/usr/bin/env python3
"""Génère tools/gateway/dp_table.h et docs/TOPICS.md depuis docs/firmware/DP_TABLE.md.
Usage : gen_gateway.py [DP_TABLE.md] [dp_table.h] [TOPICS.md]
Les types sont une heuristique sur le nom ; ils sont à confirmer par les lots de décompilation (DP_VALUES.md).
La convention d'appel des IPC_APP_* (int f(int)) est une hypothèse de travail : voir l'en-tête de dp_table.h."""
import re, sys

src = sys.argv[1] if len(sys.argv) > 1 else "docs/firmware/DP_TABLE.md"
hdr = sys.argv[2] if len(sys.argv) > 2 else "tools/gateway/dp_table.h"
doc = sys.argv[3] if len(sys.argv) > 3 else "docs/TOPICS.md"

ACTIONS = ("reboot", "format_sd_card", "unmount_sd_card", "ptz_start_move", "ptz_stop_move", "ptz_check")
ISSUES = {  # DP -> issue de lot
    103: 54, 104: 54, 188: 54, 198: 54, 162: 54,
    106: 55, 134: 55, 120: 55, 139: 55, 140: 55, 167: 55, 168: 55, 169: 55, 170: 55, 159: 55, 247: 55,
    109: 56, 110: 56, 111: 56, 112: 56, 117: 56, 150: 56, 151: 56,
    116: 57, 119: 57, 132: 57, 161: 57, 174: 57, 175: 57, 176: 57, 177: 57, 178: 57, 179: 57, 190: 57, 199: 57,
}

def kind(name, has_set):
    if name in ACTIONS:
        return "ACTION"
    if not has_set:
        return "SENSOR"
    if re.search(r"(onoff|switch|enable)$", name):
        return "BOOL"
    if re.search(r"(mode|state|status|select|control|command|list|draw)$", name):
        return "ENUM"
    return "INT"

rows, unresolved = [], []
for l in open(src, encoding="utf-8"):
    m = re.match(r"\| (\d+) \| `(\S+)` \| (0x[0-9a-f]+) \| (.*?) \| ", l)
    if not m:
        continue
    dp, handler, addr, fns = int(m[1]), m[2], m[3], re.findall(r"`IPC_APP_(\w+)`", m[4])
    if not fns:
        unresolved.append((dp, handler, addr))
        continue
    setter = next((f for f in fns if f.startswith("set_")), None)
    getter = next((f for f in fns if f.startswith("get_")), None)
    base = (setter or getter or fns[0])
    name = re.sub(r"^(set|get)_", "", base)
    rows.append(dict(dp=dp, name=name, setter=setter, getter=getter, fn=fns[0],
                     kind=kind(name, bool(setter) or name in ACTIONS), handler=handler, addr=addr))
rows.sort(key=lambda r: r["dp"])

names = [r["name"] for r in rows]
dups = {n for n in names if names.count(n) > 1}
assert not dups, dups

with open(hdr, "w", encoding="utf-8") as f:
    f.write("/* Généré par tools/gateway/gen_gateway.py : ne pas éditer. */\n"
            "#ifndef DP_TABLE_H\n#define DP_TABLE_H\n\n"
            "/* Convention d'appel ASSUMÉE, à confirmer lot par lot (issues #54 à #58,\n"
            " * livrable docs/firmware/DP_VALUES.md) : setter int f(int valeur) ; getter\n"
            " * int f(int), appelé avec 0. Le runtime (tools/motion/md_inject.c) n'appelle\n"
            " * une entrée que si elle est activée explicitement (DP_ENABLE), donc rien\n"
            " * n'est exposé tant que le lot correspondant n'a pas confirmé la ligne. */\n\n"
            "enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR };\n\n"
            "struct dp_entry {\n"
            "  int dp;\n"
            "  const char *name;      /* topic : <base>/<name>[/set] */\n"
            "  enum dp_kind kind;     /* heuristique, voir DP_VALUES.md */\n"
            "  int (*set)(int);       /* IPC_APP_set_* ou action ; NULL = lecture seule */\n"
            "  int (*get)(int);       /* IPC_APP_get_* ; NULL = pas d'état lisible */\n"
            "};\n\n")
    externs = sorted({("IPC_APP_%s" % r["fn"]) for r in rows if r["kind"] == "ACTION"} |
                     {("IPC_APP_%s" % r["setter"]) for r in rows if r["setter"]} |
                     {("IPC_APP_%s" % r["getter"]) for r in rows if r["getter"]})
    for e in externs:
        f.write("extern int %s(int);\n" % e)
    f.write("\nstatic const struct dp_entry dp_table[] = {\n")
    for r in rows:
        k = r["kind"]
        if k == "ACTION":
            f.write('  {%d, "%s", DP_ACTION, IPC_APP_%s, 0},\n' % (r["dp"], r["name"], r["fn"]))
        else:
            set_ = "IPC_APP_%s" % r["setter"] if r["setter"] else "0"
            get_ = "IPC_APP_%s" % r["getter"] if r["getter"] else "0"
            f.write('  {%d, "%s", DP_%s, %s, %s},\n' % (r["dp"], r["name"], k, set_, get_))
    f.write("};\n\n#define DP_TABLE_LEN %d\n\n#endif\n" % len(rows))

CH = {"BOOL": "ON / OFF", "ENUM": "valeur à confirmer", "INT": "entier, plage à confirmer",
      "ACTION": "ordre (charge utile ignorée ou paramètres)", "SENSOR": "lecture seule"}
with open(doc, "w", encoding="utf-8") as f:
    f.write("""# Convention de topics MQTT (passerelle locale)

Généré par `tools/gateway/gen_gateway.py` depuis `docs/firmware/DP_TABLE.md`. **Ne pas éditer le tableau à la main.**

## Règles

- `<base>` = nom de la caméra (ex. `camera188`), déjà utilisé par `night` et `privacy`.
- **Commande** : `<base>/<fonction>/set`. **État** : `<base>/<fonction>`, publié *retained*.
- `<fonction>` = nom de l'`IPC_APP_set_*`/`get_*` sans préfixe (ex. `IPC_APP_set_flip_onoff` → `flip_onoff`).
- Charges utiles : booléen `ON`/`OFF` ; énumération et entier en texte ; action = toute charge utile (les paramètres, s'il y en a, viennent des lots de décompilation).
- Fonction sans `set_` : lecture seule (`sensor`), pas de topic `/set`.
- Le même ordre peut venir d'ONVIF : les deux canaux écrivent dans `/tmp/md_cmd`.
- **Alias historiques conservés** : `<base>/night` (= `night_mode`, DP 108) et `<base>/privacy` (preset privé).
- **Activation** : aucune ligne n'est abonnée ni publiée au démarrage ; `DP_ENABLE` (voir `tools/motion/README.md`) active des lignes pour test, dans l'attente des lots de décompilation.
- Les **types sont une heuristique** sur le nom ; chaque ligne est confirmée par un lot de décompilation (issues #54 à #58) avant d'être exposée.

## Table

| DP | Topic | Type | Valeurs | Écriture | Lecture | Lot |
|---|---|---|---|---|---|---|
""")
    for r in rows:
        lot = "#%d" % ISSUES[r["dp"]] if r["dp"] in ISSUES else "#58"
        f.write("| %d | `%s` | %s | %s | %s | %s | %s |\n" % (
            r["dp"], r["name"], r["kind"].lower(), CH[r["kind"]],
            "`%s`" % (r["setter"] or r["fn"]) if r["kind"] != "SENSOR" else "—",
            "`%s`" % r["getter"] if r["getter"] else "—", lot))
    if unresolved:
        f.write("\n## DP sans fonction `IPC_APP_*` identifiée\n\n| DP | Gestionnaire | Adresse |\n|---|---|---|\n")
        for dp, h, a in sorted(unresolved):
            f.write("| %d | `%s` | %s |\n" % (dp, h, a))
print(len(rows), "DP exposables,", len(unresolved), "sans fonction")
