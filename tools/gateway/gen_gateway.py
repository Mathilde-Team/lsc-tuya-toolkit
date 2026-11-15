#!/usr/bin/env python3
"""Génère tools/gateway/dp_table.h et docs/TOPICS.md depuis docs/firmware/DP_TABLE.md
et docs/firmware/DP_VALUES.md (confirmations des lots de décompilation).
Usage : gen_gateway.py [DP_TABLE.md] [dp_table.h] [TOPICS.md] [DP_VALUES.md]
Sans ligne « Exposé = oui » dans DP_VALUES.md, un DP garde son type heuristique et n'est PAS
exposé par la passerelle (aucun pointeur de fonction généré, aucun appel possible)."""
import os, re, sys

src = sys.argv[1] if len(sys.argv) > 1 else "docs/firmware/DP_TABLE.md"
hdr = sys.argv[2] if len(sys.argv) > 2 else "tools/gateway/dp_table.h"
doc = sys.argv[3] if len(sys.argv) > 3 else "docs/TOPICS.md"
val = sys.argv[4] if len(sys.argv) > 4 else "docs/firmware/DP_VALUES.md"
KINDS = ("bool", "enum", "int", "action", "sensor", "json")
ABIS = {"int": "ABI_INT", "str": "ABI_STR", "out": "ABI_OUT", "out3": "ABI_OUT3", "aucun": "ABI_NONE", "—": "ABI_NONE", "-": "ABI_NONE"}

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
    setter = next((f for f in fns if f.startswith("set_")), None) or next((f for f in fns if not f.startswith("get_")), None)
    getter = next((f for f in fns if f.startswith("get_")), None)
    base = (setter or getter or fns[0])
    name = re.sub(r"^(set|get)_", "", base)
    rows.append(dict(dp=dp, name=name, setter=setter, getter=getter, fn=fns[0],
                     kind=kind(name, bool(setter) or name in ACTIONS), handler=handler, addr=addr))
rows.sort(key=lambda r: r["dp"])

def load_values(path):
    """Lignes de DP_VALUES.md : | DP | Fonction | Type | Valeurs | Écriture | Lecture | Effets | Preuve | Exposé |"""
    conf = {}
    if not os.path.exists(path):
        return conf
    for l in open(path, encoding="utf-8"):
        c = [x.strip() for x in l.strip().strip("|").split("|")]
        if len(c) != 9 or not c[0].isdigit():
            continue
        dp, typ, vals, wr, rd, proof, exp = int(c[0]), c[2].strip("`").lower(), c[3], c[4].strip("`"), c[5].strip("`"), c[7], c[8].lower()
        assert typ in KINDS, "DP %d : type %r inconnu" % (dp, typ)
        assert wr in ABIS and rd in ABIS, "DP %d : écriture/lecture = int, str ou aucun" % dp
        toks = re.findall(r"`([^`]+)`", vals)
        assert wr != "out", "DP %d : « out » n'existe qu'en lecture" % dp
        if typ in ("action", "json", "sensor"):
            v = "|".join(toks) if toks and typ == "action" else None
        elif typ == "enum":
            assert toks, "DP %d : enum sans valeurs `a` `b`" % dp
            v = "|".join(toks)
        elif typ == "int":
            assert len(toks) == 2 and all(re.fullmatch(r"-?\d+", t) for t in toks), "DP %d : int = `min` `max`" % dp
            v = "%s..%s" % tuple(toks)
        else:
            v = None
        expose = exp.startswith("oui")
        assert not expose or re.search(r"0x[0-9a-f]+", proof), "DP %d exposé sans adresse dans Preuve" % dp
        conf[dp] = dict(kind=typ.upper(), values=v, set_abi=ABIS[wr], get_abi=ABIS[rd], exposed=expose, raw=vals)
    return conf

conf = load_values(val)
for r in rows:
    c = conf.get(r["dp"])
    r.update(values=None, set_abi="ABI_NONE", get_abi="ABI_NONE", exposed=False, confirmed=bool(c))
    if c:
        r.update(kind=c["kind"], values=c["values"], set_abi=c["set_abi"], get_abi=c["get_abi"],
                 exposed=c["exposed"], raw=c["raw"])
        if r["kind"] == "ACTION" and not r["setter"]:
            r["setter"] = r["fn"]
        if r["kind"] == "SENSOR":
            r["getter"] = r["getter"] or r["setter"] or r["fn"]
            r["setter"] = None

names = [r["name"] for r in rows]
dups = {n for n in names if names.count(n) > 1}
assert not dups, dups

