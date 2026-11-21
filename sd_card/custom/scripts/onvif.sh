#!/bin/sh

SD_DIR="/tmp/sd"
. /tmp/sd/custom/scripts/logdir.sh
CUSTOM_DIR="${SD_DIR}/custom"
DATETIME="$(date +%Y%m%d_%H%M%S)"
CFG_FILE="${CUSTOM_DIR}/configs/hack.conf"
LOG_FILE="${LOGS}/onvif.${DATETIME}.log"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*"
}

mkdir -p "${LOGS}" >/dev/null 2>&1 || true
exec >>"${LOG_FILE}" 2>&1

load_cfg() {
    # Defaults
    ONVIF=1
    ONVIF_PTZ=1
    ONVIF_PTZCTL="${CUSTOM_DIR}/bin/ptzctl"

    if [ -f "${CFG_FILE}" ]; then
        # shellcheck disable=SC1090
        . "${CFG_FILE}"
    fi
}

mount_onvif_ptz_helpers() {
    if [ "${ONVIF_PTZ}" != "1" ]; then
        return 0
    fi

    for helper in ptz_move get_position is_moving ptz_presets.sh; do
        src="${CUSTOM_DIR}/scripts/${helper}"
        if [ ! -x "${src}" ]; then
            log "WARN ONVIF PTZ helper missing: ${src}"
            continue
        fi
        chmod +x "${src}" >/dev/null 2>&1 || true
    done

    if [ ! -x "${ONVIF_PTZCTL}" ]; then
        log "WARN ONVIF PTZ controller missing: ${ONVIF_PTZCTL}"
    fi
}

ensure_onvif() {
    mount_onvif_ptz_helpers

    # Numéro de série ONVIF unique (adresse MAC) : Home Assistant s'en sert comme identifiant, sinon il refuse la 2e caméra
    cfg="${CUSTOM_DIR}/configs/onvif_simple_server.conf"
    sn="$(sed 's/://g' /sys/class/net/wlan0/address 2>/dev/null)"
    if [ -n "${sn}" ] && [ -f "${cfg}" ] && ! grep -q "^serial_num=${sn}\$" "${cfg}"; then
        sed -i "s/^serial_num=.*/serial_num=${sn}/" "${cfg}"
    fi

    MINI_HTTPD="${CUSTOM_DIR}/bin/mini_cgi_httpd"
    ONVIF_WWW="${CUSTOM_DIR}/onvif/www/onvif"

    if ps | grep -v grep | grep -q "mini_cgi_httpd"; then
        return 0
    fi

    # Pile ONVIF legere (voir docs/ONVIF.md) : mini serveur CGI + onvif_simple_server
    if [ -x "${MINI_HTTPD}" ] && [ -x "${ONVIF_WWW}/onvif_simple_server" ]; then
        "${MINI_HTTPD}" 8080 "${ONVIF_WWW}" >/dev/null 2>&1 &
        log "ONVIF started (mini_cgi_httpd on :8080)"
        # Découverte automatique (WS-Discovery) pour Home Assistant
        if [ -x "${CUSTOM_DIR}/bin/wsd" ] && ! ps | grep -v grep | grep -q "bin/wsd"; then
            ptz_flag=1
            [ "$(cat /tmp/hw_ptz 2>/dev/null)" = "0" ] && ptz_flag=0
            "${CUSTOM_DIR}/bin/wsd" wlan0 8080 "$(hostname)" "${ONVIF_MODEL:-IPC}" "${ptz_flag}" >/dev/null 2>&1 &
        fi
        return 0
    fi

    # Pile d'origine (lighttpd), absente de certains firmwares
    if ps | grep -v grep | grep -q "lighttpd -f /usr/local/etc/lighttpd.conf"; then
        return 0
    fi
    if [ -x /usr/local/bin/mini_onvif_service.sh ]; then
        /usr/local/bin/mini_onvif_service.sh start || true
        log "ONVIF started (mini_onvif_service.sh)"
    else
        log "ONVIF requested but nothing to start (no mini_cgi_httpd, no mini_onvif_service.sh)"
    fi
}

log_onvif_status() {
    if [ "${ONVIF}" != "1" ]; then
        return 0
    fi
    # Once per boot.
    if [ -e /tmp/.ht_onvif_status_logged ]; then
        return 0
    fi

    {
        echo "[${SD_DIR}] ONVIF status $(date +%Y-%m-%dT%H:%M:%S)"
        echo ""
        echo "## ps"
        ps w | grep -v grep | grep -E "mini_cgi_httpd|lighttpd|wsd_simple_server|onvif_notify_server|events_service" || true
        echo ""
        echo "## netstat"
        netstat -lntp 2>/dev/null | grep -E ":80\b|:8080\b|:3702\b|:8899\b" || true
        netstat -lnt 2>/dev/null | grep -E ":80\b|:8080\b|:3702\b|:8899\b" || true
        echo ""
        echo "## http probe"
        if command -v wget >/dev/null 2>&1; then
            wget -qO- http://127.0.0.1:8080/ 2>/dev/null | head -c 200 || true
            echo ""
            wget -qO- http://127.0.0.1:8080/onvif/device_service 2>/dev/null | head -c 200 || true
            echo ""
        fi
    } > "${LOGS}/onvif_status.txt" 2>&1

    touch /tmp/.ht_onvif_status_logged 2>/dev/null || true
    log "Wrote ${LOGS}/onvif_status.txt"
}

load_cfg
if [ "${ONVIF}" = "1" ]; then
  ensure_onvif
  log_onvif_status
fi