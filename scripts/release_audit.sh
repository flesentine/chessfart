#!/usr/bin/env sh
set -eu

VERSION="$(cat VERSION)"
case "$VERSION" in
  1.0.0) ;;
  *) echo "Unexpected VERSION: $VERSION" >&2; exit 1 ;;
esac

test -f include/version.h
test -f release/README.TXT
test -f release/KEYS.TXT
test -f release/KNOWNISS.TXT
test -f release/DOSBOX.CONF
test -f docs/REPLAY_FORMAT.md

grep -q '^CHESS FART 1.0.0 CONTROLS$' release/KEYS.TXT
if grep -q '1.0.0-rc1' release/KEYS.TXT release/README.TXT release/KNOWNISS.TXT; then
    echo "Current release documentation still contains stale rc1 metadata." >&2
    exit 1
fi
grep -q '2 PLAYERS' release/README.TXT
grep -q 'format version 2' release/README.TXT
grep -q 'Legacy version-1 game saves remain loadable and restore CPU mode' release/README.TXT
grep -q 'CHESSFRT.RPL' release/README.TXT
grep -q 'CHESSFRT.RPL' release/KEYS.TXT
grep -q '^THEMES$' release/README.TXT
grep -q 'Royal Basement / Crimson Cellar' release/README.TXT
grep -q 'CHESSFRT.CFG version 2' release/README.TXT
grep -q 'T                  Cycle Royal Basement / Crimson Cellar theme' release/KEYS.TXT
grep -q 'CHESSFART_REPLAY 1' docs/REPLAY_FORMAT.md

# A bare ======= line is valid DOS text decoration, so only the unambiguous
# opening/closing merge markers are release blockers here.
if grep -R -n -E '^(<<<<<<<|>>>>>>>)' src include tests release docs >/tmp/chessfart-conflicts.txt; then
    cat /tmp/chessfart-conflicts.txt >&2
    exit 1
fi

if grep -R -n -E '\b(TODO|FIXME|XXX)\b' src include >/tmp/chessfart-release-todos.txt; then
    cat /tmp/chessfart-release-todos.txt >&2
    echo "Release source contains TODO/FIXME/XXX markers." >&2
    exit 1
fi

legacy_runtime="$(find src include scripts -type f \
    \( -name 'main_build*' -o -name 'board_view_build*' \
       -o -name '*_build*_host.c' -o -name '*_build*_dos.c' \
       -o -name '*_build*.h' -o -name 'build_dos_build*.bat' \) -print)"
if [ -n "$legacy_runtime" ]; then
    printf '%s\n' "$legacy_runtime" >&2
    echo "Retired build-lineage runtime scaffolding has reappeared." >&2
    exit 1
fi

legacy_makefiles="$(find . -maxdepth 1 -type f -name 'makefile.build*.dos' -print)"
if [ -n "$legacy_makefiles" ]; then
    printf '%s\n' "$legacy_makefiles" >&2
    echo "Retired per-build DOS makefiles have reappeared." >&2
    exit 1
fi

git diff --check

echo "Release audit passed for Chess Fart $VERSION."