with open(hdr, "w", encoding="utf-8") as f:
    f.write("/* Généré par tools/gateway/gen_gateway.py : ne pas éditer. */\n"
            "#ifndef DP_TABLE_H\n#define DP_TABLE_H\n\n"
            "enum dp_kind { DP_BOOL, DP_ENUM, DP_INT, DP_ACTION, DP_SENSOR, DP_JSON };\n"
            "/* Passage de la valeur (r0) : ABI_INT entier, ABI_STR chaîne, ABI_NONE sans argument / sans lecture ;\n   lecture seulement : ABI_OUT = la fonction écrit un entier via un pointeur (r0),\n   ABI_OUT3 = trois entiers non signés via trois pointeurs (r0, r1, r2), publiés en JSON. */\n"
            "enum dp_abi { ABI_NONE, ABI_INT, ABI_STR, ABI_OUT, ABI_OUT3 };\n\n"
            "/* Type générique : l'appel réel est casté selon set_abi/get_abi (gateway.c). */\n"
            "typedef long (*dp_fn)(void);\n\n"
            "struct dp_entry {\n  int dp;\n  const char *name;      /* topic : <base>/<name>[/set] */\n"
            "  enum dp_kind kind;     /* confirmé si exposed, sinon heuristique */\n"
            "  const char *setter;    /* IPC_APP_* d'écriture ou NULL */\n"
            "  const char *getter;    /* IPC_APP_* de lecture ou NULL */\n"
            "  enum dp_abi set_abi;   /* argument de setter */\n"
            "  enum dp_abi get_abi;   /* retour de getter */\n"
            "  const char *values;    /* enum : \"a|b|c\" ; int : \"min..max\" ; NULL sinon */\n"
            "  int exposed;           /* 1 = confirmé par DP_VALUES.md (Exposé = oui) */\n"
            "  dp_fn set_fn;          /* résolu au chargement (symbole faible), NULL si non exposé */\n"
            "  dp_fn get_fn;\n};\n\n")
    q = lambda s: '"IPC_APP_%s"' % s if s else "0"
    syms = []
    for r in rows:
        if r["exposed"]:
            for fn in (r["setter"], r["getter"]):
                if fn and fn not in syms:
                    syms.append(fn)
    if syms:
        f.write("/* Symboles exportés par anyka_ipc (nm -D) ; faibles : absents = NULL, jamais appelés. */\n")
        for fn in syms:
            f.write("extern long IPC_APP_%s(void) __attribute__((weak));\n" % fn)
        f.write("\n")
    f.write("static const struct dp_entry dp_table[] = {\n")
    for r in rows:
        k = r["kind"]
        setter = r["setter"] if k != "ACTION" else (r["setter"] or r["fn"])
        getter = r["getter"] if k != "ACTION" else None
        ex = r["exposed"]
        f.write('  {%d, "%s", DP_%s, %s, %s, %s, %s, %s, %d, %s, %s},\n' % (
            r["dp"], r["name"], k, q(setter), q(getter), r["set_abi"], r["get_abi"],
            '"%s"' % r["values"] if r["values"] else "0", 1 if ex else 0,
            "IPC_APP_%s" % setter if ex and setter else "0",
            "IPC_APP_%s" % getter if ex and getter else "0"))
    f.write("};\n\n#define DP_TABLE_LEN %d\n#define DP_EXPOSED_LEN %d\n\n#endif\n" % (
        len(rows), sum(1 for r in rows if r["exposed"])))

CH = {"BOOL": "ON / OFF", "ENUM": "valeur à confirmer", "INT": "entier, plage à confirmer",
      "ACTION": "ordre (charge utile ignorée ou paramètres)", "SENSOR": "lecture seule",
      "JSON": "texte JSON"}
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
- **Alias PTZ et haut-parleur** (hors table des DP, scripts du toolkit ; arguments validés avant le shell) :
  `ptz_absolute/set` = `x,y` de -1 à 1 ; `ptz_preset_goto/set` = n° ; `ptz_preset_save/set` = nom (`A-Za-z0-9_`, publie `ptz_preset_saved` = n°) ;
  `ptz_preset_delete/set` = n° ; `ptz_move/set` = `up`/`down`/`left`/`right` (1 s puis arrêt) ou `stop` ; `ptz_home/set` = recalage sur les butées ; `ptz_calibrate/set` = `ON` (séquence de test, longue) ;
  `speaker_siren/set` = `ON`/`OFF` (boucle `sirene.ulaw`, 30 s max) ; `speaker_play/set` = nom d'un fichier `<nom>.ulaw` du dossier audio ou `stop`.
  États retained : `ptz_position` (`pan,tilt,zoom`, position suivie par calcul), `ptz_presets` (`n=nom,pan,tilt,zoom;…`), `speaker_siren`.
- `sd_health` : `ON` = problème (SD absente ou montée en lecture seule après une FAT corrompue), `OFF` = saine ; lu dans `/proc/mounts`, entité HA « Problème carte SD ».
- `sd_storage` (DP 109) est publié en JSON `{"total":…,"used":…,"free":…}` en **Kio** ; si le firmware renvoie 0 (stockage Tuya non initialisé sans cloud), la valeur vient de `df -k /tmp/sd`.
- Les **types sont une heuristique** sur le nom tant qu'un lot de décompilation (issues #54 à #58) ne les a pas confirmés dans `docs/firmware/DP_VALUES.md`.
- **Seuls les DP « exposé = oui »** sont pilotés par la passerelle (`tools/gateway/gateway.c`) ; les autres sont ignorés (journal `gateway: unknown or unexposed`).
- Le canal fichier `/tmp/md_cmd` accepte une ligne `<fonction> <valeur>` (ex. `night day`, `flip_onoff ON`).

## Table

| DP | Topic | Type | Valeurs | Écriture | Lecture | Lot | Exposé |
|---|---|---|---|---|---|---|---|
""")
    for r in rows:
        lot = "#%d" % ISSUES[r["dp"]] if r["dp"] in ISSUES else "#58"
        vals = r["raw"] if r["confirmed"] else CH[r["kind"]]
        f.write("| %d | `%s` | %s | %s | %s | %s | %s | %s |\n" % (
            r["dp"], r["name"], r["kind"].lower() + ("" if r["confirmed"] else " ?"), vals,
            "`%s`" % (r["setter"] or r["fn"]) if r["kind"] != "SENSOR" else "—",
            "`%s`" % r["getter"] if r["getter"] else "—", lot, "oui" if r["exposed"] else "non"))
    if unresolved:
        f.write("\n## DP sans fonction `IPC_APP_*` identifiée\n\n| DP | Gestionnaire | Adresse |\n|---|---|---|\n")
        for dp, h, a in sorted(unresolved):
            f.write("| %d | `%s` | %s |\n" % (dp, h, a))
print(len(rows), "DP exposables,", sum(1 for r in rows if r["exposed"]), "exposés (confirmés),",
      len(unresolved), "sans fonction")
