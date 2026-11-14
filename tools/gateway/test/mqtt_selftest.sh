#!/bin/sh
# Bout en bout sur PC : md_inject.c + gateway.c + firmware simulé contre un broker mosquitto local.
# Vérifie l'abonnement <base>/+/set, les alias night/privacy, /tmp/md_cmd et une charge > 127 octets.
# Attention : utilise /tmp/md_cmd du PC.
set -e
cd "$(dirname "$0")"
command -v mosquitto >/dev/null || { echo "mosquitto absent : test MQTT ignoré"; exit 0; }
T=$(mktemp -d)
PORT=18830
cleanup() { kill $BRK $APP $SUB 2>/dev/null || true; rm -rf "$T"; }
trap cleanup EXIT
printf 'listener %s 127.0.0.1\nallow_anonymous true\n' $PORT > "$T/m.conf"
mosquitto -c "$T/m.conf" >"$T/broker.log" 2>&1 & BRK=$!
sleep 1
printf '127.0.0.1\n%s\n\n\n' $PORT > "$T/cred"
cc -DSTARTUP_DELAY_S=0 -I.. -o "$T/app" ../../motion/md_inject.c ../gateway.c fake_fw.c -lpthread
MOTION_MQTT_FILE="$T/cred" MOTION_TOPIC=camtest/motion "$T/app" >"$T/app.log" 2>&1 & APP=$!
mosquitto_sub -p $PORT -t 'camtest/#' -v >"$T/sub.log" & SUB=$!
sleep 7
mosquitto_pub -p $PORT -t camtest/night/set -m day
mosquitto_pub -p $PORT -t camtest/privacy/set -m ON
mosquitto_pub -p $PORT -t camtest/flip_onoff/set -m ON
echo "night auto" > /tmp/md_cmd
sleep 3
LONG=$(python3 -c 'print("x"*200)')
mosquitto_pub -p $PORT -t camtest/long/set -m "$LONG"
mosquitto_pub -p $PORT -t camtest/night/set -m night
sleep 2
cat "$T/app.log"; echo ---; cat "$T/sub.log"
ok=0
grep -q "FAKE irled 1" "$T/app.log" || { echo "ÉCHEC night day"; ok=1; }
grep -q "FAKE system /tmp/sd/custom/scripts/privacy.sh on" "$T/app.log" || { echo "ÉCHEC privacy"; ok=1; }
grep -q "FAKE irled 0" "$T/app.log" || { echo "ÉCHEC /tmp/md_cmd"; ok=1; }
grep -q "^camtest/night day" "$T/sub.log" || { echo "ÉCHEC état night"; ok=1; }
grep -q "^camtest/privacy ON" "$T/sub.log" || { echo "ÉCHEC état privacy"; ok=1; }
grep -q "unknown or unexposed flip_onoff" "$T/app.log" || { echo "ÉCHEC DP non exposé"; ok=1; }
grep -q "unknown or unexposed long" "$T/app.log" && { echo "ÉCHEC : charge > tampon val acceptée"; ok=1; }
grep -q "FAKE irled 2" "$T/app.log" || { echo "ÉCHEC : flux désynchronisé après une trame > 127 octets"; ok=1; }
[ $ok = 0 ] && echo "MQTT OK"
exit $ok
