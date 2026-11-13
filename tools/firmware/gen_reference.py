#!/usr/bin/env python3
"""Génère docs/firmware/*.md (référence de désassemblage de anyka_ipc) à partir des exports Ghidra.

Entrées (dossier --in) : calls.tsv (ExportCalls.java), functions.tsv + strings.tsv (ExportAll.java).
Le binaire sert à lire la table des DP Tuya dans .data.
Usage : gen_reference.py --in <dossier_ghidra> --bin <anyka_ipc> --out docs/firmware
"""
import argparse
import collections
import os
import re
import struct

ap = argparse.ArgumentParser()
ap.add_argument("--in", dest="inp", required=True)
ap.add_argument("--bin", required=True)
ap.add_argument("--out", required=True)
a = ap.parse_args()

# ---------------------------------------------------------------- chargement
fn = {}  # nom -> (adresse, taille, sources, nb chaînes)
addr2name = {}
order = []
for l in open(f"{a.inp}/out/functions.tsv"):
    ad, name, size, src, ns = l.rstrip("\n").split("\t")
    fn[name] = (int(ad, 16), int(size), [s for s in src.split(",") if s], int(ns))
    addr2name[int(ad, 16)] = name
    order.append(name)
out, inn = collections.defaultdict(list), collections.defaultdict(list)
for l in open(f"{a.inp}/calls.tsv"):
    c, k, t = l.rstrip("\n").split("\t")
    out[c].append((k, t))
    inn[t].append((k, c))
strs = collections.defaultdict(list)
for l in open(f"{a.inp}/out/strings.tsv"):
    f, s = l.rstrip("\n").split("\t", 1)
    strs[f].append(s)

# ---------------------------------------------------------------- module d'origine
# Les objets d'un binaire statique sont contigus : une fonction sans fichier connu entre
# deux fonctions du même fichier lui est rattachée (déduction, marquée « ~ »).
by_addr = sorted(fn, key=lambda n: fn[n][0])
src_of = {}
for n in by_addr:
    s = fn[n][2]
    if s:
        src_of[n] = (s[0], False)
known = [n for n in by_addr if n in src_of]
idx = {n: i for i, n in enumerate(by_addr)}
prev = {}
last = None
for n in by_addr:
    if n in src_of:
        last = src_of[n][0]
    prev[n] = last
nxt = {}
last = None
for n in reversed(by_addr):
    if n in src_of:
        last = src_of[n][0]
    nxt[n] = last
for n in by_addr:
    if n not in src_of and prev[n] and prev[n] == nxt[n]:
        src_of[n] = (prev[n], True)


def group(n):
    s = src_of.get(n, (None,))[0]
    if s:
        st = s.rsplit(".", 1)[0]
        for key, g in [("ht_video", "Vidéo"), ("ht_audio", "Audio"), ("ht_ptz", "PTZ"), ("ht_zoom", "PTZ"),
                       ("ht_night", "Jour/nuit"), ("ht_montion", "Détection"), ("ht_pir", "Détection"),
                       ("ht_network", "Réseau"), ("ht_record", "Enregistrement"), ("ht_playback", "Enregistrement"),
                       ("ht_tuya", "Colle Tuya (ht_tuya_*)"), ("main", "Application (main, config, système)"),
                       ("ht_", "Application (main, config, système)"), ("rtsp", "RTSP"), ("CRingBuf", "RTSP"),
                       ("ssl", "TLS"), ("x509", "TLS"), ("tuya", "SDK Tuya"), ("gw_", "SDK Tuya"), ("iot_", "SDK Tuya"),
                       ("mqc", "SDK Tuya"), ("mqtt", "SDK Tuya"), ("httpc", "SDK Tuya"), ("ipc_httpc", "SDK Tuya"),
                       ("smart_frame", "SDK Tuya"), ("simplekv", "SDK Tuya"), ("app_agent", "SDK Tuya"),
                       ("online_log", "SDK Tuya"), ("cloud_op", "SDK Tuya"), ("active_reqs", "SDK Tuya")]:
            if st.startswith(key):
                return g
        return "Autres bibliothèques"
    for pre, g in [("IPC_APP_", "Colle Tuya (ht_tuya_*)"), ("TUYA_APP_", "Colle Tuya (ht_tuya_*)"),
                   ("ht_tuya", "Colle Tuya (ht_tuya_*)"), ("ht_video", "Vidéo"), ("ht_audio", "Audio"),
                   ("ht_ptz", "PTZ"), ("ht_zoom", "PTZ"), ("ht_night", "Jour/nuit"), ("ht_montion", "Détection"),
                   ("ht_pir", "Détection"), ("ht_network", "Réseau"), ("ht_record", "Enregistrement"),
                   ("ht_playback", "Enregistrement"), ("ht_", "Application (main, config, système)"),
                   ("ak_", "SDK Anyka (ak_*)"), ("tuya_", "SDK Tuya"), ("tal_", "SDK Tuya"), ("tkl_", "SDK Tuya"),
                   ("mbedtls", "TLS"), ("ssl_", "TLS")]:
        if n.startswith(pre):
            return g
    if n.startswith("FUN_"):
        return "Inconnu"
    return "libc / divers"


