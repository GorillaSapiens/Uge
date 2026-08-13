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
Copyright (C) 2026 GorillaSapiens.
This program comes with ABSOLUTELY NO WARRANTY; type 'warranty' for details.
This is free software; see LICENSE for copying conditions. Type 'help' for help.
using positional format; enter 'format fraction' for fraction format
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
- persistent `format positional` / `format fraction` output selection, with
  `-positional` / `-fraction` startup options and `fraction()`, `positional()`,
  `decimal()`, and `debug()` one-shot forms;
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
- `if`/`else`, `while`, and `for` control flow with braced multiline blocks;
- scalar `bc`-like `define` functions with parameters, `local` variables,
  `return`, and recursion (arrays are deliberately omitted);
- editable command history and multiline expressions/statements;
- an interactive GPL warranty notice, with `warranty` printing the GPLv3
  warranty disclaimer and limitation of liability.

Ordinary rational/complex arithmetic remains exact whenever the operation is
rational. Transcendental results that cannot be represented exactly are stored
as rational approximations in the real and imaginary components; Uge does not
switch its underlying representation to floating point.

See [UGE.md](UGE.md) for the calculator language, commands, functions, radix
syntax, output formats, and GNU `bc` compatibility notes.

## The numeric classes

The numeric library provides four C++ classes whose names echo familiar
mathematical number-set notation. The calculator itself continues to use the
`N` -> `Q` -> `C` path; `Z` is a standalone signed-integer wrapper provided for
completeness and is not used by the calculator.

- `N` -- **ℕ₀**: arbitrary-precision natural numbers including zero.
  `N` is the unsigned magnitude layer; Uge uses the convention
  ℕ₀ = {0, 1, 2, ...}, so zero is included and negative values are not.
- `Z` -- **ℤ**: arbitrary-precision signed integers. `Z` is represented as an
  `N` magnitude plus a sign boolean; zero is canonicalized as nonnegative.
- `Q` -- **ℚ**: exact signed rational numbers. `Q` keeps its own sign and
  rational structure directly on top of `N`; it does not depend on `Z`.
- `C` -- inspired by **ℂ**: complex numbers represented by two `Q` components.
  Values in ℚ + iℚ are represented exactly; irrational real or imaginary
  components are represented by rational approximations at the requested
  working precision.

### `N` -- arbitrary-precision natural numbers (ℕ₀)

`N` stores an element of ℕ₀ = {0, 1, 2, ...} in a growable array of
16-bit limbs. It expands as needed rather than imposing a fixed machine-word
limit. `N` is deliberately unsigned and serves as the magnitude building block
for both `Z` and `Q`.

### `Z` -- arbitrary-precision signed integers (ℤ)

`Z` stores exactly two pieces of state: an `N` magnitude and a boolean sign.
Negative zero is not permitted; any zero magnitude is canonicalized as
nonnegative. Ordinary integer arithmetic is exact. Division truncates toward
zero and the remainder carries the dividend's sign, matching the usual
truncating signed-integer identity. Bitwise operations use infinite-width
two's-complement semantics, and right shift is arithmetic.

`Z` is intentionally independent of the calculator. `Q` already has a sign
and several `N` components, so routing `Q` through `Z` would add representation
without adding information.

### `Q` -- exact rational numbers (ℚ)

`Q` represents a signed rational value using `N` values for its whole part,
numerator, and denominator. Fractions therefore remain fractions through
ordinary arithmetic instead of being rounded merely because their positional
representation repeats.

`Q` represents every rational number exactly. It also supplies roots and
transcendental functions; irrational results are represented by rational
approximations computed at a requested working precision.

### `C` -- rational-component complex numbers

`C` contains two `Q` values: one real component and one imaginary component.
Addition, subtraction, multiplication, division, and other operations remain
exact whenever their component arithmetic is exact. In particular, values in
ℚ + iℚ are exact; general elements of ℂ with irrational components are
approximated component by component.

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
- `ntest` -- interactive/test driver for `N`;
- `ztest` -- interactive/test driver for `Z`;
- `qtest` -- interactive/test driver for `Q`;
- `ctest` -- interactive/test driver for `C`.

Then run the calculator with:

```sh
./uge
```

The Makefile uses compiler-generated dependency files (`-MMD -MP`) rather than
machine-specific `makedepend` output.

## Regression tests

Run the complete noninteractive regression suite with:

```sh
make test
```

The suite has two layers:

- `tests/regression.cpp` exercises the `N`, `Z`, `Q`, and `C` APIs directly,
  including exact arithmetic, comparisons, signed integer behavior, radix
  round-trips, normalized trigonometric special values, complex arithmetic,
  and expected domain errors;
- `tests/uge_regression.sh` drives the `uge` executable as a user would and
  checks calculator syntax and output, arbitrary radices, positional/fraction
  formats, exact trigonometric cases, complex values, variables, control flow,
  functions and `local` scope, recursion, diagnostics, help, and warranty text.

`.github/workflows/test.yml` runs `make test` automatically on branch pushes
and pull requests. Tagged releases run the same suite before either platform
binary is built, so a failing regression test prevents publication.

The older `ntest`, `ztest`, `qtest`, and `ctest` programs remain useful as
interactive low-level probes; they are not the automated regression suite.

## Releases

Pushing a tag whose name starts with `v` runs
[`.github/workflows/release.yml`](.github/workflows/release.yml). For example:

```sh
git tag v0.1.0
git push origin v0.1.0
```

The workflow first runs the full regression suite. If it passes, it
cross-compiles a self-contained Windows x86-64 executable with MinGW-w64,
builds a statically linked Linux x86-64 executable, and publishes a GitHub
Release containing:

```text
uge-0.1.0-linux-x86_64.tar.gz
uge-0.1.0-windows-x86_64.zip
SHA256SUMS
```

Both platform archives have the same layout at their root:

```text
bin/    precompiled executable (`uge` or `uge.exe`)
src/    complete tracked source tree from the release tag
```

The `src/` directory is produced with `git archive` from the exact tagged
commit, so it does not contain object files or other working-tree build output.

## Source overview

```text
uge_n.hpp / uge_n.cpp       N implementation
uge_z.hpp / uge_z.cpp       Z implementation
uge_q.hpp / uge_q.cpp       Q implementation
uge_c.hpp / uge_c.cpp       C implementation
uge.cpp                     interactive calculator
ntest.cpp                   N test/interactive driver
ztest.cpp                   Z test/interactive driver
qtest.cpp                   Q test/interactive driver
ctest.cpp                   C test/interactive driver
tests/regression.cpp        automated N/Z/Q/C API regression suite
tests/uge_regression.sh     automated calculator regression suite
UGE.md                      calculator reference
THEORY.md                   representation and design rationale
.github/workflows/test.yml     push/pull-request regression workflow
.github/workflows/release.yml  tagged-release build/package workflow
```

Uge is open source; see [LICENSE](LICENSE).
