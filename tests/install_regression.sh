#!/bin/sh
set -eu

TMPDIR_BASE=${TMPDIR:-/tmp}
WORK=$(mktemp -d "$TMPDIR_BASE/uge-install-regression.XXXXXX")
trap 'rm -rf "$WORK"' EXIT HUP INT TERM

make install DESTDIR="$WORK" PREFIX=/usr >/dev/null
[ -x "$WORK/usr/bin/uge" ] || {
   echo '[FAIL] install regression: DESTDIR/PREFIX install did not create usr/bin/uge' >&2
   exit 1
}

out=$(printf 'sin(pi/6)\nquit\n' | "$WORK/usr/bin/uge" -q)
[ "$out" = '0.5' ] || {
   echo "[FAIL] install regression: installed uge returned '$out'" >&2
   exit 1
}

make uninstall DESTDIR="$WORK" PREFIX=/usr >/dev/null
[ ! -e "$WORK/usr/bin/uge" ] || {
   echo '[FAIL] install regression: uninstall left usr/bin/uge behind' >&2
   exit 1
}

printf '[PASS] install regression: DESTDIR/PREFIX install/uninstall\n'
