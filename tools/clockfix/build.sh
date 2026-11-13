#!/bin/sh
# Compile clockfix.so pour l'AK3918 (ARM926, uClibc) sans dépendre de la glibc.
set -e
cd "$(dirname "$0")"
arm-linux-gnueabi-gcc -march=armv5te -mfloat-abi=soft -Os -fPIC -shared -nostdlib \
  -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 \
  -o clockfix.so clockfix.c
arm-linux-gnueabi-strip clockfix.so
arm-linux-gnueabi-readelf -d clockfix.so | grep -E "NEEDED|SONAME" || true
arm-linux-gnueabi-nm -D clockfix.so
ls -l clockfix.so
