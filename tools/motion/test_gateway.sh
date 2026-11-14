#!/bin/sh
# Test logiciel local de la passerelle générique (issue #53), SANS caméra :
#   - version hôte de md_inject.c (gcc, pas de -nostdlib) ;
#   - stubs des fonctions du firmware générés depuis tools/gateway/dp_table.h ;
#   - broker mosquitto local jetable.
# Vérifie : défaut « rien d'exposé », alias night/privacy inchangés, commandes et états
# retained des entrées actives (bool, entier, capteur, action), canal /tmp/md_cmd,
# entrées non activées ignorées, jeton inconnu de DP_ENABLE journalisé.
# Prérequis : gcc, python3, mosquitto, mosquitto_pub, mosquitto_sub.
# Usage : tools/motion/test_gateway.sh [port_mqtt]   (défaut 18883)
set -u
cd "$(dirname "$0")"
PORT=${1:-18883}
WORK=$(mktemp -d /tmp/mdtest.XXXXXX)
FAIL=0

cleanup()
{
    [ -n "${MAINPID:-}" ] && kill "$MAINPID" 2>/dev/null
    [ -n "${MOSQ:-}" ] && kill "$MOSQ" 2>/dev/null
    rm -rf "$WORK"
}
trap cleanup EXIT INT TERM

ok()   { echo "PASS: $1"; }
bad()  { echo "FAIL: $1"; FAIL=1; }

# ---------- génération des stubs (toutes les fonctions référencées par dp_table.h) ----------
python3 - "$WORK" <<'EOF' || exit 1
import re, sys
work = sys.argv[1]
src = open('../gateway/dp_table.h').read()
fns = re.findall(r'extern int (\w+)\(int\);', src)
bases = {}
for f in fns:
    if f.startswith('IPC_APP_set_'):
        bases.setdefault(f[12:], {})['set'] = f
    elif f.startswith('IPC_APP_get_'):
        bases.setdefault(f[12:], {})['get'] = f
    else:
        bases.setdefault(f, {})['act'] = f
out = ['#include <stdio.h>', '#include <stdlib.h>', '#include <unistd.h>', '''
static FILE *lgf;
static void rec(const char *n, int v)
{
    const char *p = getenv("STUBLOG");
    if (!lgf && p)
        lgf = fopen(p, "a");
    if (lgf) {
        fprintf(lgf, "%s %d\\n", n, v);
        fflush(lgf);
    }
}
''']
for base, d in sorted(bases.items()):
    out.append('static int st_%s;' % base)
    if 'set' in d:
        out.append('int %s(int v) { rec("%s", v); st_%s = v; return 0; }' % (d['set'], d['set'][8:], base))
    if 'act' in d:
        out.append('int %s(int v) { rec("%s", v); return 0; }' % (d['act'], d['act'][8:]))
    if 'get' in d:
        out.append('int %s(int v) { (void)v; rec("%s", st_%s); return st_%s; }' % (d['get'], d['get'][8:], base, base))
out.append('''
int ht_night_mode_set_irled_mode(int m) { rec("ht_night_mode_set_irled_mode", m); return 0; }
int ht_montion_detect_set_active_cb(void (*cb)(long long)) { (void)cb; return 0; }
int ht_montion_detect_open(void) { return 0; }
int ht_montion_detect_set_sensitivity(int s) { rec("ht_montion_detect_set_sensitivity", s); return 0; }
int ht_montion_detect_set_human_filter(int h) { rec("ht_montion_detect_set_human_filter", h); return 0; }
int main(void) { for (;;) pause(); }
''')
open(work + '/stubs.c', 'w').write('\n'.join(out))
EOF

# ---------- compilation hôte ----------
gcc -O1 -g -shared -fPIC -DSTARTUP_DELAY_S=1 -I../gateway -o "$WORK/md_host.so" md_inject.c || exit 1
gcc -O1 -g -rdynamic -o "$WORK/test_main" "$WORK/stubs.c" || exit 1
printf '127.0.0.1\n%s\n\n\n' "$PORT" > "$WORK/mqtt.cred"

pub() { mosquitto_pub -h 127.0.0.1 -p "$PORT" -t "$1" -m "$2" ${3:+-r}; }
# valeur retained d'un topic (vide si absent)
ret() { mosquitto_sub -h 127.0.0.1 -p "$PORT" -t "$1" -C 1 -W 3 2>/dev/null; }

# ---------- broker local ----------
mosquitto -p "$PORT" >/dev/null 2>&1 &
MOSQ=$!
sleep 1
mosquitto_pub -h 127.0.0.1 -p "$PORT" -t mdtest/ping -m x || { echo "FAIL: broker mosquitto injoignable"; exit 1; }

