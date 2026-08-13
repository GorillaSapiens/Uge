# `uge` calculator

`uge` is an interactive exact rational/complex calculator built on Uge's `C`
type.  Its command language intentionally resembles GNU `bc`.  Every calculator
value is a complex number whose real and imaginary components are `Q` rationals.
When the imaginary component is zero it is omitted from normal output, so ordinary
real calculations retain the same appearance and exact-rational behavior.

That distinction matters for non-decimal input.  For example:

```
base 12
.49 + .03
0.5
```

The two input values are converted directly to exact rational values before the
addition.  No conversion through decimal fixed-point takes place.

## Building

```
make
```

This builds `uge`, `ntest`, `qtest`, and `ctest`.

Run the calculator with:

```
./uge
```

Command-line options include:

```
-q, --quiet   suppress the interactive banner
-l            accepted for GNU bc compatibility; no separate math library is needed
-positional   start with positional output format (default)
-fraction     start with fraction output format
-h, --help    show command-line help
```

If both `-positional` and `-fraction` are given, the last one wins.  The selected
format is in effect while command files are evaluated as well as during
interactive input.  One or more file names may be given; they are evaluated in
order before interactive input is read.

## Interactive editing and history

When standard input is a terminal, `uge` provides its own small line editor so
it does not require readline or another editing library.

- Up/Down: previous/next history entry
- Left/Right: move within the line
- Home/End: beginning/end of line
- Backspace/Delete: delete characters
- Ctrl-A/Ctrl-E: beginning/end of line
- Ctrl-U: delete back to the beginning
- Ctrl-K: delete to the end
- Ctrl-L: clear and redraw the screen
- Ctrl-C: cancel the current input line
- Ctrl-D on an empty line: exit

History is saved in `~/.uge_history` and restored at the next interactive run.

## Numbers and bases

`ibase` controls the radix used to parse ordinary numeric literals.  `obase`
controls the radix used for normal output.

The convenient `base` extension changes both at once:

```
ibase = 12
obase = 16
base 8
base = 10
```

`ibase`, `obase`, and `base` values are **always parsed in base 10**, regardless
of the current `ibase`.  This is deliberate.  A command such as:

```
base 36
```

always means radix thirty-six; you never have to work out how to spell decimal
36 in the radix you are trying to leave.

Supported radices are 2 through 65536.

Digits are written as:

```
0 .. 9
A .. Z
{36} .. {65535}
```

The integer inside `{...}` is always written in decimal.  Lowercase names are
reserved for variables and functions, which keeps `a` unambiguously a variable
while `A` is a radix digit.

Examples:

```
base 16
FF + 1
100

base 37
{36} + 1
10

base 65536
{65535} + 1
10
```

A repeating positional fraction uses parentheses:

```
base 10
1 / 3
0.(3)

base 2
1 / 5
0.(0011)
```

Mixed fractions such as `2'1/3` are also accepted as numeric literals.


### Complex values

The built-in constant `i` is the imaginary unit.  A numeric coefficient may be
written directly before `i`, so the calculator accepts the same forms that its
normal complex printer emits:

```
i
i * i
-1

2i
1 + 2i
1+2i

1/2i
0.5i
```

The adjacency rule is deliberately limited to a trailing `i`; Uge does not
otherwise add general implicit multiplication.  Thus `2i`, `1/2i`, and `(1+2)i`
mean multiplication by `i`, while unrelated forms such as `2pi` still require
an explicit `*`.

The real and imaginary components independently retain exact `Q` arithmetic.
For example:

```
fraction((1/2 + 1/3i) + (1/4 - 1/7i))
3/4+4/21i
```

Radix input and output apply to both components.  Lowercase `i` remains distinct
from the uppercase radix digits used for bases above 10.

## Expressions

The usual `bc`-style arithmetic operators are supported:

```
+  -  *  /  %  ^
```

`^` is exponentiation, as in GNU `bc`; `**` is accepted as an alias.
Exponentiation is right-associative.

```
2 ^ 10
1024

2 ^ 3 ^ 2
512
```

Unary operators are:

```
+  -  !  ~
```

`~` and the bitwise operators below require a real operand and then use the
underlying `Q` operation:

```
&  |  <<  >>
```

Because `^` is used for `bc`-compatible exponentiation, bitwise exclusive-or is
available as the function `xor(a,b)`.

Comparisons produce 0 or 1:

```
==  !=  <  <=  >  >=
```

