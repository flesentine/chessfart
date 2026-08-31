#!/usr/bin/env sh
set -eu

VERSION="$(cat VERSION)"
ROOT="build/release/CHESSFRT"
ZIP="build/release/chessfart-${VERSION}-dos.zip"
SHA="${ZIP}.sha256"
COMMIT="$(git rev-parse HEAD)"

if [ ! -s build/dos/CHESSFRT.EXE ]; then
    echo "Missing build/dos/CHESSFRT.EXE; build the DOS target first." >&2
    exit 1
fi

rm -rf build/release
mkdir -p "$ROOT"
cp build/dos/CHESSFRT.EXE "$ROOT/CHESSFRT.EXE"
cp release/DOSBOX.CONF "$ROOT/DOSBOX.CONF"
cp release/README.TXT "$ROOT/README.TXT"
cp release/KEYS.TXT "$ROOT/KEYS.TXT"
cp release/KNOWNISS.TXT "$ROOT/KNOWNISS.TXT"

cat > "$ROOT/BUILDINFO.TXT" <<EOF
CHESS FART ${VERSION}
Source commit: ${COMMIT}
Reference compiler: Open Watcom V2 2026-07-13-Build
Open Watcom snapshot SHA-256: 35e9c56ca93234ba0b8b596387815399317ea0548aeac1318f0674dca3921314
Target: 16-bit DOS, large memory model, 386 instruction selection
Video: VGA Mode 13h, 320x200x256
Audio: Sound Blaster 8-bit PCM / PC speaker fallback
Build command: scripts/build_dos_ci.sh
EOF

(cd build/release && zip -X -r "$(basename "$ZIP")" CHESSFRT >/dev/null)
sha256sum "$ZIP" > "$SHA"
unzip -t "$ZIP" >/dev/null

for required in CHESSFRT.EXE DOSBOX.CONF README.TXT KEYS.TXT KNOWNISS.TXT BUILDINFO.TXT; do
    unzip -Z1 "$ZIP" | grep -qx "CHESSFRT/$required"
done

printf 'Release package: %s\n' "$ZIP"
cat "$SHA"