# lance une instance : base topic, DP_ENABLE, fichier journal
launch()
{
    rm -f /tmp/md_cmd
    STUBLOG="$WORK/stub_$3.log" MOTION_MQTT_FILE="$WORK/mqtt.cred" MOTION_TOPIC="$1/motion" \
        DP_ENABLE="$2" DP_POLL=3 LD_PRELOAD="$WORK/md_host.so" \
        "$WORK/test_main" > "$WORK/log_$3.txt" 2>&1 &
    MAINPID=$!
    sleep 8
}

# ---------- instance A : défaut, DP_ENABLE vide ----------
launch "cama" "" a

pub cama/night/set day
sleep 1
[ "$(ret cama/night)" = "day" ] && ok "alias night : état retained" || bad "alias night : état retained"
sleep 1
grep -q "^ht_night_mode_set_irled_mode 1$" "$WORK/stub_a.log" && ok "alias night : ht_night_mode_set_irled_mode(1) appelé" \
    || bad "alias night : ht_night_mode_set_irled_mode(1) appelé"

pub cama/privacy/set ON
sleep 1
[ "$(ret cama/privacy)" = "ON" ] && ok "privacy : état retained" || bad "privacy : état retained"

pub cama/flip_onoff/set ON
sleep 1
[ -z "$(ret cama/flip_onoff)" ] && ok "défaut : entrée non activée pas exposée (topic absent)" \
    || bad "défaut : entrée non activée pas exposée (topic absent)"
grep -q "set_flip_onoff" "$WORK/stub_a.log" && bad "défaut : setter appelé à tort" \
    || ok "défaut : aucun setter appelé"

echo "watermark_onoff ON" > /tmp/md_cmd
sleep 2
[ -z "$(ret cama/watermark_onoff)" ] && grep -q "set_watermark_onoff" "$WORK/stub_a.log" && bad "défaut : /tmp/md_cmd a activé une entrée" \
    || ok "défaut : /tmp/md_cmd ignore les entrées non activées"

kill "$MAINPID" 2>/dev/null; wait "$MAINPID" 2>/dev/null; MAINPID=""

# ---------- instance B : entrées actives 103 (bool), 106 (int), 110 (capteur), 111 (action) ----------
launch "camb" "103,106,110,111,bidon" b

grep -q "dp_enable ignored: bidon" "$WORK/log_b.txt" && ok "DP_ENABLE : jeton inconnu journalisé" \
    || bad "DP_ENABLE : jeton inconnu journalisé"
[ "$(ret camb/sd_status)" = "0" ] && ok "capteur : état retained publié au démarrage" \
    || bad "capteur : état retained publié au démarrage"

pub camb/flip_onoff/set ON
sleep 1
[ "$(ret camb/flip_onoff)" = "ON" ] && ok "bool : commande ON -> état retained ON" || bad "bool : commande ON -> état retained ON"
grep -q "^set_flip_onoff 1$" "$WORK/stub_b.log" && ok "bool : setter appelé avec 1" || bad "bool : setter appelé avec 1"

pub camb/alarm_sensitivity/set 2
sleep 1
[ "$(ret camb/alarm_sensitivity)" = "2" ] && ok "entier : commande 2 -> état retained 2" || bad "entier : commande 2 -> état retained 2"

pub camb/format_sd_card/set 1
sleep 1
grep -q "^format_sd_card 1$" "$WORK/stub_b.log" && ok "action : fonction appelée" || bad "action : fonction appelée"
[ -z "$(ret camb/format_sd_card)" ] && ok "action : pas d'état publié" || bad "action : pas d'état publié"

echo "flip_onoff off" > /tmp/md_cmd
sleep 2
[ "$(ret camb/flip_onoff)" = "OFF" ] && ok "/tmp/md_cmd : commande générique appliquée" \
    || bad "/tmp/md_cmd : commande générique appliquée"

pub camb/flip_onoff/set nimp
sleep 1
[ "$(ret camb/flip_onoff)" = "OFF" ] && ok "bool : charge utile invalide ignorée" || bad "bool : charge utile invalide ignorée"

kill "$MAINPID" 2>/dev/null; wait "$MAINPID" 2>/dev/null; MAINPID=""

if [ $FAIL -eq 0 ]; then
    echo "TOUT OK"
else
    echo "ÉCHECS ; journaux dans $WORK (stubs.c, log_a/b.txt, stub_a/b.log)"
    exit 1
fi
