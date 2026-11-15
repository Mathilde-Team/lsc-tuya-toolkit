#!/bin/sh

SD_DIR="/tmp/sd"
. /tmp/sd/custom/scripts/logdir.sh
CUSTOM_DIR="${SD_DIR}/custom"
SCRIPTS_DIR="${CUSTOM_DIR}/scripts"
CONFIG_DIR="${CUSTOM_DIR}/configs"
STATE_DIR="${CUSTOM_DIR}/state"
CFG_FILE="${CONFIG_DIR}/hack.conf"
DATETIME="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${LOGS}/entrypoint.${DATETIME}.log"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*"
}

mkdir -p "${LOGS}" >/dev/null 2>&1 || true
mkdir -p "${STATE_DIR}" >/dev/null 2>&1 || true

exec >>"${LOG_FILE}" 2>&1

load_cfg() {
    # Defaults
    SAVE_SYSLOG=1
    SD_SYNC=1
    LOG_RAM_MAX_KB=256
    LOG_SNAPSHOT_MIN=60

    if [ -f "${CFG_FILE}" ]; then
        # shellcheck disable=SC1090
        . "${CFG_FILE}"
    fi
}

purge_logs() {
  # Rotation des journaux de demarrage (issue #7) : garde les LOGS_KEEP
  # derniers demarrages de chaque fichier (run, entrypoint, anyka_debug),
  # purge le reste. LOGS_KEEP absent, vide ou 0 = rotation desactivee.
  keep="${LOGS_KEEP:-0}"
  case "${keep}" in ''|*[!0-9]*) return 0 ;; esac
  [ "${keep}" -ge 1 ] || return 0
  for pat in run entrypoint anyka_debug; do
    ls -1t "${LOGS}/${pat}".*.log 2>/dev/null | tail -n "+$((keep + 1))" | while IFS= read -r f; do
      rm -f "${f}"
    done
  done
  log "Journaux de demarrage purges (LOGS_KEEP=${keep})"
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
    tail -n 400 /var/log/messages > "${LOGS}/messages_tail.txt" 2>/dev/null || true
    touch /tmp/.ht_syslog_saved 2>/dev/null || true
    log "Saved /var/log/messages tail to ${LOGS}/messages_tail.txt"
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
# --- Protection de la SD (FAT corrompue par les coupures de courant) ---
sd_protect() {
  # 1. ecritures synchrones : chaque ecriture part tout de suite sur la carte
  if [ "${SD_SYNC}" = "1" ]; then
    mount -o remount,sync "${SD_DIR}" >/dev/null 2>&1 && log "SD montee en sync" || log "SD sync impossible"
  fi
  # 2. etat PTZ en RAM, prereglages recopies depuis la SD
  mkdir -p /tmp/ptzstate >/dev/null 2>&1
  cp "${STATE_DIR}"/ptz_presets.db "${STATE_DIR}"/ptz_home /tmp/ptzstate/ 2>/dev/null || true
}

log_keeper() {
  # Les journaux RAM sont bornes (tmpfs) ; un instantane (fin des journaux) est
  # copie sur la SD toutes les LOG_SNAPSHOT_MIN minutes pour les analyses post-redemarrage.
  [ "${LOGS}" = "${SD_DIR}/logs" ] && return 0
  max=$(( ${LOG_RAM_MAX_KB:-256} * 1024 ))
  snap=0
  while true; do
    sleep 300
    for f in "${LOGS}"/*.log; do
      [ -f "${f}" ] || continue
      sz=$(wc -c < "${f}" 2>/dev/null || echo 0)
      if [ "${sz}" -gt "${max}" ]; then
        tail -n 300 "${f}" > "${f}.cut" 2>/dev/null && cat "${f}.cut" > "${f}"; rm -f "${f}.cut"
      fi
    done
    snap=$(( snap + 5 ))
    if [ "${LOG_SNAPSHOT_MIN:-60}" -gt 0 ] && [ "${snap}" -ge "${LOG_SNAPSHOT_MIN:-60}" ]; then
      snap=0
      mkdir -p "${SD_DIR}/logs" 2>/dev/null
      { for f in "${LOGS}"/anyka.*.log "${LOGS}"/entrypoint.*.log; do [ -f "${f}" ] && { echo "== ${f}"; tail -n 80 "${f}"; }; done; } > "${SD_DIR}/logs/ram_snapshot.txt" 2>/dev/null && sync
    fi
  done
}

log "entrypoint starting"

load_cfg
sd_protect
purge_logs
log_keeper &
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
  # anyka_ipc decale l'horloge de +8 h (fuseau Chine) ~20 s apres son demarrage :
  # l'ecart (heure - uptime) est constant, tout saut est annule.
  ref=$(( $(date +%s) - $(cut -d. -f1 /proc/uptime) ))
  while true; do
    [ -z "${TZ_STRING}" ] || [ "$(cat /tmp/TZ 2>/dev/null)" = "${TZ_STRING}" ] || echo "${TZ_STRING}" > /tmp/TZ
    now=$(date +%s)
    jump=$(( now - $(cut -d. -f1 /proc/uptime) - ref ))
    if [ "${jump}" -gt 30 ] || [ "${jump}" -lt -30 ]; then
      date -s "@$(( now - jump ))" >/dev/null 2>&1
      log "saut d'horloge de ${jump}s corrige"
    fi
    sleep 5
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

if [ "${PTZ_HOME_ON_BOOT:-0}" = "1" ]; then
  # recalage sur les butees : rend les positions absolues ONVIF fiables
  /bin/sh "${SCRIPTS_DIR}/ptz_move" -h >/dev/null 2>&1 &
  log "PTZ recale (home)"
fi

log "entrypoint done"
