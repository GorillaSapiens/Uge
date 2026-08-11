# Uge

A `bc`-like arbitrary-precision rational/complex calculator, backed by reusable C++ numeric classes.

> "It's 'Uge, with a capital U!"

## Why Uge?

Uge keeps rational arithmetic exact instead of first forcing values into a
floating-point or fixed-decimal representation.

A small base-12 example shows the difference. In base 12, `.49 + .03` is
exactly `.50` (the same value as `.5`):

```text
.49 = 57/144
.03 =  3/144
sum = 60/144 = 5/12 = .50 = .5
```

GNU `bc` instead produces `.4B`, which is `59/144`:

```text
$ bc
ibase=obase=12
.49+.03
.4B
```

Uge keeps the fractions exact:

```text
$ ./uge
uge exact rational/complex calculator
Copyright (C) GorillaSapiens; type 'help' for help.
base 12
.49+.03
0.5
```

Complex arithmetic is built in:

```text
sqrt(-1)
i
```

And normalized trigonometry preserves familiar exact results when possible:

```text
sin(pi/2)
1
```

## The `uge` calculator

`uge` is the main interactive face of the project. Its expression syntax and
command-line feel are intentionally familiar to GNU `bc` users, but every
calculator value is a `C`: a complex number whose real and imaginary components
are exact rational `Q` values. If the imaginary component is zero, it is simply
not printed, so ordinary real calculations still look ordinary.

Highlights include:

- exact rational and complex arithmetic;
- a built-in imaginary unit `i`, with input such as `2i`, `1+2i`, and `1/2i`;
- arbitrary input and output radices from base 2 through base 65536;
- exact terminating and repeating positional notation;
- independent `ibase` and `obase`, plus `base` to set both;
- `fraction()`, `positional()`, `decimal()`, and `debug()` output forms;
- arbitrary-precision `sqrt`, exponential, logarithmic, trigonometric, and
  inverse-trigonometric functions, including complex principal values where
  applicable;
- built-in constants `i`, `e`, `pi`, and `tau`;
- `sinpi`/`cospi`/`tanpi` and `sintau`/`costau`/`tantau` families, including
  inverse variants;
- `sindeg`/`cosdeg`/`tandeg` and corresponding inverse degree functions;
- `real()`, `imag()`, `conj()`, `norm()`, `abs()`, and `arg()` for complex
  values;
- a default normalized trig mode that lets expressions such as `sin(pi/2)`
  retain exact special-case results, with `trigmode direct` available for
  direct radian evaluation;
- configurable working `precision` for approximated transcendental results and
  `maxdigits` for positional output;
- editable command history and multiline expressions.

Ordinary rational/complex arithmetic remains exact whenever the operation is
rational. Transcendental results that cannot be represented exactly are stored
as rational approximations in the real and imaginary components; Uge does not
switch its underlying representation to floating point.

See [UGE.md](UGE.md) for the calculator language, commands, functions, radix
syntax, output formats, and GNU `bc` compatibility notes.

## The numeric classes

The calculator is backed by three progressively richer C++ numeric layers:

### `Z` -- arbitrary-precision integers

`Z` stores an unsigned integer in a growable array of 16-bit limbs. It expands
as needed rather than imposing a fixed machine-word limit.

### `Q` -- exact rational numbers

`Q` represents a signed rational value using `Z` integers for its whole part,
numerator, and denominator. Fractions therefore remain fractions through
ordinary arithmetic instead of being rounded merely because their positional
representation repeats.

`Q` also supplies roots and transcendental functions. Irrational results are
represented by rational approximations computed at a requested working
precision.

### `C` -- rational complex numbers

`C` contains two `Q` values: one real component and one imaginary component.
Addition, subtraction, multiplication, division, and other operations remain
exact whenever their component arithmetic is exact.

`C` also provides complex square root, exponential, logarithmic, power, and
trigonometric operations. Multi-valued complex operations use conventional
principal values where a single result is required.

A `C` with a zero imaginary component prints exactly like its real `Q` value,
which lets `C` serve as the calculator's universal numeric type without making
real-only calculations noisy.

For more detail about the representation and design rationale, see
[THEORY.md](THEORY.md).

## Building

A C++ compiler and `make` are sufficient:

```sh
make
```

This builds:

- `uge` -- the interactive calculator;
- `ztest` -- interactive/test driver for `Z`;
- `qtest` -- interactive/test driver for `Q`;
- `ctest` -- interactive/test driver for `C`.

Then run the calculator with:

```sh
./uge
```

The Makefile uses compiler-generated dependency files (`-MMD -MP`) rather than
machine-specific `makedepend` output.

## Source overview

```text
uge_z.hpp / uge_z.cpp       Z implementation
uge_q.hpp / uge_q.cpp       Q implementation
uge_c.hpp / uge_c.cpp       C implementation
uge.cpp                     interactive calculator
ztest.cpp                   Z test/interactive driver
qtest.cpp                   Q test/interactive driver
ctest.cpp                   C test/interactive driver
UGE.md                      calculator reference
THEORY.md                   representation and design rationale
```

Uge is open source; see [LICENSE](LICENSE).
