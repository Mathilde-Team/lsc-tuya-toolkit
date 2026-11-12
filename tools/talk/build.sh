#!/bin/sh
# Compile talk_inject.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
set -e
cd "$(dirname "$0")"
arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
  -o talk_inject.so talk_inject.c
arm-linux-gnueabi-strip talk_inject.so
arm-linux-gnueabi-readelf -d talk_inject.so | grep -E "NEEDED|SONAME" || true
arm-linux-gnueabi-nm -D talk_inject.so
ls -l talk_inject.so
