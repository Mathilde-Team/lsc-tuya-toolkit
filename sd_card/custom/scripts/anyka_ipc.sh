#! /bin/sh

SD_DIR="/tmp/sd"
CUSTOM_DIR="${SD_DIR}/custom"
SCRIPTS_DIR="${CUSTOM_DIR}/scripts"
DATETIME="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${SD_DIR}/logs/anyka.${DATETIME}.log"
ENTRYPOINT="${SCRIPTS_DIR}/entrypoint.sh"

MODE="$1"
CFG_FILE="${CUSTOM_DIR}/configs/hack.conf"
TALK=0
TALK_PORT=5004
TALK_ALLOW=""
TALK_GAIN=50
[ -f "${CFG_FILE}" ] && . "${CFG_FILE}"
PATH="$PATH:/bin:/sbin:/usr/bin:/usr/sbin"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*"
}

mkdir -p "${SD_DIR}/logs" >/dev/null 2>&1 || true
touch "${LOG_FILE}"
exec >>"${LOG_FILE}" 2>&1

usage() {
  echo "Usage: $0 start|stop|restart"
  exit 3
}

entrypoint() {
  if [ -x "${ENTRYPOINT}" ]; then
      /bin/sh "${ENTRYPOINT}" >/dev/null 2>&1 &
  else
      log "ERROR entrypoint missing: ${ENTRYPOINT}"
  fi

}

start() {
  mkdir -p "$LOG_DIR"
  if pgrep anyka_ipc >/dev/null 2>&1; then
    echo "anyka_ipc already running"
    exit 0
  fi

  echo "start ipc service..."
  # Haut-parleur par le LAN (tools/talk) : seulement si TALK=1 et TALK_ALLOW renseigné
  if [ "$TALK" = "1" ] && [ -n "$TALK_ALLOW" ] && [ -f "${CUSTOM_DIR}/bin/talk_inject.so" ]; then
    LD_PRELOAD="${CUSTOM_DIR}/bin/talk_inject.so" TALK_PORT="$TALK_PORT" TALK_ALLOW="$TALK_ALLOW" TALK_GAIN="$TALK_GAIN" \
      anyka_ipc >>"$LOG_FILE" 2>&1 &
  else
    anyka_ipc >>"$LOG_FILE" 2>&1 &
  fi
  echo "log: $LOG_FILE"
  entrypoint
}

stop() {
  echo "stopping ipc service..."
  killall anyka_ipc >/dev/null 2>&1 || true
  sleep 1
  killall -9 anyka_ipc >/dev/null 2>&1 || true
}

restart() {
  echo "restart ipc service..."
  stop
  start
}

case "$MODE" in
  start) start ;;
  stop) stop ;;
  restart) restart ;;
  *) usage ;;
esac

exit 0