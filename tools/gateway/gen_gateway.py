#!/usr/bin/env python3
"""Génère tools/gateway/dp_table.h, tools/gateway/dp_calls.h et docs/TOPICS.md
depuis docs/firmware/DP_TABLE.md.
Usage : gen_gateway.py [DP_TABLE.md] [dp_table.h] [TOPICS.md] [dp_calls.h]
Les types sont une heuristique sur le nom ; ils sont à confirmer par les lots de décompilation (DP_VALUES.md)."""
import re, sys

src = sys.argv[1] if len(sys.argv) > 1 else "docs/firmware/DP_TABLE.md"
hdr = sys.argv[2] if len(sys.argv) > 2 else "tools/gateway/dp_table.h"
doc = sys.argv[3] if len(sys.argv) > 3 else "docs/TOPICS.md"
calls = sys.argv[4] if len(sys.argv) > 4 else "tools/gateway/dp_calls.h"

# Getters dont la signature est confirmée par un lot de décompilation (#54 à #58) :
# DP -> nom de fonction. Un getter absent d'ici n'est jamais appelé par le runtime
# (signature inconnue : risque de plantage). Le lot ajoute le DP et régénère.
CONFIRMED_GETTERS = {}

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
            "enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR };\n\n"
            "struct dp_entry {\n  int dp;\n  const char *name;      /* topic : <base>/<name>[/set] */\n"
            "  enum dp_kind kind;     /* heuristique, voir DP_VALUES.md */\n"
            "  const char *setter;    /* IPC_APP_* d'écriture ou NULL */\n"
            "  const char *getter;    /* IPC_APP_* de lecture ou NULL */\n};\n\n"
            "static const struct dp_entry dp_table[] = {\n")
    for r in rows:
        q = lambda s: '"IPC_APP_%s"' % s if s else "0"
        k = r["kind"]
        if k == "ACTION":
            f.write('  {%d, "%s", DP_ACTION, %s, 0},\n' % (r["dp"], r["name"], q(r["fn"])))
        else:
            f.write('  {%d, "%s", DP_%s, %s, %s},\n' % (r["dp"], r["name"], k, q(r["setter"]), q(r["getter"])))
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

with open(calls, "w", encoding="utf-8") as f:
    f.write("/* Généré par tools/gateway/gen_gateway.py : ne pas éditer.\n"
            "   Couche d'appel du runtime (md_inject.c) vers les IPC_APP_* du firmware.\n"
            "   Convention provisoire des setters : int IPC_APP_set_X(const char *) — la valeur d'un DP\n"
            "   Tuya est une chaîne (docs/FIRMWARE_COMMANDS.md : « chaîne numérique (atoi) » ;\n"
            "   IPC_APP_set_night_mode, confirmé au désassemblage, prend « 0 »/« 1 »/« 2 »).\n"
            "   Chaque lot de décompilation (#54 à #58) confirme la signature réelle dans DP_VALUES.md\n"
            "   et ajuste CONFIRMED_GETTERS dans gen_gateway.py. */\n"
            "#ifndef DP_CALLS_H\n#define DP_CALLS_H\n\n#include \"dp_table.h\"\n\n")
    for r in rows:
        call = r["setter"] or (r["fn"] if r["kind"] == "ACTION" else None)
        if call:
            f.write("extern int IPC_APP_%s(const char *);\n" % call)
    for dp, fn in sorted(CONFIRMED_GETTERS.items()):
        f.write("extern int %s(void);\n" % fn)
    f.write("\n/* Commande : renvoie 0 si le firmware a accepté, -1 si inconnu/refusé. */\n"
            "static int dp_call_set(const struct dp_entry *e, const char *v)\n{\n"
            "  switch (e->dp) {\n")
    for r in rows:
        call = r["setter"] or (r["fn"] if r["kind"] == "ACTION" else None)
        if call:
            f.write("  case %d: return IPC_APP_%s(v);\n" % (r["dp"], call))
    f.write("  default: return -1;\n  }\n}\n\n"
            "/* État : appelé seulement pour les getters confirmés par un lot (sinon -1, jamais appelé\n"
            "   sur la caméra : signature inconnue). Renvoie la valeur, -1 si non confirmé. */\n"
            "static int dp_call_get(const struct dp_entry *e)\n{\n"
            "  switch (e->dp) {\n")
    for r in rows:
        if not r["getter"]:
            continue
        if r["dp"] in CONFIRMED_GETTERS:
            f.write("  case %d: return %s();\n" % (r["dp"], CONFIRMED_GETTERS[r["dp"]]))
        else:
            f.write("  case %d: break; /* IPC_APP_%s : signature à confirmer (lot #%d) */\n"
                    % (r["dp"], r["getter"], ISSUES.get(r["dp"], 58)))
    f.write("  default: break;\n  }\n  return -1;\n}\n\n#endif\n")

print(len(rows), "DP exposables,", len(unresolved), "sans fonction,",
      len(CONFIRMED_GETTERS), "getters confirmés")