G = {n: group(n) for n in fn}

NOISE = {"wirte_log", "memset", "memcpy", "strlen", "strcmp", "strncpy", "malloc", "free", "calloc", "sleep", "usleep",
         "ak_sleep_ms", "ak_thread_mutex_lock", "ak_thread_mutex_unlock", "ak_thread_mutex_init", "snprintf",
         "sprintf", "printf", "atoi", "strcpy", "strncmp", "memcmp", "ak_print_normal", "ak_print_error"}


def keep(n):
    b = n.split("@")[0]
    return b not in NOISE and (b.startswith(("ht_", "ak_", "IPC_APP_", "TUYA_APP_", "tuya_", "rtsp", "FUN_")))


def label(n):
    b = n.split("@")[0]
    return b


def mid(n):
    return re.sub(r"\W", "_", n)


def mermaid(edges, title=""):
    L = ["```mermaid", "flowchart TD"]
    nodes = {}
    for x, y in edges:
        nodes[x] = 1
        nodes[y] = 1
    for n in nodes:
        L.append(f'  {mid(n)}["{label(n)}"]')
    for x, y in edges:
        L.append(f"  {mid(x)} --> {mid(y)}")
    L.append("```")
    return L


def bfs(roots, depth=2, fan=6, pred=keep):
    edges, seen, front = [], set(roots), list(roots)
    for _ in range(depth):
        nf = []
        for r in front:
            k = 0
            for kind, t in out.get(r, []):
                if kind == "PTR" or not pred(t):
                    continue
                if k >= fan:
                    break
                k += 1
                if (r, t) not in edges:
                    edges.append((r, t))
                if t not in seen:
                    seen.add(t)
                    nf.append(t)
        front = nf
    return edges


# ---------------------------------------------------------------- table des DP
d = open(a.bin, "rb").read()
TEXT0, TEXT1 = 0x60fbc, 0x60fbc + 0x318090
dp = []
va, end = 0x50ea30, 0x50ea30 + 0x6140
cur = []
runs = []
while va < end:
    o = va - 0x20000
    i, p = struct.unpack("<II", d[o:o + 8])
    if 0 < i < 0x400 and TEXT0 <= p < TEXT1 and p % 4 == 0:
        cur.append((va, i, p))
        va += 8
    else:
        if len(cur) >= 5:
            runs.append(cur)
        cur = []
        va += 4
if len(cur) >= 5:
    runs.append(cur)
dp = max(runs, key=len) if runs else []

os.makedirs(a.out, exist_ok=True)


def w(name, lines):
    open(f"{a.out}/{name}", "w").write("\n".join(lines) + "\n")


def ipc_callees(h, depth=2):
    res, seen, front = [], {h}, [h]
    for _ in range(depth):
        nf = []
        for r in front:
            for kind, t in out.get(r, []):
                if kind != "CALL" or t in seen:
                    continue
                seen.add(t)
                if t.startswith(("IPC_APP_", "TUYA_APP_", "ht_")):
                    res.append(t)
                elif t.startswith("FUN_"):
                    nf.append(t)
        front = nf
    return res


nsrc = len([n for n in fn if n in src_of])
ninf = len([n for n in fn if n in src_of and src_of[n][1]])

