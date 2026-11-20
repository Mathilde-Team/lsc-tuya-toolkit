#!/bin/sh

SD_DIR="/tmp/sd"
CUSTOM_DIR="${SD_DIR}/custom"
SCRIPTS_DIR="${CUSTOM_DIR}/scripts"
CONFIG_DIR="${CUSTOM_DIR}/configs"
STATE_DIR="${CUSTOM_DIR}/state"
CFG_FILE="${CONFIG_DIR}/hack.conf"

DATETIME="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="${SD_DIR}/logs/run.${DATETIME}.log"

log() {
    echo "[$(date +%Y-%m-%dT%H:%M:%S)] $*"
}

load_cfg() {
    # Defaults
    OVERRIDE_HW_SETTINGS=1
    OVERRIDE_SW_SETTINGS=1
    AUTO_PATCH_FW=1

    if [ -f "${CFG_FILE}" ]; then
        # shellcheck disable=SC1090
        . "${CFG_FILE}"
    fi
}

kill_processes() {
  kill -9 "$(ps | grep -F 'drop_caches' | grep -v grep | awk '{print $1}')" || true
  rm /tmp/drop_caches.sh || true
  kill -TERM "$(pidof anyka_ipc)" || true
}

ensure_shadow() {
  src="${SD_DIR}/shadow"
  dst="/etc/config/shadow"
  done="${STATE_DIR}/shadow.done"

  [ -f "$done" ] && [ -f "$dst" ] && return 0

  log "Copying shadow"
  touch "${dst}"
  mount --bind "${src}" "${dst}" || true

  : > "$done"
  log "Shadow copied."
}

# Profil materiel : le fichier de la SD vient d'une camera PTZ testee (firmware 3.10.78.1269,
# capteur 17). Sur un autre modele (autre firmware ou capteur : IR-cut, capteur, niveaux
# jour/nuit differents), on garde le fichier d'origine de la camera et on n'y reporte que
# les resolutions de celui de la SD (le codec reste celui d'origine).
# /tmp/hw_ptz : 1 si la camera a un PTZ (ptz_support = 1 ou pan/tilt_total_steps dans le fichier d'origine), sinon 0.
hw_settings_file() {
  sdf="${CONFIG_DIR}/_ht_hw_settings.ini"
  orig="/etc/config/_ht_hw_settings.ini"
  if [ ! -f "${orig}" ]; then
    echo 1 > /tmp/hw_ptz
    echo "${sdf}"
    return 0
  fi
  if grep -q -E '^ptz_support[[:space:]]*=[[:space:]]*1|^(pan|tilt)_total_steps[[:space:]]*=[[:space:]]*[1-9]' "${orig}"; then echo 1 > /tmp/hw_ptz; else echo 0 > /tmp/hw_ptz; fi
  ver="$(cat /usr/share/VERSION 2>/dev/null)"
  s_orig="$(grep -E '^sensor_type_def[[:space:]]*=' "${orig}" | head -n 1 | sed 's/^[^=]*=[[:space:]]*//;s/[[:space:]]*$//')"
  s_sd="$(grep -E '^sensor_type_def[[:space:]]*=' "${sdf}" | head -n 1 | sed 's/^[^=]*=[[:space:]]*//;s/[[:space:]]*$//')"
  if [ "${ver}" = "3.10.78.1269" ] && [ "${s_orig}" = "${s_sd}" ]; then
    echo "${sdf}"
    return 0
  fi
  eff=/tmp/hw_effective.ini
  cp "${orig}" "${eff}"
  for k in main_width main_height sub_width sub_height; do
    v="$(grep -E "^${k}[[:space:]]*=" "${sdf}" | head -n 1 | sed 's/^[^=]*=[[:space:]]*//;s/[[:space:]]*$//')"
    [ -n "${v}" ] && sed -i "s/^${k}[[:space:]]*=.*/${k} = ${v}/" "${eff}"
  done
  log "autre modele (firmware ${ver:-?}, capteur ${s_orig:-?}) : reglages materiels d'origine conserves, resolutions reportees" >&2
  echo "${eff}"
}

