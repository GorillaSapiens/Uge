# Uge

A big and little numbers package, with an interactive exact rational/complex calculator.

> "It's 'Uge, with a capital U!"

Uge was written to avoid many of the surprises that come from representing
numbers with floating point. Its numeric types are layered:

- `Z` provides arbitrary-size unsigned integers.
- `Q` represents exact rational numbers using `Z`:
  ```
  (sign) * (whole + numerator / denominator)
  ```
- `C` represents complex numbers as a pair of `Q` values:
  ```
  real + imaginary * i
  ```

Fractions are preserved through ordinary arithmetic instead of being rounded to
floating-point or fixed-decimal approximations. Complex values likewise retain
exact rational real and imaginary components whenever the operation permits it.
Arbitrary-radix input and output are supported from base 2 through base 65536.

## The `uge` calculator

The repository also includes `uge`, an interactive calculator built on `C`.
Its language and user interface are intentionally familiar to GNU `bc` users.
Every calculator value is a complex number whose real and imaginary components
are exact `Q` rationals; values with a zero imaginary component print exactly as
they did when the calculator was `Q`-only.

For example, non-decimal fractions are converted directly to exact rationals:

```
$ ./uge
uge exact rational/complex calculator
Copyright (C) GorillaSapiens; type 'help' for help.
base 12
.49 + .03
0.5
```

The calculator includes:

- variables, assignments, expression parsing, and `bc`-style operators;
- exact complex arithmetic with built-in `i`, including forms such as `2i` and
  `1+2i`;
- editable command history with the Up/Down arrow keys;
- independent `ibase` and `obase`, plus `base` to set both at once;
- radices from 2 through 65536, including exact repeating positional notation;
- positional, fractional, decimal, and debug output forms;
- `sqrt`, `sin`, `cos`, `tan`, `atan`, `atan2`, `ln`, and `e(x)`, including
  complex principal values where applicable;
- `real`, `imag`, `conj`, `norm`, `abs`, and `arg` for complex values;
- built-in `e`, `pi`, and `tau` values;
- `sinpi`/`cospi`/`tanpi` and `sintau`/`costau`/`tantau` families for normalized
  angular arguments, including their inverse variants;
- `sindeg`/`cosdeg`/`tandeg` and corresponding inverse degree functions;
- a default normalized trig evaluation mode that preserves exact results such
  as `sin(pi/2) == 1` when possible, with `trigmode direct` available to call
  the underlying complex/rational radian approximations directly;
- configurable working `precision` for non-rational approximations and
  `maxdigits` for positional output.

Transcendental results are represented by rational approximations in the real
and imaginary components without introducing a floating-point type. Ordinary
complex arithmetic remains exact whenever its `Q` component operations are exact.

See [UGE.md](UGE.md) for the calculator language, commands, functions, radix
syntax, output formats, and differences from GNU `bc`.

## Library interface

`Z`, `Q`, and `C` provide progressively richer numeric layers.

`Z` supplies arbitrary-size integer magnitude arithmetic.

`Q` supplies exact signed rational arithmetic and operations such as `abs()`,
`floor()`, `sgn()`, powers, roots, and transcendental functions. When a
transcendental result is not rational, it is represented by a rational
approximation at the requested working precision.

`C` is built from two `Q` values, one real and one imaginary. Ordinary complex
addition, subtraction, multiplication, division, and integral powers therefore
remain exact when their component arithmetic is exact. `C` also provides
complex operations such as conjugation, norm, magnitude, argument, square root,
exponential, logarithm, powers, and trigonometric functions. Functions with
multiple complex values use the usual principal-value convention. Complex
transcendental results remain pairs of rational approximations rather than
introducing a floating-point representation.

When the imaginary component of a `C` value is zero, its normal textual output
omits the imaginary part, so real-valued complex results retain the familiar
`Q` representation.

Several textual representations are available, including detailed internal
output, exact fractions, and positional output with repeating digits. Values
may also be converted to ordinary C++ numeric types when desired.

Build the library tests and calculator with:

```
make
```

The current top-level Makefile produces `ztest`, `qtest`, `ctest`, and `uge`.
