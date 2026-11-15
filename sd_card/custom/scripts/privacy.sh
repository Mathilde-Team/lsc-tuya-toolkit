#!/bin/sh

# Position privée (issue #13) : envoie la caméra sur le preset PRIVACY_PRESET.
# Le preset « privé » (tête tournée vers le mur/le bas) se crée une fois avec
# ptz_presets.sh (add_preset) ou via ONVIF (SetPreset), cf. docs/PRIVACY.md.
# ATTENTION : une position PTZ ne coupe ni le flux RTSP ni le micro.

SD_DIR="/tmp/sd"
CUSTOM_DIR="${SD_DIR}/custom"
SCRIPTS_DIR="${CUSTOM_DIR}/scripts"
CFG_FILE="${CUSTOM_DIR}/configs/hack.conf"
DATETIME="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${SD_DIR}/logs/privacy.${DATETIME}.log"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*" >> "${LOG_FILE}"
}

mkdir -p "${SD_DIR}/logs" >/dev/null 2>&1 || true
# SD en lecture seule (FAT corrompue) : journal dans /tmp pour que les moteurs restent commandables
( : >> "${LOG_FILE}" ) 2>/dev/null || LOG_FILE="/tmp/privacy.${DATETIME}.log"
exec >> "${LOG_FILE}" 2>&1

load_cfg() {
    # Defaults
    PRIVACY_PRESET=0
    PRIVACY_HOME_PRESET=0

    if [ -f "${CFG_FILE}" ]; then
        # shellcheck disable=SC1090
        . "${CFG_FILE}"
    fi
}

load_cfg

# Usage : privacy.sh [on|off]  (on par défaut). off = retour au preset PRIVACY_HOME_PRESET.
ACTION="${1:-on}"
if [ "${ACTION}" = "off" ]; then
    TARGET="${PRIVACY_HOME_PRESET}"
else
    TARGET="${PRIVACY_PRESET}"
fi

POS_FILE="/tmp/privacy_prev_pos"

# Sans preset : pose privee PRIVACY_POS (x,y de -1 a 1, defaut 0,-1 = tete vers le bas),
# la position courante est memorisee puis restauree par "off".
if [ "${TARGET}" = "0" ] || [ -z "${TARGET}" ]; then
    PRIVACY_POS="${PRIVACY_POS:-0,-1}"
    if [ "${ACTION}" = "off" ]; then
        [ -f "${POS_FILE}" ] || { log "off : aucune position memorisee"; exit 0; }
        DEST="$(cat "${POS_FILE}")"
        rm -f "${POS_FILE}"
    else
        [ -f "${POS_FILE}" ] && { log "on : deja en position privee"; exit 0; }
        "${SCRIPTS_DIR}/get_position" | awk -F, '{printf "%.3f,%.3f\n", ($1-180)/180, ($2-90)/90}' > "${POS_FILE}"
        DEST="${PRIVACY_POS}"
    fi
    case "${DEST}" in
        *[!0-9.,-]*|'') log "position invalide : ${DEST}"; exit 1 ;;
    esac
    log "privacy.sh ${ACTION} -> position ${DEST}"
    "${SCRIPTS_DIR}/ptz_move" -j "${DEST},0"
    log "privacy.sh fait"
    exit 0
fi

case "${TARGET}" in
    *[!0-9]*)
        log "preset ${TARGET} invalide"
        exit 0
        ;;
esac

log "privacy.sh ${ACTION} -> preset ${TARGET}"
if [ ! -x "${SCRIPTS_DIR}/ptz_move" ]; then
    log "ERROR ptz_move manquant : ${SCRIPTS_DIR}/ptz_move"
    exit 1
fi
"${SCRIPTS_DIR}/ptz_move" -p "${TARGET}"
log "privacy.sh fait"
