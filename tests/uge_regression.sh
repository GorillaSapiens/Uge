#!/bin/sh
set -eu

UGE=${UGE:-./uge}
TMPDIR_BASE=${TMPDIR:-/tmp}
WORK=$(mktemp -d "$TMPDIR_BASE/uge-regression.XXXXXX")
trap 'rm -rf "$WORK"' EXIT HUP INT TERM

checks=0

fail() {
   echo "[FAIL] uge regression: $*" >&2
   exit 1
}

run_exact() {
   name=$1
   input=$2
   expected=$3
   checks=$((checks + 1))
   printf '%s' "$input" | "$UGE" -q >"$WORK/out" 2>"$WORK/err" || fail "$name: uge exited nonzero"
   [ ! -s "$WORK/err" ] || {
      echo "--- stderr ---" >&2
      cat "$WORK/err" >&2
      fail "$name: unexpected stderr"
   }
   printf '%s' "$expected" >"$WORK/expected"
   if ! cmp -s "$WORK/expected" "$WORK/out"; then
      echo "--- expected ---" >&2
      cat "$WORK/expected" >&2
      echo "--- got ---" >&2
      cat "$WORK/out" >&2
      fail "$name: stdout mismatch"
   fi
}

run_error() {
   name=$1
   input=$2
   needle=$3
   checks=$((checks + 1))
   printf '%s' "$input" | "$UGE" -q >"$WORK/out" 2>"$WORK/err" || fail "$name: uge exited nonzero"
   grep -F "$needle" "$WORK/err" >/dev/null || {
      echo "--- stderr ---" >&2
      cat "$WORK/err" >&2
      fail "$name: expected error containing '$needle'"
   }
}

run_contains() {
   name=$1
   input=$2
   needle=$3
   checks=$((checks + 1))
   printf '%s' "$input" | "$UGE" -q >"$WORK/out" 2>"$WORK/err" || fail "$name: uge exited nonzero"
   [ ! -s "$WORK/err" ] || {
      echo "--- stderr ---" >&2
      cat "$WORK/err" >&2
      fail "$name: unexpected stderr"
   }
   grep -F "$needle" "$WORK/out" >/dev/null || {
      echo "--- stdout ---" >&2
      cat "$WORK/out" >&2
      fail "$name: expected stdout containing '$needle'"
   }
}

run_cli_error() {
   name=$1
   expected_status=$2
   needle=$3
   shift 3
   checks=$((checks + 1))
   set +e
   "$UGE" "$@" >"$WORK/out" 2>"$WORK/err"
   status=$?
   set -e
   [ "$status" -eq "$expected_status" ] || fail "$name: exit $status, expected $expected_status"
   grep -F "$needle" "$WORK/err" >/dev/null || {
      echo "--- stderr ---" >&2
      cat "$WORK/err" >&2
      fail "$name: expected error containing '$needle'"
   }
}

run_complex_roundtrip() {
   name=$1
   setup=$2
   expression=$3
   checks=$((checks + 1))
   first=$(printf '%s\n%s\nquit\n' "$setup" "$expression" | "$UGE" -q 2>"$WORK/err") || fail "$name: first parse exited nonzero"
   [ ! -s "$WORK/err" ] || fail "$name: first parse wrote stderr"
   second=$(printf '%s\n%s\nquit\n' "$setup" "$first" | "$UGE" -q 2>"$WORK/err") || fail "$name: round-trip parse exited nonzero"
   [ ! -s "$WORK/err" ] || fail "$name: round-trip parse wrote stderr"
   [ "$second" = "$first" ] || fail "$name: '$first' round-tripped as '$second'"
}

run_exact "basic arithmetic" \
'1+2
1/3+1/6
2**20
quit
' \
'3
0.5
1048576
'

run_exact "exact normalized trig" \
'sin(pi/6)
cos(pi)
tan(pi/4)
sinpi(1/6)
sintau(1/4)
sindeg(30)
quit
' \
'0.5
-1
1
0.5
1
0.5
'

run_exact "normalized trig companion identity" \
'precision=64
sinpi(1/3)/sqrt(3)
cospi(1/6)/sqrt(3)
sintau(2/3)/sqrt(3)
quit
' \
'0.5
0.5
-0.5
'

run_exact "constant identities" \
'tau==2*pi
e==e(1)
pi==pi()
quit
' \
'1
1
1
'

run_exact "Ce reconstruction" \
'sqrt(3)*sqrt(3)
sin(pi/3)*sqrt(3)
sin(pi/3)/sqrt(3)
quit
' \
'3
1.5
0.5
'

run_exact "exact nearby rational is not reconstructed" \
'fraction(3000000000000000000000000000001/1000000000000000000000000000000)
quit
' \
"3'1/1000000000000000000000000000000
"

run_contains "debug reconstructed value remains approximate" \
'debug(sqrt(3)*sqrt(3))
quit
' 'exact=false'
run_contains "debug exact value stays exact" \
'debug(3)
quit
' 'exact=true'

run_exact "errors display toggle" \
'precision=16
errors
3
errors on
3
sqrt(3)*sqrt(3)
errors
errors off
3
errors
quit
' \
'errors off
3
3 +/- 0
3 +/- 0.004658912085509447909004165921942330896854400634765625
errors on
3
errors off
'

run_exact "error output form" \
'precision=16
error(3)
errors on
error(3)
error(sqrt(-2))
format fraction
error(sqrt(3)*sqrt(3))
quit
' \
"0
0
0.001178808510303497314453125i
using fraction format; enter 'format positional' for positional format
83927498929009/18014398509481984
"

run_error "bad errors setting" \
'errors maybe
quit
' \
"errors must be 'on' or 'off'"