`==` and `!=` compare both complex components.  Complex numbers have no natural
ordering, so `<`, `<=`, `>`, and `>=` require both operands to be real.  `%`,
`~`, bitwise operators, shifts, `floor()`, `sgn()`, and the `atan2` family also
require the relevant operands to be real.

Logical tests treat `0+0i` as false and every other complex value as true.
Logical AND and OR are also supported:

```
&&  ||
```

Parentheses follow the normal rules of precedence.  Statements may be separated
with semicolons or newlines.  Expressions may continue on another line while
parentheses remain open, or when the line ends with `\`.  Braced statement
blocks are multiline as well.

Comments may use any of:

```
/* block comment */
// rest of line
# rest of line
```

## Variables

Variable names begin with a lowercase letter or underscore and may then contain
lowercase letters, digits, and underscores.  Unassigned variables have value 0,
as in `bc`.

```
a = 12
b = a * 3
b
36
```

Assignment statements do not print a result.  The following compound assignments
are supported:

```
+=  -=  *=  /=  %=  ^=
```

Prefix and postfix `++` and `--` are supported as well.

The special variable `last` contains the last value printed by an expression or
an explicit output command.

## Statements and control flow

`uge` supports `bc`-style conditional and loop statements.  Conditions use the
same truth rule as logical expressions: `0+0i` is false and every other value
is true.  A body may be a single statement or a braced block.

```
if (x > 0)
    y = x
else
    y = -x

if (z) {
    real_part = real(z)
    imag_part = imag(z)
}
else {
    real_part = 0
    imag_part = 0
}
```

The `else` belongs to the nearest unmatched `if`.  With a braced top-level
`if`, the closing `}` and `else` may be entered on separate lines.

Loops use the familiar forms:

```
count = 0
while (count < 10) {
    count += 1
}

sum = 0
for (k = 0; k < 10; k++) {
    sum += k
}
```

The three expressions in a `for` header are control expressions and are not
printed.  Any of them may be empty; an empty condition is true.  `break` exits
the nearest loop and `continue` starts its next iteration.

Because `i` is the built-in imaginary unit, it is not available as a loop
variable; names such as `j`, `k`, or `n` are convenient alternatives.

## User-defined functions

Scalar user-defined functions use `bc`-like `define` syntax.  Every
argument, local, and return value is an ordinary `C` calculator value; arrays
are deliberately not implemented.

```
define gcd(a, b) {
    local t
    while (b != 0) {
        t = a % b
        a = b
        b = t
    }
    return(a)
}

gcd(123456, 7890)
6
```

Parameters are local to the call.  Variables named by a `local` declaration
are also local and begin at zero.  Other variable references are global, which
matches the traditional `bc` model.  Multiple local names may be declared at
once:

```
local x, y, t
```

Functions may call other user functions and may recurse:

```
define fact(n) {
    if (n <= 1)
        return(1)
    return(n * fact(n - 1))
}

fact(10)
3628800
```

`return(expr)` or `return expr` returns a value immediately.  A bare `return`
and a function that reaches the end without a `return` both return zero.
Function definitions are global and may be replaced by a later `define`.
Built-in function names and the reserved constants `i`, `e`, `pi`, and `tau`
cannot be reused as function, parameter, or `local` names.  Function call depth
is limited to 256 calls.

## Built-in functions

Functions implemented by the current `C` type are exposed directly:

```
sqrt(x)
sin(x)
cos(x)
tan(x)
atan(x)
atan2(y,x)

sinpi(x)
cospi(x)
tanpi(x)
atanpi(x)
atan2pi(y,x)

sintau(x)
costau(x)
tantau(x)
atantau(x)
atan2tau(y,x)

sindeg(x)
cosdeg(x)
tandeg(x)
atandeg(x)
atan2deg(y,x)

