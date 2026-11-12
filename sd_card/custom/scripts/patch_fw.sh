#!/bin/sh
# Patch 20 images/s appliqué SUR la caméra, à partir de ses propres fichiers (rien du fabricant n'est redistribué).
# Appliqué seulement si les MD5 correspondent au firmware testé (3.10.78.1269) ; sinon la caméra reste inchangée.
# Équivalent des outils tools/patch_anyka_ipc.py et tools/patch_isp_conf.py.
# Usage : patch_fw.sh [dossier_sortie_bin] [dossier_sortie_configs]

SD_DIR="/tmp/sd"
BIN_DIR="${1:-${SD_DIR}/custom/bin}"
CFG_DIR="${2:-${SD_DIR}/custom/configs}"

IPC_SRC="${PATCH_IPC_SRC:-/usr/bin/anyka_ipc}"
ISP_SRC="${PATCH_ISP_SRC:-/usr/local/isp_sc2336_mipi_2lane_h3b.conf}"
IPC_MD5_ORIG="3d8cad8080cae1f8bd6bca7331e2c651"
IPC_MD5_PATCHED="27ee718fccc520b9989dddeb9ffa124a"
ISP_MD5_ORIG="e5cd9fc1f2e88d2f99aa4041cd3bb646"
ISP_MD5_PATCHED="754d46cf9a3abbfca980c367d1268145"

log() { echo "[patch_fw] $*"; }
md5() { md5sum "$1" 2>/dev/null | cut -d' ' -f1; }

# poke <fichier> <offset décimal> <octets en octal : \NNN\NNN...>
poke() { printf "$3" | dd of="$1" bs=1 seek="$2" conv=notrunc 2>/dev/null; }

patch_ipc() {
  out="${BIN_DIR}/anyka_ipc_fps20b"
  [ "$(md5 "$out")" = "$IPC_MD5_PATCHED" ] && return 0
  rm -f "$out"
  [ "$(md5 "$IPC_SRC")" = "$IPC_MD5_ORIG" ] || { log "anyka_ipc : firmware différent, pas de patch"; return 1; }
  mkdir -p "$BIN_DIR"
  cp "$IPC_SRC" "$out.tmp" || return 1
  poke "$out.tmp" 677088 '\024'            # 0xa54e0 : mov r2,#15 -> #20 (canal principal)
  poke "$out.tmp" 677312 '\024'            # 0xa55c0 : idem (sous-flux)
  poke "$out.tmp" 764404 '\000\000\240\341' # 0xba9f4 : bgt (jeter l'image) -> nop
  if [ "$(md5 "$out.tmp")" = "$IPC_MD5_PATCHED" ]; then
    mv -f "$out.tmp" "$out"; chmod 755 "$out"; log "anyka_ipc patché (20 i/s)"
  else
    rm -f "$out.tmp"; log "anyka_ipc : résultat inattendu, patch annulé"; return 1
  fi
}

patch_isp() {
  out="${CFG_DIR}/isp_sc2336_fps20.conf"
  [ "$(md5 "$out")" = "$ISP_MD5_PATCHED" ] && return 0
  rm -f "$out"
  [ "$(md5 "$ISP_SRC")" = "$ISP_MD5_ORIG" ] || { log "ISP : fichier différent, pas de patch"; return 1; }
  mkdir -p "$CFG_DIR"
  cp "$ISP_SRC" "$out.tmp" || return 1
  # deux copies (0x8cb4 et 0x1b75c) : fps 14->20 à -4 et +8, max_exp_time 2385->1688 à 0, +12 et +20
  for base in 36020 112476; do
    poke "$out.tmp" $((base - 4)) '\024\000\000\000'
    poke "$out.tmp" $((base + 8)) '\024\000\000\000'
    for rel in 0 12 20; do poke "$out.tmp" $((base + rel)) '\230\006\000\000'; done
  done
  if [ "$(md5 "$out.tmp")" = "$ISP_MD5_PATCHED" ]; then
    mv -f "$out.tmp" "$out"; log "ISP patché (20 i/s)"
  else
    rm -f "$out.tmp"; log "ISP : résultat inattendu, patch annulé"; return 1
  fi
}

patch_ipc
patch_isp
exit 0
