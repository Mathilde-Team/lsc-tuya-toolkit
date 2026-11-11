#!/bin/sh

SD_DIR="/tmp/sd"
CUSTOM_DIR="${SD_DIR}/custom"
SCRIPTS_DIR="${CUSTOM_DIR}/scripts"
CONFIG_DIR="${CUSTOM_DIR}/configs"
STATE_DIR="${CUSTOM_DIR}/state"
CFG_FILE="${CONFIG_DIR}/hack.conf"
DATETIME="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${SD_DIR}/logs/entrypoint.${DATETIME}.log"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*"
}

mkdir -p "${SD_DIR}/logs" >/dev/null 2>&1 || true
mkdir -p "${STATE_DIR}" >/dev/null 2>&1 || true

exec >>"${LOG_FILE}" 2>&1

load_cfg() {
    # Defaults
    SAVE_SYSLOG=1

    if [ -f "${CFG_FILE}" ]; then
        # shellcheck disable=SC1090
        . "${CFG_FILE}"
    fi
}

save_syslog_tail() {
    if [ "${SAVE_SYSLOG}" != "1" ]; then
        return 0
    fi
    if [ ! -f /var/log/messages ]; then
        return 0
    fi
    # Once per boot.
    if [ -e /tmp/.ht_syslog_saved ]; then
        return 0
    fi
    tail -n 400 /var/log/messages > "${SD_DIR}/logs/messages_tail.txt" 2>/dev/null || true
    touch /tmp/.ht_syslog_saved 2>/dev/null || true
    log "Saved /var/log/messages tail to ${SD_DIR}/logs/messages_tail.txt"
}

set_cpus() {
  ulimit -c unlimited
  echo "1" > /proc/sys/kernel/core_uses_pid
  echo "/mnt/ht_log/core_%e_%p_%t" > /proc/sys/kernel/core_pattern
  rm /tmp/camera_start_times
}

wait_rtsp_listening() {
  PORTS="554 88 89"
  MAX="${1:-30}"   # seconds
  i=0
  log "Wait for RTSP"

  while [ "$i" -lt "$MAX" ]; do
    for p in $PORTS; do
      # Prefer netstat if available
      if netstat -ltnp 2>/dev/null | grep -q "anyka_ipc" | grep -q ":$p "; then
        log "RTSP running"
        return 0
      fi
      # BusyBox netstat often lacks -p; fallback without process match
      if netstat -ltn 2>/dev/null | grep -q ":$p "; then
        log "RTSP running"
        return 0
      fi
    done
    sleep 1
    i=$((i+1))
  done
  log "Timeout waiting for RTSP"
}

wait_anyka() {
  # Wait untill anyka_ipc is running
  while ! pgrep anyka_ipc >/dev/null 2>&1; do
    sleep 1
  done
  log "anyka_ipc running"
  sleep 30
}
log "entrypoint starting"

load_cfg
save_syslog_tail

set_time() {
  # Fuseau Paris + heure via un serveur NTP du LAN (aucun acces Internet)
  [ -n "${TZ_STRING}" ] && echo "${TZ_STRING}" > /tmp/TZ
  [ -n "${NTP_SERVER}" ] || return 0
  i=0
  while [ "$i" -lt 5 ]; do
    ntpd -n -q -p "${NTP_SERVER}" >/dev/null 2>&1 && { log "NTP ok: $(date)"; return 0; }
    sleep 5
    i=$((i+1))
  done
  log "NTP failed (${NTP_SERVER})"
}

set_time
remote_syslog() {
  [ -n "${CAM_HOSTNAME}" ] && hostname "${CAM_HOSTNAME}"
  [ -n "${SYSLOG_SERVER}" ] || return 0
  pid="$(ps | grep '[s]yslogd -D -n -O' | awk '{print $1}' | head -n 1)"
  [ -n "${pid}" ] && kill "${pid}" 2>/dev/null
  sleep 1
  # -L : garde aussi le journal local en RAM ; -R : envoi UDP vers le serveur
  syslogd -D -n -O /var/log/messages -s 200 -b 3 -L -R "${SYSLOG_SERVER}" &
  log "syslog remote -> ${SYSLOG_SERVER} (host $(hostname))"
}
remote_syslog
# anyka_ipc remet son propre fuseau apres le demarrage : on le reapplique.
keep_tz() {
  [ -n "${TZ_STRING}" ] || return 0
  while true; do
    [ "$(cat /tmp/TZ 2>/dev/null)" = "${TZ_STRING}" ] || echo "${TZ_STRING}" > /tmp/TZ
    sleep 20
  done
}
keep_tz &
log "DATE IS: $(date)"
/bin/sh "${SCRIPTS_DIR}/telnet.sh" >/dev/null 2>&1 &
/bin/sh "${SCRIPTS_DIR}/ftp.sh" >/dev/null 2>&1 &
/bin/sh "${SCRIPTS_DIR}/offline.sh" >/dev/null 2>&1 &
/bin/sh "${SCRIPTS_DIR}/wifi_apply.sh" >/dev/null 2>&1 &
/bin/sh "${SCRIPTS_DIR}/onvif.sh" >/dev/null 2>&1 &

# Wait until any RTSP port is open
wait_rtsp_listening 30

log "entrypoint done"
