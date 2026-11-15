#!/bin/sh
# Tests PC de la passerelle : gateway.c (#53) et découverte HA ha.c (#59), avec une table générée depuis une fixture.
# Optionnel : test MQTT de bout en bout (mosquitto requis) avec MQTT=1.
set -e
cd "$(dirname "$0")"
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
python3 ../gen_gateway.py ../../../docs/firmware/DP_TABLE.md "$T/dp_table.h" "$T/TOPICS.md" DP_VALUES.fixture.md
cc -Wall -Wextra -Werror -Wno-unused-parameter -Wno-cast-function-type -I.. -DGW_TABLE="\"$T/dp_table.h\"" -DGW_AUDIO_DIR="\"/tmp/\"" -DGW_MOUNTS="\"/tmp/gw_mounts_test\"" -o "$T/t" test_gateway.c ../gateway.c -lpthread
"$T/t"
# Découverte HA (#59) : en-tête généré depuis la table de la fixture, sortie C == sortie Python, JSON valide
python3 ../gen_ha_discovery.py "$T/dp_table.h" "$T/ha.h"
cc -Wall -Wextra -Werror -I.. -DGW_HA_TABLE="\"$T/ha.h\"" -o "$T/h" test_ha.c ../ha.c
"$T/h" cam/salon cam/salon/motion > "$T/c.txt"
python3 ../gen_ha_discovery.py --print --base cam/salon --motion cam/salon/motion "$T/dp_table.h" > "$T/py.txt"
cmp "$T/c.txt" "$T/py.txt"
python3 check_ha.py "$T/c.txt" cam/salon
if [ "$MQTT" = "1" ]; then sh ./mqtt_selftest.sh; fi
