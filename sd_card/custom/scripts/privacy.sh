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

case "${TARGET}" in
    ''|*[!0-9]*|0)
        log "preset ${ACTION} absent ou nul : rien a faire"
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
