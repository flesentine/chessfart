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

echo "Release audit passed for Chess Fart $VERSION."