ln(x)
e
e(x)
pi
pi()
tau
tau()
abs(x)
real(x)
imag(x)
conj(x)
norm(x)
arg(x)
floor(x)
sgn(x)
pow(x,y)
xor(x,y)
```

The ordinary trig functions use radians.  `sin`, `cos`, `tan`, and `atan` accept
complex arguments.  `atan2(y,x)` remains the usual real quadrant-aware function,
returns an angle in `(-pi,pi]`, and requires real `y` and `x`; `atan2(0,0)` is
undefined.

`sqrt()`, `ln()`, `e()`, powers, and the ordinary trig functions use the complex
extensions supplied by `C`.  Where a complex function needs a branch, Uge uses
the principal value.  Thus, for example, `sqrt(-1)` is `i`.

Complex-specific helpers are:

```
real(z)   # real component
imag(z)   # imaginary component
conj(z)   # complex conjugate
norm(z)   # real^2 + imag^2
abs(z)    # magnitude
arg(z)    # principal argument in radians
```

`norm()` is exact when its component arithmetic is exact.  `abs()` and `arg()`
may require approximation.

The calculator has two evaluation modes for the ordinary radian trig names:

```
trigmode normalized
trigmode direct
```

`normalized` is the default.  In normalized mode the ordinary functions use
the calculator's cached `pi` value as the angular normalization unit:

```
sin(x)       -> sinpi(x/pi)
cos(x)       -> cospi(x/pi)
tan(x)       -> tanpi(x/pi)
atan(x)      -> atanpi(x)*pi
atan2(y,x)   -> atan2pi(y,x)*pi
```

This lets expressions involving the same cached `pi` value retain exact
normalized results where possible.  For example:

```
sin(pi/2)    -> 1
cos(pi)      -> -1
tan(pi/4)    -> 1
```

`direct` mode instead calls the ordinary `C::sin()`, `C::cos()`, `C::tan()`,
`C::atan()`, and `C::atan2()` implementations directly.  Because the cached
`pi` is a rational approximation, the two modes can differ very slightly for
general inputs.  Both modes still take ordinary `sin()`, `cos()`, and `tan()`
arguments in radians.

The explicit `*pi`, `*tau`, and `*deg` functions are never affected by
`trigmode`.  They always retain the meanings described below.  Enter
`trigmode` by itself to display the current setting.

The `*pi` variants measure angles in half-turns: `sinpi(x)` means `sin(pi*x)`,
while `atanpi(x)` and `atan2pi(y,x)` return their answers divided by pi.  The
`*tau` variants measure angles in turns: `sintau(x)` means `sin(tau*x)`, while
`atantau(x)` and `atan2tau(y,x)` return fractions of a full turn.  `atan2pi()`
returns in `(-1,1]` and `atan2tau()` returns in `(-1/2,1/2]`.  For real
rational arguments, normalized angles are reduced exactly before any
approximation is introduced.  Consequently important real values such as these
remain exact (and print with no imaginary component):

```
sinpi(1)       -> 0
cospi(1)       -> -1
tanpi(1/4)     -> 1
sintau(1/4)    -> 1
costau(1/2)    -> -1
atanpi(1)      -> 1/4
atantau(1)     -> 1/8
atan2tau(1,0)  -> 1/4
```

The `*deg` variants are convenience wrappers around the turn-based functions.
Forward trig divides the degree argument by 360 before calling the matching
`*tau` function, while inverse trig converts the returned fraction of a turn
back to degrees:

```
sindeg(x)       -> sintau(x/360)
cosdeg(x)       -> costau(x/360)
tandeg(x)       -> tantau(x/360)
atandeg(x)      -> atantau(x)*360
atan2deg(y,x)   -> atan2tau(y,x)*360
```

For real inputs this preserves exact degree results whenever the underlying
turn-normalized function has an exact rational result:

```
sindeg(30)      -> 0.5
cosdeg(60)      -> 0.5
tandeg(45)      -> 1
atandeg(1)      -> 45
atan2deg(1,0)   -> 90
```

`e`, `pi`, and `tau` are built-in values.  `pi` and `tau` may also be
written `pi()` and `tau()`.  Bare `e` is the cached approximation of Euler's
constant, while `e(x)` means e raised to the power x, so `e(1)` is the same
value as bare `e` and `e(0)` is 1.  `tau` is derived exactly as twice the cached
approximation of `pi`.

`sqrt()`, non-rational powers, trigonometric functions, `ln()`, `e()`, and
`pi` ultimately use `Q`'s rational approximation mechanism.  No floating-point
type is introduced: a complex approximate result is still a `C` containing
rational approximations in its `Q` components.  The special `precision` value
controls the working precision used when Uge must approximate a result.  It
defaults to 256:

```
precision = 8192
```

`precision` is always assigned using decimal input, regardless of `ibase`.

The calculator caches `pi` and `e` (`e(1)`) at the current `precision`.  At the
default precision of 256, the exact rational values produced by Uge's algorithms
are baked into the program so startup does not have to recompute them.  Other
precisions are computed normally.  Returning to precision 256 reloads the baked
values.  `tau` is always derived as exactly `2*pi`.  Changing `maxdigits` does
not invalidate the cache because it affects only output formatting.

The transcendental functions are implemented without floating point.  The `Q`
layer supplies the real rational series and range reduction; `C` builds the
standard complex extensions on top of those operations.  `pi` uses Machin's
arctangent formula.  During approximate calculations, intermediate `Q` values
are truncated to a binary working-precision grid to keep their exact rational
denominators from growing without useful bound.

## Output forms

Ordinary output can be rendered in either **positional** or **fraction** form.
Positional form is the default.  The persistent setting is selected with:

```
format positional
format fraction
```

Entering `format` by itself reports the current setting and how to switch to the
other form.  Changing the setting prints the same status message.  At interactive
startup Uge reports the selected format, for example:

```text
using positional format; enter 'format fraction' for fraction format
```

The startup selection can also be made from the command line:

```
./uge -positional
./uge -fraction
```

`-positional` is the default.  If both options are supplied, the last one wins.
The `-q` option suppresses the interactive startup banner and format message.

`format pos` and `format frac` are accepted as abbreviations.  The format setting
does not change `obase`: it only chooses whether ordinary results are rendered
as positional digits or as exact fractions.  For example:

```text
base 12
.49 + .03
0.5
foo = last
obase 10
foo
0.41(6)
format fraction
using fraction format; enter 'format positional' for positional format
foo
5/12
format positional
using positional format; enter 'format fraction' for fraction format
foo
0.41(6)
```

"Decimal notation" is the special case of positional notation where the radix is
10; it is not the right name for the same notation in another radix.

Several explicit top-level output forms are also provided:

```
positional(expression)
pos(expression)
fraction(expression)
frac(expression)
decimal(expression)
debug(expression)
print expression
```

`positional()` and `pos()` always print positional notation in `obase`, regardless
of the current `format`.  `fraction()` and `frac()` always print the exact value
as an integer, fraction, or mixed fraction, with the numerator and denominator
themselves written in `obase`.  For complex values each component is formatted
in that radix and a zero imaginary component is omitted.  `decimal()` always
prints positional notation in radix 10 regardless of `obase` or `format`.
`debug()` exposes the internal `C` representation and its two `Q` components.
`print expression` uses the current persistent `format`.

### Base 10 examples

In base 10, mixed fractions can be printed exactly while `positional()` shows
the same value in ordinary decimal notation:

```
base 10
fraction(2.625)
2'5/8