# ---------------------------------------------------------------- DP_TABLE.md
L = ["# Table des commandes Tuya (DP) de `anyka_ipc`", "",
     "Généré par `tools/firmware/gen_reference.py` à partir du désassemblage. **Ne pas éditer à la main.**", "",
     f"Le binaire contient une table de {len(dp)} paires **(numéro de DP, fonction)** dans `.data` à partir de 0x{dp[0][0]:x}. "
     "Quand le service Tuya reçoit un ordre du cloud, il cherche le numéro dans cette table et appelle la fonction. "
     "Ces fonctions appellent à leur tour les `IPC_APP_*` puis les `ht_*`.", "",
     "**Conséquence pour le hack** : sans cloud, cette table n'est jamais parcourue ; on appelle directement les `IPC_APP_*` / `ht_*` (comme `md_inject.so`).", "",
     "Le nom du DP Tuya n'est **pas** dans le binaire : c'est le couple `IPC_APP_set_*` / `get_*` appelé qui dit ce que fait le DP.", "",
     "| DP | Gestionnaire | Adresse | `IPC_APP_*` / `ht_*` appelés (profondeur 2) | Source |", "|---|---|---|---|---|"]
for va_, i, p in dp:
    h = addr2name.get(p, f"0x{p:x}")
    cs = ", ".join(f"`{c}`" for c in list(dict.fromkeys(ipc_callees(h)))[:8]) if h in fn else ""
    L.append(f"| {i} | `{h}` | 0x{p:x} | {cs or '—'} | {src_of.get(h, ('?',))[0]} |")
w("DP_TABLE.md", L)

# ---------------------------------------------------------------- MODULES.md
mods = collections.defaultdict(list)
for n in fn:
    s = src_of.get(n)
    mods[s[0] if s else "(inconnu)"].append(n)
L = ["# Modules (fichiers sources) de `anyka_ipc`", "",
     "Généré par `tools/firmware/gen_reference.py`. Le fichier source vient des chaînes de journal (`xxx.c:ligne`) "
     f"référencées par chaque fonction : {nsrc - ninf} fonctions rattachées directement, {ninf} déduites par contiguïté (objets contigus dans `.text`).", "",
     "| Module | Groupe | Fonctions | Octets | Fonctions publiques `ht_*`/`IPC_APP_*` (extrait) |", "|---|---|---|---|---|"]
for m, ns in sorted(mods.items(), key=lambda x: -len(x[1])):
    pub = [n for n in ns if n.startswith(("ht_", "IPC_APP_", "TUYA_APP_"))]
    ex = ", ".join(f"`{x}`" for x in pub[:5]) + (" …" if len(pub) > 5 else "")
    L.append(f"| `{m}` | {G[ns[0]]} | {len(ns)} | {sum(fn[n][1] for n in ns) * 1} | {ex or '—'} |")
w("MODULES.md", L)

# ---------------------------------------------------------------- ARCHITECTURE.md
cnt = collections.Counter(G.values())
sz = collections.Counter()
for n in fn:
    sz[G[n]] += fn[n][1]
L = ["# Architecture de `anyka_ipc`", "",
     "Généré par `tools/firmware/gen_reference.py` (voir `README.md` pour la méthode). Les nombres viennent de Ghidra 12.2 sur le binaire 3.10.78.1269.", "",
     "## Groupes de fonctions", "", "| Groupe | Fonctions | Taille (octets) |", "|---|---|---|"]
for g, c in cnt.most_common():
    L.append(f"| {g} | {c} | {sz[g]} |")
agg = collections.Counter()
for c_, lst in out.items():
    for k, t in lst:
        if k == "PTR" or c_ not in G or t not in G:
            continue
        if G[c_] != G[t] and G[c_] not in ("libc / divers", "Inconnu") and G[t] not in ("libc / divers", "Inconnu"):
            agg[(G[c_], G[t])] += 1
ed = [(x, y, n_) for (x, y), n_ in agg.most_common(45) if n_ >= 8]
L += ["", "## Dépendances entre groupes", "",
      "Une flèche A → B signifie que des fonctions de A appellent des fonctions de B ; le nombre est le nombre de liens d'appel (8 au minimum).", "",
      "```mermaid", "flowchart LR"]
gid = {g: f"g{i}" for i, g in enumerate(cnt)}
for g in cnt:
    L.append(f'  {gid[g]}["{g}"]')
for x, y, n_ in ed:
    L.append(f"  {gid[x]} -->|{n_}| {gid[y]}")
