#!/bin/sh
# Relais ONVIF 0 (mode prive) : "privacy_relay.sh on|off".
# Passe par md_inject.so (/tmp/md_cmd) pour que <base>/privacy et la persistance suivent ;
# sans MQTT personne ne lit /tmp/md_cmd : on appelle alors privacy.sh directement.
case "$1" in
    on|off) ;;
    *) exit 1 ;;
esac
echo "privacy $1" > /tmp/md_cmd
i=0
while [ -e /tmp/md_cmd ] && [ "${i}" -lt 6 ]; do
    sleep 1
    i=$((i + 1))
done
if [ -e /tmp/md_cmd ]; then
    rm -f /tmp/md_cmd
    exec /tmp/sd/custom/scripts/privacy.sh "$1"
fi
exit 0