positional(2'5/8)
2.625

fraction(7/3)
2'1/3

positional(7/3)
2.(3)
```

### Base 12 examples

The same output functions follow `obase`.  With base 12 selected, both the
mixed-fraction components and positional digits are written in duodecimal:

```
base 12
fraction(2'5/8)
2'5/8

positional(2'5/8)
2.76

fraction(.49 + .03)
5/10

positional(.49 + .03)
0.5

decimal(.49 + .03)
0.41(6)
```

The `5/10` above is a base-12 fraction: denominator `10` means twelve.

`maxdigits` limits how far positional output searches or prints a
non-repeating prefix or repetend.  The default is 1024.

```
maxdigits = 200
maxdigits 200
```

`maxdigits` and `precision` are assigned using decimal input, regardless of
`ibase`.

There is deliberately no `bc`-style `scale` setting.  `Q` component arithmetic
is exact, so division is not truncated to a fixed number of fractional digits.

## Deliberate differences from GNU `bc`

The user interface is intentionally familiar, but `uge` is not a complete `bc`
implementation.

- All calculator values are `C` complex numbers with `Q` rational components.
  A zero imaginary component is suppressed in normal output.
- `i` is a built-in constant; forms such as `2i` and `1+2i` are accepted.
- Complex ordering is not defined; ordering and integer-style operations require
  real operands.
- Division is exact whenever the underlying rational component arithmetic is
  exact; there is no fixed-point `scale` setting.
- `maxdigits` affects positional rendering only; it does not change arithmetic.
- Scalar user-defined functions and `if`/`while`/`for` control flow are
  supported, including recursion and variables declared with `local`.  Arrays
  are deliberately not implemented.
- Only mathematical functions already supported by Uge are exposed; `-l` is a
  compatibility no-op rather than a separate GNU math library load.
- Base-setting values are always interpreted in decimal, deliberately avoiding
  the confusing self-referential `ibase` behavior of traditional calculators.

Use `quit` or `halt` to exit.  `help` prints a short reminder.
