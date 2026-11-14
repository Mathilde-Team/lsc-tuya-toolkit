#!/bin/sh
# Compile md_inject.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
# La passerelle générique (tools/gateway/gateway.c, #53) et la découverte HA (ha.c, #59) sont liées dans la même bibliothèque.
set -e
cd "$(dirname "$0")"
python3 ../gateway/gen_gateway.py ../../docs/firmware/DP_TABLE.md ../gateway/dp_table.h ../../docs/TOPICS.md ../../docs/firmware/DP_VALUES.md
python3 ../gateway/gen_ha_discovery.py ../gateway/dp_table.h ../gateway/ha_discovery.h
arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -fvisibility=hidden -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -I../gateway \
  -o md_inject.so md_inject.c ../gateway/gateway.c ../gateway/ha.c
arm-linux-gnueabi-strip md_inject.so
arm-linux-gnueabi-readelf -d md_inject.so | grep -E "NEEDED|SONAME" || true
arm-linux-gnueabi-nm -D md_inject.so
# Garde-fou uClibc : aucun symbole libc non déjà importé, aucune routine libgcc (division).
ALLOWED="atoi close connect getenv inet_addr memcpy memset open64 open poll pthread_create read sleep socket strcat strcmp strcpy strlen strncpy strrchr syscall system unlink usleep write"
for u in $(arm-linux-gnueabi-nm -D --undefined-only md_inject.so | awk '$1=="U"{print $2}'); do
  case "$u" in ht_*|IPC_APP_*) continue ;; esac
  echo " $ALLOWED " | grep -q " $u " || { echo "symbole non autorisé : $u" >&2; exit 1; }
done
ls -l md_inject.so
