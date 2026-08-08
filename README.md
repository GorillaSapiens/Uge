# Uge

A big and little numbers package, with an interactive exact-rational calculator.

> "It's 'Uge, with a capital U!"

Uge was written to avoid many of the surprises that come from representing
numbers with floating point.  Its `Z` type provides arbitrary-size unsigned
integers, while its `Q` type represents values as exact rational numbers:

```
(sign) * (whole + numerator / denominator)
```

Fractions are preserved through ordinary arithmetic instead of being rounded to
floating-point or fixed-decimal approximations.  Arbitrary-radix input and
output are supported from base 2 through base 65536.

## The `uge` calculator

The repository also includes `uge`, an interactive calculator built on `Q`.
Its language and user interface are intentionally familiar to GNU `bc` users,
but ordinary arithmetic uses exact rational values rather than decimal
fixed-point arithmetic.

For example, non-decimal fractions are converted directly to exact rationals:

```
$ ./uge
uge exact rational calculator
Copyright (C) GorillaSapiens; type 'help' for help.
base 12
.49 + .03
0.5
```

The calculator includes:

- variables, assignments, expression parsing, and `bc`-style operators;
- editable command history with the Up/Down arrow keys;
- independent `ibase` and `obase`, plus `base` to set both at once;
- radices from 2 through 65536, including exact repeating positional notation;
- positional, fractional, decimal, and debug output forms;
- `sqrt`, `sin`, `cos`, `tan`, `atan`, `atan2`, `ln`, and `e(x)`;
- built-in `pi` and `tau` values;
- `sinpi`/`cospi`/`tanpi` and `sintau`/`costau`/`tantau` families for normalized
  angular arguments, including their inverse variants, plus degree convenience
  functions such as `sindeg` and `atan2deg`;
- a default normalized trig evaluation mode that preserves exact results such
  as `sin(pi/2) == 1` when possible, with `trigmode direct` available to call
  the underlying rational-radian approximations directly;
- configurable working `precision` for non-rational approximations and
  `maxdigits` for positional output.

Transcendental results are rational approximations computed without introducing
a floating-point type.  Ordinary rational arithmetic remains exact.

See [UGE.md](UGE.md) for the calculator language, commands, functions, radix
syntax, output formats, and differences from GNU `bc`.

## Library interface

`Z` and `Q` provide the usual arithmetic and comparison operators.  `Q` also
provides operations such as `abs()`, `floor()`, `sgn()`, powers, roots, and the
transcendental functions used by the calculator.

Several textual representations are available, including detailed internal
output, exact fractions, and positional output with repeating digits.  Values
may also be converted to ordinary C++ numeric types when desired.

Build the library tests and calculator with:

```
make
```

This produces `ztest`, `qtest`, and `uge`.
