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
abs(x)
floor(x)
sgn(x)
pow(x,y)
xor(x,y)
```

`sqrt()` and non-rational powers use `Q`'s rational approximation mechanism.
The approximation precision is controlled by the special `precision` value,
which is specified in bits and defaults to 256:

```
precision = 8192
```

Like base-setting commands, `precision` is always assigned using decimal input.

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

For example:

```
base 12
fraction(.49 + .03)
5/10

positional(.49 + .03)
0.5

decimal(.49 + .03)
0.41(6)
```

The `5/10` above is a base-12 fraction: denominator `10` means twelve.

`scale` is accepted as a `bc`-familiar name, but Uge does not use `bc`'s
fixed-point scale semantics.  Arithmetic remains exact.  In `uge`, `scale`
(and its clearer alias `maxdigits`) limits how far positional output searches
or prints a non-repeating prefix or repetend.  The default is 1024.

```
scale = 200
maxdigits 200
```

These configuration values are also assigned in decimal.

## Deliberate differences from GNU `bc`

The user interface is intentionally familiar, but `uge` is not a complete `bc`
implementation.

- All ordinary numeric values are exact `Q` rational numbers.
- Division is exact; it is not truncated to a decimal `scale`.
- `scale` therefore does not change arithmetic.
- General user-defined functions, arrays, control-flow statements, and the GNU
  math library are not implemented.
- Only mathematical functions already supported by Uge are exposed.
- Base-setting values are always interpreted in decimal, deliberately avoiding
  the confusing self-referential `ibase` behavior of traditional calculators.

Use `quit` or `halt` to exit.  `help` prints a short reminder.
