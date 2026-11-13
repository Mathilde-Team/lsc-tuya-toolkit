#!/usr/bin/env python3
"""Génère docs/FIRMWARE_COMMANDS.md par désassemblage de anyka_ipc.
Usage : gen_commands.py <anyka_ipc> <sortie.md>   (nécessite arm-linux-gnueabi-objdump/nm)"""
import re, subprocess, sys

binf, out = sys.argv[1], sys.argv[2]
nm = subprocess.check_output(["arm-linux-gnueabi-nm", "-D", binf], text=True)
sym = {p[2]: p[0] for p in (l.split() for l in nm.splitlines()) if len(p) == 3 and p[1] == "T"}
dis = subprocess.check_output(["arm-linux-gnueabi-objdump", "-d", binf], text=True, errors="ignore")

funcs, cur = {}, None
for l in dis.split("\n"):
    m = re.match(r"^[0-9a-f]+ <(.+?)@@Base>:", l)
    if m:
        cur = m.group(1)
        funcs[cur] = []
    elif cur and l.startswith("   "):
        funcs[cur].append(l)

def info(n):
    ins, regs, calls = funcs.get(n, []), set(), []
    for l in ins[:14]:
        m = re.search(r"str\s+(r[0-3]), \[(fp|sp)", l)
        if m:
            regs.add(m.group(1))
        if re.search(r"\sbl?\s", l):
            break
    for l in ins:
        m = re.search(r"bl\s+[0-9a-f]+ <([^@+>]+)", l)
        if m:
            calls.append(m.group(1))
    return len(regs), list(dict.fromkeys(calls))

VERBS = {"set": "Règle", "get": "Lit", "start": "Démarre", "stop": "Arrête", "init": "Initialise",
         "deinit": "Libère", "open": "Ouvre", "close": "Ferme", "enable": "Active", "disable": "Désactive",
         "pause": "Met en pause", "resume": "Reprend", "check": "Vérifie", "save": "Enregistre",
         "load": "Charge", "reset": "Réinitialise", "sync": "Synchronise", "goto": "Va à", "update": "Met à jour",
         "send": "Envoie", "recv": "Reçoit", "create": "Crée", "destroy": "Détruit", "read": "Lit", "write": "Écrit"}

def role(n):
    t = re.sub(r"^(IPC_APP_|ht_tuya_func_|ht_)", "", n).split("_")
    v = VERBS.get(t[0])
    return (v + " " + " ".join(t[1:])) if v else " ".join(t)

# Connu par lecture du désassemblage (sinon « non analysé »)
KNOWN = {
 "IPC_APP_set_night_mode": ("Mode de la LED IR / nuit (clé enum_night_mode)", "chaîne « 0 »/« 1 »/« 2 »", "MQTT <base>/night/set, ONVIF IrCutFilter (via md_inject)", "testé"),
 "ht_night_mode_set_irled_mode": ("Mode jour/nuit : change l'état +16 sous mutex, met la détection en pause", "0 auto, 1 jour, 2 nuit forcée ; refusé si mode couleur (+12) = 2", "MQTT <base>/night/set, ONVIF IrCutFilter ON/OFF/AUTO", "utilisé"),
 "ht_night_mode_set": ("Mode « couleur » (+12) et éclairage", "0..2 (2 = lumière d'appoint)", "non", "non testé"),
 "ht_night_mode_get": ("Retourne le mode courant (+12)", "—", "non", "non testé"),
 "ht_night_mode_get_state": ("Retourne l'état jour/nuit courant", "—", "non", "non testé"),
 "ht_night_mode_check_is_night": ("Vrai si l'état courant est nuit", "—", "non", "non testé"),
 "IPC_APP_set_night_vision_mode": ("Vision nocturne IR ou couleur (enum_night_vision_mode)", "chaîne « 0 »/« 1 »", "non", "non testé"),
 "IPC_APP_set_sleep_mode": ("Veille/privacy cloud : arrête le stockage, l'enregistrement et envoie ht_ptz_set_cmd(9/10)", "0 ou 1", "non (dépend du cloud Tuya)", "non testé"),
 "IPC_APP_get_sleep_mode": ("Lit bool_sleep_mode", "—", "non", "non testé"),
 "ht_ptz_set_cmd": ("Commande de veille PTZ", "9 = entrer en veille, 10 = sortir ; autres ignorées", "non", "non testé"),
 "ht_ptz_goto_preset": ("Va à une position mémorisée", "numéro de preset", "ONVIF GotoPreset (via ptz_move -p)", "testé"),
 "ht_montion_detect_open": ("Active la détection de mouvement", "aucun", "MQTT <base>/motion (ON/OFF) via md_inject", "testé"),
 "ht_montion_detect_set_active_cb": ("Enregistre la fonction appelée à chaque mouvement", "pointeur de fonction(long long ms)", "MQTT motion", "testé"),
 "ht_montion_detect_set_sensitivity": ("Sensibilité de détection", "0..2", "MOTION_SENS (hack.conf), pas de commande à chaud", "testé"),
 "ht_montion_detect_set_human_filter": ("Filtre « humain »", "0 ou 1", "MOTION_HUMAN (hack.conf)", "testé, efficacité non démontrée"),
 "ht_tuya_func_set_night_mode": ("Appelle ht_night_mode_set_irled_mode", "entier 0..2", "indirect (voir IPC_APP_set_night_mode)", "non testé"),
 "ht_tuya_func_set_night_vision_mode": ("Appelle ht_night_mode_set", "entier", "non", "non testé"),
}