L += ["```", "", "## Séquence de démarrage (`main`)", "",
      "Ordre des appels dans `main` (désassemblage, hors fonctions utilitaires). Le service Tuya est lancé en dernier ; sans cloud, la caméra continue avec RTSP et le matériel.", "", "```mermaid", "flowchart TD"]
seq = [t for k, t in out["main"] if k == "CALL" and t.startswith(("ht_", "ak_", "SystemCall")) and "deinit" not in t and "stop" not in t][:26]
seq = list(dict.fromkeys(seq))
prev_ = "main"
L.append('  main(["main"])')
for t in seq:
    L.append(f'  {mid(t)}["{t}"]')
    L.append(f"  {mid(prev_)} --> {mid(t)}")
    prev_ = t
L += ["```", "", "## Threads et fonctions dont l'adresse est prise", "",
      "Les fonctions qui créent un thread (`ak_thread_create`, `pthread_create`) et les fonctions dont elles prennent l'adresse (point d'entrée du thread ou callback).", "",
      "| Créateur | Fonctions dont l'adresse est prise |", "|---|---|"]
cr = sorted({c_ for _, c_ in inn.get("ak_thread_create", [])} | {c_ for _, c_ in inn.get("pthread_create", [])})
for c_ in cr:
    ptr = [t for k, t in out[c_] if k == "PTR"]
    L.append(f"| `{c_}` | {', '.join('`'+t+'`' for t in dict.fromkeys(ptr)) or '—'} |")
w("ARCHITECTURE.md", L)

# ---------------------------------------------------------------- FLOWS.md
flows = [
    ("Vidéo : capture, encodage, RTSP", ["ht_video_codec_start_encode", "ht_rtsp_start"], 3, 5),
    ("Audio : capture et lecture", ["ht_audio_codec_start_encode", "ht_audio_codec_start_decode"], 3, 5),
    ("PTZ", ["ht_ptz_set_cmd", "ht_ptz_goto_preset"], 3, 5),
    ("Jour/nuit", ["ht_night_mode_init", "ht_night_mode_set_irled_mode"], 3, 6),
    ("Détection de mouvement", ["ht_montion_detect_init", "ht_montion_detect_open"], 3, 6),
    ("Enregistrement SD", ["ht_record_init", "ht_playback_init"], 3, 5),
    ("Réseau et Wi-Fi", ["ht_network_init", "ht_network_connect_wifi"], 3, 5),
    ("Service Tuya (cloud)", ["ht_tuya_service_start"], 2, 7),
    ("Privacy / veille", ["IPC_APP_set_sleep_mode"], 3, 6),
]
L = ["# Flux (diagrammes d'appels)", "",
     "Généré par `tools/firmware/gen_reference.py`. Chaque diagramme part de fonctions connues et suit les **appels directs** (profondeur et nombre d'enfants limités ; les fonctions utilitaires sont masquées). "
     "Ce sont des **extraits**, pas le graphe complet (28 000 liens, voir `ghidra/calls.tsv` à régénérer). Les appels par pointeur de fonction n'y sont pas suivis, sauf via la table des DP (`DP_TABLE.md`).", "",
     "## Chemin d'une commande cloud (DP) et chemin local", "",
     "```mermaid", "flowchart LR",
     "  cloud[Cloud Tuya] --> sdk[SDK Tuya : tuya_ipc_dp_handler]",
     "  sdk --> table[Table des DP dans .data]",
     "  table --> h[Gestionnaire FUN_xxx]",
     "  h --> app[IPC_APP_set_*]",
     "  app --> ht[ht_* : matériel et configuration]",
     "  local[md_inject.so : MQTT, fichier /tmp/md_cmd] -->|appel direct| app",
     "  local -->|appel direct| ht",
     "```", ""]
for t, roots, dpth, fan in flows:
    roots = [r for r in roots if r in fn]
    if not roots:
        continue
    L += [f"## {t}", ""]
    L += mermaid(bfs(roots, dpth, fan))
    L.append("")
w("FLOWS.md", L)

# ---------------------------------------------------------------- stats pour README
open(f"{a.out}/.stats", "w").write(f"{len(fn)} {len(dp)} {nsrc} {ninf}\n")
print(len(fn), "fonctions,", len(dp), "DP,", nsrc, "avec module,", ninf, "déduits")
