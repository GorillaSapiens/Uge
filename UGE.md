# `uge` calculator

`uge` is an interactive exact-rational calculator built on Uge's `Q` type.
Its command language intentionally resembles GNU `bc`, but calculations remain
exact rational values instead of decimal fixed-point values.

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

This builds `uge`, `ztest`, and `qtest`.

Run the calculator with:

```
./uge
```

`uge -q` suppresses the interactive banner.  `uge -l` is accepted for GNU
`bc` command-line compatibility; there is no separate math library to load.
One or more file names may also be given.  They are evaluated in order before
interactive input is read.

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

`~` and the bitwise operators below operate on the whole-number part, matching
the underlying `Q` operations:

```
&  |  <<  >>
```

Because `^` is used for `bc`-compatible exponentiation, bitwise exclusive-or is
available as the function `xor(a,b)`.

Comparisons produce 0 or 1:

```
==  !=  <  <=  >  >=
```

Logical AND and OR are also supported:

```
&&  ||
```

Parentheses follow the normal rules of precedence.  Statements may be separated
with semicolons.  An expression may continue on another line while parentheses
remain open, or when the line ends with `\`.

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

## Functions

Functions implemented by the current `Q` type are exposed directly:

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
e(x)
pi
pi()
tau
tau()
abs(x)
floor(x)
sgn(x)
pow(x,y)
xor(x,y)
```

The ordinary trig functions use radians.  `atan2(y,x)` is quadrant-aware and
returns an angle in `(-pi,pi]`; `atan2(0,0)` is undefined.

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

`direct` mode instead calls the ordinary `Q::sin()`, `Q::cos()`, `Q::tan()`,
`Q::atan()`, and `Q::atan2()` implementations directly on their rational
radian arguments.  Because the cached `pi` is a rational approximation, the
two modes can differ very slightly for general inputs.  Both modes still take
ordinary `sin()`, `cos()`, and `tan()` arguments in radians.

The explicit `*pi`, `*tau`, and `*deg` functions are never affected by
`trigmode`.  They always retain the meanings described below.  Enter
`trigmode` by itself to display the current setting.

The `*pi` variants measure angles in half-turns: `sinpi(x)` means `sin(pi*x)`,
while `atanpi(x)` and `atan2pi(y,x)` return their answers divided by pi.  The
`*tau` variants measure angles in turns: `sintau(x)` means `sin(tau*x)`, while
`atantau(x)` and `atan2tau(y,x)` return fractions of a full turn.  `atan2pi()`
returns in `(-1,1]` and `atan2tau()` returns in `(-1/2,1/2]`.  Normalized
arguments are reduced exactly as rational numbers before any approximation is
introduced.  Consequently important values such as these are exact `Q`s:

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

This preserves exact degree results whenever the underlying turn-normalized
function has an exact rational result:

```
sindeg(30)      -> 0.5
cosdeg(60)      -> 0.5
tandeg(45)      -> 1
atandeg(1)      -> 45
atan2deg(1,0)   -> 90
```

`pi` and `tau` are built-in values and may also be written `pi()` and `tau()`.
`tau` is derived exactly as twice the cached approximation of `pi`.  `e(x)`
means e raised to the power x, so `e(1)` gives Euler's constant and `e(0)` is 1.

`sqrt()`, non-rational powers, trigonometric functions, `ln()`, `e()`, and
`pi` use `Q`'s rational approximation mechanism.  No floating-point type is
introduced: the returned value is a rational approximation represented by a
`Q`.  The approximation precision is controlled by the special `precision`
value, which controls the working precision used when Uge must approximate a
non-rational result.  It defaults to 256:

```
precision = 8192
```

`precision` is always assigned using decimal input, regardless of `ibase`.

The calculator caches `pi` and `e(1)` at the current `precision`.  Both values
are computed once at startup and recomputed only when `precision` changes.
Changing `maxdigits` does not invalidate the cache because it affects only
output formatting.

The transcendental functions are implemented without floating point.  `sin()`,
`cos()`, `atan()`, `ln()`, and `e()` use rational series with range reduction;
`pi` uses Machin's arctangent formula; `tan()` is formed from sine and cosine;
and `atan2()` applies the standard quadrant corrections to arctangent.  During
these approximate calculations, intermediate values are truncated to a binary
working-precision grid to keep their exact rational denominators from growing
without useful bound.

## Output forms

The ordinary output of `uge` is **positional notation** in `obase`.  "Decimal
notation" is the special case where the radix is 10; it is not the right name
for the same notation in another radix.

Several top-level output forms are provided:

```
positional(expression)
pos(expression)
fraction(expression)
frac(expression)
decimal(expression)
debug(expression)
print expression
```

`positional()` and `pos()` print in `obase`, just like ordinary expression
output.  `fraction()` and `frac()` print the exact value as an integer, fraction,
or mixed fraction, with the numerator and denominator themselves written in
`obase`.  `decimal()` always prints positional notation in radix 10 regardless
of `obase`.  `debug()` exposes the internal `Q` representation.

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

There is deliberately no `bc`-style `scale` setting.  `Q` arithmetic is exact,
so division is not truncated to a fixed number of fractional digits.

## Deliberate differences from GNU `bc`

The user interface is intentionally familiar, but `uge` is not a complete `bc`
implementation.

- All ordinary numeric values are exact `Q` rational numbers.
- Division is exact; there is no fixed-point `scale` setting.
- `maxdigits` affects positional rendering only; it does not change arithmetic.
- General user-defined functions, arrays, control-flow statements, and the GNU
  math library are not implemented.
- Only mathematical functions already supported by Uge are exposed.
- Base-setting values are always interpreted in decimal, deliberately avoiding
  the confusing self-referential `ibase` behavior of traditional calculators.

Use `quit` or `halt` to exit.  `help` prints a short reminder.
