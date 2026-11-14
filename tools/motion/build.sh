#!/bin/sh
# Compile md_inject.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
# Vérifications après compilation :
#   1. aucun symbole __aeabi_* (libgcc non liée : pas de division dans le code) ;
#   2. chaque symbole U est soit une fonction du firmware (présente dans
#      docs/FIRMWARE_COMMANDS.md, donc exportée par anyka_ipc), soit une fonction
#      connue de la libc.so.0 de la caméra (liste blanche ci-dessous).
set -e
cd "$(dirname "$0")"
CC=arm-linux-gnueabi-gcc
arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
  -I../gateway -o md_inject.so md_inject.c
arm-linux-gnueabi-strip md_inject.so
arm-linux-gnueabi-readelf -d md_inject.so | grep -E "NEEDED|SONAME" || true

# 1. pas de division (aucune aide __aeabi_* de libgcc)
if arm-linux-gnueabi-nm -D md_inject.so | grep -q __aeabi; then
    echo "ERREUR : symbole __aeabi_* présent (division) ; libgcc n'est pas liée" >&2
    exit 1
fi

# 2. symboles U : firmware ou libc (liste blanche des fonctions uClibc déjà utilisées)
LIBC=" accept atoi close connect getenv htons inet_addr memcpy memset open open64 poll pthread_create read sleep socket strcmp strcpy strlen strncpy strcat strncmp strrchr syscall system unlink usleep write "
BAD=0
for s in $(arm-linux-gnueabi-nm -D md_inject.so | awk '$1=="U"{print $2}'); do
    case $s in
        IPC_APP_*|ht_*)
            grep -q "\`$s\`" ../../docs/FIRMWARE_COMMANDS.md || {
                echo "ERREUR : $s absente de docs/FIRMWARE_COMMANDS.md" >&2
                BAD=1
            }
            ;;
        *)
            case "$LIBC" in
                *" $s "*) ;;
                *) echo "ERREUR : symbole U inconnu (ni firmware ni liste libc) : $s" >&2; BAD=1 ;;
            esac
            ;;
    esac
done
[ $BAD -eq 0 ] || exit 1
echo "symboles U vérifiés (firmware + libc)"
arm-linux-gnueabi-nm -D md_inject.so
ls -l md_inject.so