def expo(n):
    if n in KNOWN:
        return KNOWN[n]
    return None

groups = [("IPC_APP_", "Commandes applicatives Tuya (`IPC_APP_*`)"), ("ht_tuya_func_", "Réglages Tuya (`ht_tuya_func_*`)"),
          ("ht_ptz_", "PTZ (`ht_ptz_*`)"), ("ht_night_", "Vision nocturne (`ht_night_mode_*`)"),
          ("ht_montion_", "Détection de mouvement (`ht_montion_detect_*`)"), ("ht_audio_codec_", "Audio (`ht_audio_codec_*`)"),
          ("ht_video_", "Vidéo, OSD, codec (`ht_video_*`)"), ("ht_config", "Configuration (`ht_config_*`)"),
          ("ht_system", "Système (`ht_system_*`)"), ("ht_tuya_", "Autres `ht_tuya_*`")]

def values(n, calls):
    if "atoi@plt" in calls or "atoi" in calls:
        return "chaîne numérique (atoi)"
    if "ht_tuya_config_set_boolean" in calls:
        return "0 ou 1 (booléen)"
    if "ht_tuya_config_set_integer" in calls:
        return "entier"
    if "ht_tuya_config_set_string" in calls:
        return "chaîne"
    return "non analysé"

L = ["# Commandes connues du firmware (anyka_ipc 3.10.78.1269)", "",
     "Liste obtenue **par désassemblage / table des symboles** de `anyka_ipc` (`tools/firmware/gen_commands.py`, `arm-linux-gnueabi-nm -D` + `objdump -d`).",
     "Toutes les fonctions listées sont des symboles `T` exportés : **appelables depuis une bibliothèque `LD_PRELOAD`** (comme `md_inject.so`).", "",
     "Colonnes :", "",
     "- **Args** : nombre de registres r0–r3 sauvegardés dans le prologue (≈ nombre d'arguments, approximation).",
     "- **Rôle** : en *gras-italique* = confirmé par lecture du désassemblage ; sinon simple lecture du nom (**hypothèse**).",
     "- **Valeurs** : valeurs attendues ; déduites des appels internes (`atoi`, `ht_tuya_config_set_*`) sauf si confirmées.",
     "- **Exposé** : où la fonction est atteignable aujourd'hui (MQTT, ONVIF, `hack.conf`) ; « non » = seulement par appel direct.",
     "- **Utilisable** : « testé » = essayée sur la caméra ; sinon « non testé ».", "",
     "**Principe : toute affirmation sur le firmware doit venir du désassemblage** (`objdump -d`), pas d'une supposition.", ""]
used = set()
for pre, title in groups:
    names = sorted(n for n in sym if n.startswith(pre) and n not in used)
    if not names:
        continue
    L += [f"## {title} ({len(names)})", "", "| Fonction | Adresse | Args | Rôle | Valeurs | Exposé | Utilisable |", "|---|---|---|---|---|---|---|"]
    for n in names:
        a, calls = info(n)
        k = expo(n)
        if k:
            r, v, e, u = k
            r = f"***{r}***"
        else:
            r, v, e, u = role(n), values(n, calls), "non", "non testé"
        L.append(f"| `{n}` | 0x{sym[n]} | {a} | {r} | {v} | {e} | {u} |")
    L.append("")
    used.update(names)
open(out, "w").write("\n".join(L) + "\n")
print(len(used), "fonctions")
