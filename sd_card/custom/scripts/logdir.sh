#!/bin/sh
# Source commune (". logdir.sh") : choisit le dossier des journaux et de l'etat.
# Une SD FAT s'abime si le courant saute pendant une ecriture : par defaut tout ce qui
# s'ecrit souvent (journaux, position PTZ) reste en RAM. LOG_TO_SD=1 dans hack.conf
# remet les journaux sur la SD. Exporte LOGS (journaux) et RAM_STATE (position PTZ).

_cfg="/tmp/sd/custom/configs/hack.conf"
_v="$(sed -n 's/^LOG_TO_SD=//p' "${_cfg}" 2>/dev/null | tail -n 1 | tr -d '"' | tr -d "'")"
if [ "${_v}" = "1" ]; then
    LOGS="/tmp/sd/logs"
else
    LOGS="/tmp/hacklogs"
fi
RAM_STATE="/tmp/ptzstate"
mkdir -p "${LOGS}" "${RAM_STATE}" >/dev/null 2>&1 || true
unset _cfg _v
