#!/bin/sh
# Reglages memorises apres un reboot.
#   persist.sh save <nom> <valeur>   appele par la passerelle MQTT apres chaque reglage reussi
#   persist.sh restore               appele par entrypoint.sh : rejoue les reglages via /tmp/md_cmd
#   persist.sh clear                 oublie tout
# Fichier : custom/state/settings.conf, une ligne "<nom> <valeur>". Ecrit seulement si la
# valeur change (usure de la SD) ; jamais les DP 111/112/150/151 (stockage de la SD systeme).

DIR="/tmp/sd/custom/state"
FILE="${DIR}/settings.conf"

case "$1" in
save)
    name="$2"
    val="$3"
    case "${name}" in
        ""|*[!a-z0-9_]*) exit 1 ;;
        sd_*|*_sd_card|format_*|unmount_*|sleep_mode) exit 0 ;;
    esac
    case "${val}" in
        ""|*[!A-Za-z0-9_.,-]*) exit 1 ;;
    esac
    mkdir -p "${DIR}" 2>/dev/null || exit 1
    if grep -q "^${name} ${val}\$" "${FILE}" 2>/dev/null; then
        exit 0
    fi
    {
        grep -v "^${name} " "${FILE}" 2>/dev/null
        echo "${name} ${val}"
    } > "${FILE}.tmp" && mv "${FILE}.tmp" "${FILE}"
    ;;
restore)
    [ -f "${FILE}" ] || exit 0
    # attend que md_inject.so lise /tmp/md_cmd (il le consomme en ~1 s)
    sleep 20
    while read -r name val; do
        [ -n "${name}" ] && [ -n "${val}" ] || continue
        # le mode prive est rejoue en dernier, et seulement s'il etait actif
        [ "${name}" = "privacy" ] && continue
        i=0
        while [ -e /tmp/md_cmd ] && [ "${i}" -lt 10 ]; do sleep 1; i=$((i + 1)); done
        echo "${name} ${val}" > /tmp/md_cmd
    done < "${FILE}"
    if grep -q "^privacy ON\$" "${FILE}"; then
        i=0
        while [ -e /tmp/md_cmd ] && [ "${i}" -lt 10 ]; do sleep 1; i=$((i + 1)); done
        echo "privacy on" > /tmp/md_cmd
    fi
    ;;
clear)
    rm -f "${FILE}"
    ;;
*)
    echo "usage: persist.sh save <nom> <valeur> | restore | clear" >&2
    exit 1
    ;;
esac
