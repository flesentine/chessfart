#!/usr/bin/env sh
set -eu

OW_URL="https://github.com/open-watcom/open-watcom-v2/releases/download/2026-07-13-Build/ow-snapshot.tar.xz"
OW_SHA256="35e9c56ca93234ba0b8b596387815399317ea0548aeac1318f0674dca3921314"
OW_ROOT="${WATCOM:-$HOME/open-watcom-2026-07-13}"
ARCHIVE="${RUNNER_TEMP:-/tmp}/ow-snapshot-2026-07-13.tar.xz"

if [ ! -x "$OW_ROOT/binl64/wcl" ]; then
    rm -rf "$OW_ROOT"
    mkdir -p "$OW_ROOT"
    curl -L --fail --retry 3 "$OW_URL" -o "$ARCHIVE"
    printf '%s  %s\n' "$OW_SHA256" "$ARCHIVE" | sha256sum -c -
    tar -xJf "$ARCHIVE" -C "$OW_ROOT"
fi

printf 'WATCOM=%s\n' "$OW_ROOT"
printf 'PATH=%s/binl64:%s/binw:%s\n' "$OW_ROOT" "$OW_ROOT" "$PATH"
printf 'INCLUDE=%s/h\n' "$OW_ROOT"
