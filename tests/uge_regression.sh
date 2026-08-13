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
