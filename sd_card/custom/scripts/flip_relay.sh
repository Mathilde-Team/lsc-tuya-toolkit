#!/bin/sh
# Relais ONVIF 1 (miroir) et 2 (retournement vertical) : "flip_relay.sh mirror|flip_v on|off".
# Passe par md_inject.so (/tmp/md_cmd) pour que <base>/mirror, <base>/flip_v et la persistance suivent.
case "$1" in
    mirror|flip_v) ;;
    *) exit 1 ;;
esac
case "$2" in
    on) v=ON ;;
    off) v=OFF ;;
    *) exit 1 ;;
esac
echo "$1 ${v}" > /tmp/md_cmd
i=0
while [ -e /tmp/md_cmd ] && [ "${i}" -lt 6 ]; do
    sleep 1
    i=$((i + 1))
done
[ -e /tmp/md_cmd ] && rm -f /tmp/md_cmd
exit 0
