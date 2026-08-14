#!/bin/sh
set -eu

SCRIPT=${VERSION_SCRIPT:-./gen_version_h.pl}
TMPDIR_BASE=${TMPDIR:-/tmp}
WORK=$(mktemp -d "$TMPDIR_BASE/uge-version-regression.XXXXXX")
trap 'rm -rf "$WORK"' EXIT HUP INT TERM
checks=0

fail() {
   echo "[FAIL] version regression: $*" >&2
   exit 1
}

# GitHub tag wins over every fallback.
checks=$((checks + 1))
(
   cd "$WORK"
   GITHUB_REF_TYPE=tag GITHUB_REF_NAME=v9.8.7 \
      GITHUB_SHA=0123456789abcdef0123456789abcdef01234567 \
      "$OLDPWD/$SCRIPT" tag.h
)
grep -F '#define UGE_VERSION "v9.8.7"' "$WORK/tag.h" >/dev/null || fail 'GitHub tag not selected'
grep -F '#define UGE_VERSION_SOURCE "github-tag"' "$WORK/tag.h" >/dev/null || fail 'GitHub tag source missing'

# With no tag, a GitHub/Git commit is prefixed with g, never v.
checks=$((checks + 1))
(
   cd "$WORK"
   env -u GITHUB_REF_TYPE -u GITHUB_REF_NAME \
      GITHUB_SHA=0123456789abcdef0123456789abcdef01234567 \
      "$OLDPWD/$SCRIPT" commit.h
)
grep -F '#define UGE_VERSION "g0123456789ab"' "$WORK/commit.h" >/dev/null || fail 'commit fallback incorrect'
grep -F '#define UGE_VERSION_SOURCE "git-commit"' "$WORK/commit.h" >/dev/null || fail 'commit source missing'

# With no Git/GitHub metadata, use d + UTC ISO-8601.
checks=$((checks + 1))
(
   cd "$WORK"
   env -u GITHUB_REF_TYPE -u GITHUB_REF_NAME -u GITHUB_SHA \
      "$OLDPWD/$SCRIPT" date.h
)
grep -Eq '^#define UGE_VERSION "d[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z"$' "$WORK/date.h" || fail 'date fallback is not d + ISO-8601'
grep -F '#define UGE_VERSION_SOURCE "date"' "$WORK/date.h" >/dev/null || fail 'date source missing'

# The compiled program must report exactly the generated version string.
checks=$((checks + 1))
expected=$(sed -n 's/^#define UGE_VERSION "\(.*\)"$/\1/p' version.h)
actual=$(./uge -V | sed -n '1s/^uge //p')
[ -n "$expected" ] || fail 'version.h has no UGE_VERSION'
[ "$actual" = "$expected" ] || fail "uge -V reports '$actual', expected '$expected'"

printf '[PASS] version regression: %d cases\n' "$checks"
