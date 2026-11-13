#!/bin/sh
# Compile md_inject.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
set -e
cd "$(dirname "$0")"
arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
  -o md_inject.so md_inject.c
arm-linux-gnueabi-strip md_inject.so
arm-linux-gnueabi-readelf -d md_inject.so | grep -E "NEEDED|SONAME" || true
arm-linux-gnueabi-nm -D md_inject.so
ls -l md_inject.so
