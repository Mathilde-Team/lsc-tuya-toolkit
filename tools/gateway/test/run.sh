#!/bin/sh
# Tests PC de la passerelle (issue #53) : logique de gateway.c avec une table générée depuis une fixture.
# Optionnel : test MQTT de bout en bout (mosquitto requis) avec MQTT=1.
set -e
cd "$(dirname "$0")"
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
python3 ../gen_gateway.py ../../../docs/firmware/DP_TABLE.md "$T/dp_table.h" "$T/TOPICS.md" DP_VALUES.fixture.md
cc -Wall -Wextra -Werror -Wno-unused-parameter -Wno-cast-function-type -I.. -DGW_TABLE="\"$T/dp_table.h\"" -o "$T/t" test_gateway.c ../gateway.c
"$T/t"
[ "$MQTT" = "1" ] && sh ./mqtt_selftest.sh
exit 0
