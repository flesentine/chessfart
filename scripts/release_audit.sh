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

git diff --check

# Temporary test probe: find one deterministic position where the corrected
# edge-bonus alpha-beta window and the pre-v8 unshifted window choose
# different root actions. This is replaced by a frozen regression fixture
# before the PR is ready to merge.
mkdir -p build/host
cc -std=c89 -pedantic -Wall -Wextra -Werror -O2 -Iinclude \
  tests/scan_alpha_beta.c \
  src/game/board.c src/game/gas.c src/game/cpu_config.c \
  src/game/cpu_actions.c src/game/cpu_eval.c src/game/cpu_search.c \
  src/game/cpu_format.c -o build/host/scan_alpha_beta
./build/host/scan_alpha_beta

echo "Release audit passed for Chess Fart $VERSION."
