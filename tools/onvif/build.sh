#!/bin/bash
# Compile la pile ONVIF legere pour la camera (ARM926, statique) :
#   - onvif_simple_server (roleoroleo, GPL-3.0)  -> sd_card/custom/onvif/www/onvif/
#   - mini_cgi_httpd (ce depot)                  -> sd_card/custom/bin/
# Prerequis PC : arm-linux-gnueabi-gcc, cmake, make, curl, git.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="${WORK:-$HERE/_build}"
CC="arm-linux-gnueabi-gcc"
export CC AR=arm-linux-gnueabi-ar RANLIB=arm-linux-gnueabi-ranlib STRIP=arm-linux-gnueabi-strip
export CFLAGS="-Os -march=armv5te -marm"

mkdir -p "$WORK" && cd "$WORK"
[ -d onvif_simple_server ] || git clone -q https://github.com/roleoroleo/onvif_simple_server.git
git -C onvif_simple_server checkout -q -f 7b3941a   # version testee
# Positions absolues PTZ normalisees -1..1 (voir docs/ONVIF.md) : x/y d'AbsoluteMove et de GetStatus
git -C onvif_simple_server apply "$HERE/ptz_absolu_normalise.patch"
S="$WORK/onvif_simple_server/extras"

curl -sSLo zlib.tgz https://github.com/madler/zlib/releases/download/v1.3.1/zlib-1.3.1.tar.gz
curl -sSLo jsonc.tgz https://github.com/json-c/json-c/archive/refs/tags/json-c-0.18-20240915.tar.gz
curl -sSLo mbedtls.tbz https://github.com/Mbed-TLS/mbedtls/releases/download/mbedtls-3.6.2/mbedtls-3.6.2.tar.bz2

rm -rf "$S/zlib" "$S/json-c" "$S/mbedtls"
tar xzf zlib.tgz && mv zlib-1.3.1 "$S/zlib"
(cd "$S/zlib" && ./configure --static >/dev/null && make -j4 >/dev/null)

tar xzf jsonc.tgz && mv json-c-json-c-0.18-20240915 "$S/json-c"
mkdir -p "$S/json-c/build"
(cd "$S/json-c/build" && cmake .. -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_C_COMPILER="$CC" \
    -DCMAKE_C_FLAGS="$CFLAGS" -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIBS=ON -DBUILD_TESTING=OFF \
    -DBUILD_APPS=OFF -DDISABLE_WERROR=ON -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$PWD/_install" >/dev/null && make -j4 >/dev/null && make install >/dev/null)

tar xjf mbedtls.tbz && mv mbedtls-3.6.2 "$S/mbedtls"
cp "$S/mbedtls_config.h" "$S/mbedtls/include/mbedtls/mbedtls_config.h"
(cd "$S/mbedtls" && make -j4 lib CC="$CC" AR="$AR" CFLAGS="$CFLAGS" >/dev/null)

# La config est lue sur la SD, le journal va en RAM
cd "$WORK/onvif_simple_server"
sed -i -e 's#"/etc/onvif_simple_server.conf"#"/tmp/sd/custom/configs/onvif_simple_server.conf"#' \
       -e 's#"/var/log/onvif_simple_server.log"#"/tmp/onvif_simple_server.log"#' onvif_simple_server.c
HAVE_MBEDTLS=1 make -f Makefile.static -j4 CC="$CC -static $CFLAGS" STRIP="$STRIP" onvif_simple_server

$CC -static $CFLAGS -Wall -Wextra -o mini_cgi_httpd "$HERE/mini_cgi_httpd.c"
$STRIP mini_cgi_httpd

# Installation dans l'arborescence de la SD
SD="$HERE/../../sd_card/custom"
mkdir -p "$SD/onvif/www/onvif" "$SD/bin"
cp onvif_simple_server "$SD/onvif/www/onvif/"
for d in device_service_files events_service_files generic_files media_service_files \
         media2_service_files ptz_service_files deviceio_service_files; do
    cp -R "$d" "$SD/onvif/www/onvif/"
done
cp mini_cgi_httpd "$SD/bin/"
echo "OK"
