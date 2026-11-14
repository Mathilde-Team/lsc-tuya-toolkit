#!/bin/sh
# Compile md_inject.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
# La table des DP et la couche d'appel sont générées : gen_gateway.py d'abord (depuis la racine du dépôt).
set -e
cd "$(dirname "$0")"

# Symboles U attendus : chacun doit exister dans la libc.so.0 de la caméra (uClibc).
# Toute apparition d'un nouveau symbole doit être vérifiée sur la caméra avant déploiement.
EXPECTED="atoi close connect getenv ht_montion_detect_open ht_montion_detect_set_active_cb ht_montion_detect_set_human_filter ht_montion_detect_set_sensitivity ht_night_mode_set_irled_mode inet_addr memcpy memset open64 poll pthread_create read sleep socket strcat strcmp strcpy strlen strncpy strrchr syscall system unlink usleep write"

arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
  -o md_inject.so md_inject.c
arm-linux-gnueabi-strip md_inject.so
arm-linux-gnueabi-readelf -d md_inject.so | grep -E "NEEDED|SONAME" || true
arm-linux-gnueabi-nm -D md_inject.so

# Les IPC_APP_* viennent du binaire anyka_ipc (table des DP) ; le reste doit être
# strictement la liste ci-dessus (pas de division : pas de libgcc).
GOT=$(arm-linux-gnueabi-nm -D md_inject.so | awk '$1 == "U" && $2 !~ /^IPC_APP_/ {print $2}' | sort)
EXP=$(echo "$EXPECTED" | tr ' ' '\n' | sed '/^$/d' | sort)
if [ "$GOT" != "$EXP" ]; then
    echo "SYMBOLES U INATTENDUS :" >&2
    echo "$EXP" | diff - "$GOT" >&2 || true
    exit 1
fi
ls -l md_inject.so