run_exact "complex arithmetic" \
'sqrt(-1)
(1+2i)*(3-4i)
conj(1+2i)
norm(1+2i)
real(3+4i)
imag(3+4i)
quit
' \
'i
11+2i
1-2i
5
3
4
'

run_complex_roundtrip "complex repeating positional round-trip" "base 10" "-1/3+2/7i"
run_complex_roundtrip "complex braced-digit round-trip" "base 65536" "{65535}+{65534}i"

run_exact "fraction and positional formats" \
'format fraction
1/3
format positional
1/3
quit
' \
"using fraction format; enter 'format positional' for positional format
1/3
using positional format; enter 'format fraction' for fraction format
0.(3)
"

checks=$((checks + 1))
printf '1/3\nquit\n' | "$UGE" -q -fraction >"$WORK/out" 2>"$WORK/err"
[ ! -s "$WORK/err" ] || fail "-fraction: unexpected stderr"
[ "$(cat "$WORK/out")" = "1/3" ] || fail "-fraction: wrong output"

run_exact "arbitrary radix" \
'base 12
.49+.03
format fraction
0.5
base 36
Z
base 37
{36}+1
1+{36}
base 65536
{65535}
base 10
quit
' \
"0.5
using fraction format; enter 'format positional' for positional format
5/10
Z
10
10
{65535}
"

run_exact "variables and semicolons" \
'x=5
x*=7
x
1+2;3+4
last
quit
' \
'35
3
7
7
'

run_exact "if else and loops" \
'x=0
if (1) {
 x=10
}
else {
 x=20
}
x
j=0
s=0
while (j < 5) {
 j=j+1
 if (j == 3) continue
 s=s+j
}
s
s=0
for (j=0; j<10; j=j+1) {
 if (j == 5) break
 s=s+j
}
s
quit
' \
'10
12
10
'

run_exact "functions locals recursion" \
'x=99
define gcd(a,b) {
 local t
 while (b != 0) {
  t=a%b
  a=b
  b=t
 }
 return(a)
}
gcd(123456,7890)
define fact(n) {
 if (n <= 1) return(1)
 return(n*fact(n-1))
}
fact(10)
define scope(x) {
 local y
 y=x+1
 return(y)
}
scope(4)
x
define zero() {
 local z
 return(z)
}
zero()
quit
' \
'6
3628800
5
99
0
'

run_exact "trigmode state" \
'trigmode
trigmode direct
trigmode
trigmode normalized
trigmode
quit
' \
'normalized
direct
normalized
'

run_error "reserved i assignment" 'i=5
quit
' 'left side of assignment is not a variable'
run_error "local outside function" 'local x
quit
' 'local used outside a function'
run_error "complex ordering" '1 < i
quit
' 'operation requires a real value'
run_error "approximate equality ambiguity" 'sqrt(3)*sqrt(3)==3
quit
' 'equality is ambiguous within Ce error bounds'
run_error "break outside loop" 'break
quit
' 'break used outside a loop'
run_error "continue outside loop" 'continue
quit
' 'continue used outside a loop'
run_error "return outside function" 'return(1)
quit
' 'return used outside a function'
run_error "old auto declaration rejected" 'define f() {
 auto t
 return(0)
}
f()
quit
' "unexpected 't'"

run_error "division by zero" '1/0
quit
' 'divide by zero'
run_error "modulo by zero" '1%0
quit
' 'divide by zero'
run_error "radix too small" 'base 1
quit
' 'base must be between 2 and 65536'
run_error "radix too large" 'base 65537
quit
' 'base must be between 2 and 65536'
run_error "digit outside radix" 'base 37
{37}
quit
' 'digit {37} is not valid in base 37'
run_error "digit exceeds maximum" 'base 65536
{65536}
quit
' 'digit value exceeds 65535'
run_error "malformed repeating fraction" '0.(3
' 'incomplete statement at end of input'
run_error "malformed complex literal" '1+2ii
quit
' "unexpected 'ii'"
run_error "negative shift" '1 << -1
quit
' 'shift count must not be negative'
run_error "wrong function argument count" 'define f(a) { return(a) }
f()
quit
' 'f() takes 1 argument'
run_error "recursion depth guard" 'define r() { return(r()) }
r()
quit
' 'function call depth exceeds 64'
run_error "reserved pi assignment" 'pi=5
quit
' 'left side of assignment is not a variable'
run_error "reserved e assignment" 'e=5
quit
' 'left side of assignment is not a variable'
run_error "reserved tau assignment" 'tau=5
quit
' 'left side of assignment is not a variable'
run_error "unmatched block" 'if (1) {
1
' 'incomplete statement at end of input'
run_cli_error "unknown option" 2 "unknown option '--definitely-not-an-option'" --definitely-not-an-option
run_cli_error "missing input file" 1 "cannot open" "$WORK/no-such-file.uge"

checks=$((checks + 1))
printf 'warranty\nquit\n' | "$UGE" -q >"$WORK/out" 2>"$WORK/err"
[ ! -s "$WORK/err" ] || fail "warranty: unexpected stderr"
grep -F 'THERE IS NO WARRANTY FOR THE PROGRAM' "$WORK/out" >/dev/null || fail "warranty text missing"
grep -F 'Limitation of Liability' "$WORK/out" >/dev/null || fail "liability text missing"

checks=$((checks + 1))
"$UGE" --help >"$WORK/out" 2>"$WORK/err"
[ ! -s "$WORK/err" ] || fail "--help: unexpected stderr"
grep -F -- '-fraction' "$WORK/out" >/dev/null || fail "--help missing -fraction"
grep -F -- '-positional' "$WORK/out" >/dev/null || fail "--help missing -positional"

printf '[PASS] uge regression: %d cases\n' "$checks"