apply_overrides() {
  mount --bind "${SCRIPTS_DIR}/recover_cfg.sh" /usr/sbin/recover_cfg.sh || true
  mount --bind "${SCRIPTS_DIR}/anyka_ipc.sh" /usr/sbin/anyka_ipc.sh || true
  # LED: script qui garde la LED éteinte
  [ -f "${SCRIPTS_DIR}/state_led.sh" ] && mount --bind "${SCRIPTS_DIR}/state_led.sh" /usr/sbin/state_led.sh && /bin/sh /usr/sbin/state_led.sh off
  # Vidéo 20 i/s : fabriqué sur la caméra à partir de SES fichiers si le firmware est celui testé (3.10.78), sinon rien
  [ "${AUTO_PATCH_FW}" = "1" ] && /bin/sh "${SCRIPTS_DIR}/patch_fw.sh" || true
  # Vidéo: ISP à 20 fps (le capteur est limité à 14 fps par le fichier d'origine)
  [ -f "${CONFIG_DIR}/isp_sc2336_fps20.conf" ] && mount --bind "${CONFIG_DIR}/isp_sc2336_fps20.conf" /etc/config/isp_sc2336_mipi_2lane_h3b.conf || true
  # Vidéo: anyka_ipc patché (frame_rate du canal VI : 15 -> 20)
  [ -f "${CUSTOM_DIR}/bin/anyka_ipc_fps20b" ] && mount --bind "${CUSTOM_DIR}/bin/anyka_ipc_fps20b" /usr/bin/anyka_ipc || true
  hw_file="$(hw_settings_file)"
  if [ "${OVERRIDE_HW_SETTINGS}" = "1" ]; then
    mount --bind "${hw_file}" /usr/local/_ht_hw_settings.ini || true
  fi
  if [ "${OVERRIDE_SW_SETTINGS}" = "1" ]; then
    mount --bind "${CONFIG_DIR}/_ht_sw_settings.ini" /etc/config/_ht_sw_settings.ini || true
  fi
  # mount --bind "${CONFIG_DIR}/factory_cfg.ini" /usr/local/factory_cfg.ini || true
  onvif_conf="${CONFIG_DIR}/onvif_simple_server.conf"
  if [ "$(cat /tmp/hw_ptz 2>/dev/null)" = "0" ]; then
    # camera fixe : ONVIF n'annonce pas de PTZ
    sed 's/^ptz=1/ptz=0/' "${onvif_conf}" > /tmp/onvif_effective.conf && onvif_conf=/tmp/onvif_effective.conf
  fi
  mount --bind "${onvif_conf}" /usr/local/etc/onvif_simple_server.conf || true
  touch "${SD_DIR}/logs/anyka_debug.${DATETIME}.log"
  touch /etc/config/log_debug
  mount --bind "${SD_DIR}/logs/anyka_debug.${DATETIME}.log" /etc/config/log_debug || true

  ensure_shadow
  sync
}


show_info() {
  log "===================="
  log "------------ process"
  ps aux
  log "------------ /tmp"
  ls -a /tmp
  log "------------ netstat"
  netstat -tulpn
  log "===================="
}

main() {
  log "Checkin stuff pre-hack"
  show_info
  if [ ! -x "${SCRIPTS_DIR}/dump_fs.sh" ]; then
      log "ERROR dump_fs.sh missing or not executable: ${SCRIPTS_DIR}/dump_fs.sh"
      return 1
  fi

  /bin/sh "${SCRIPTS_DIR}/dump_fs.sh" >/dev/null 2>&1

  apply_overrides || true
  kill_processes || true

  log "Checkin stuff post-hack"
  show_info
}

mkdir -p "${SD_DIR}"
mount /dev/mmcblk0p1 "${SD_DIR}" || mount /dev/mmcblk0 "${SD_DIR}" || true

mkdir -p "${SD_DIR}/logs"
load_cfg
exec >>"${LOG_FILE}" 2>&1

/bin/sh "${SCRIPTS_DIR}/sounds.sh" >/dev/null 2>&1

log "hack.sh starting"
log "DATE IS: $(date)"

main
log "hack.sh done"