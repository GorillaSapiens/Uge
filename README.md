# Uge

A `bc`-like arbitrary-precision rational/complex calculator, backed by reusable C++ numeric classes.

> "It's 'Uge, with a capital U!"

## Why Uge?

Uge keeps exact information exact for as long as the mathematics allows it.
A familiar trigonometric identity makes the difference visible immediately:

```text
$ ./uge -q
sin(pi/6)
0.5
```

GNU `bc`'s math library numerically approximates both pi and the sine:

```text
$ bc -l
pi=a(1)*4
s(pi/6)
.49999999999999999999
```

Uge's default normalized trigonometry knows that `pi/6` is exactly one sixth
of its cached pi value, so the special angle reduces exactly to `1/2` rather
than preserving approximation noise.

The same distinction shows up in ordinary fractions. GNU `bc` converts `1/3`
to its finite working-scale decimal approximation, so multiplying that result
back by 3 does not recover exactly 1:

```text
$ bc
bc 1.08.2
Copyright 1991-1994, 1997, 1998, 2000, 2004, 2006, 2008, 2012-2018, 2024 Free Software Foundation, Inc.
This is free software with ABSOLUTELY NO WARRANTY.
For details type `warranty'.
1/3
.33333333333333333333
last * 3
.99999999999999999999
```

Uge retains `1/3` as an exact rational value. Its repeating positional output
is only a representation of that exact value, so multiplying `last` by 3
returns exactly 1:

```text
$ ./uge
uge g565f8bc14d05 exact rational/complex calculator
Copyright (C) 2026 GorillaSapiens.
This program comes with ABSOLUTELY NO WARRANTY; type 'warranty' for details.
This is free software; see LICENSE for copying conditions. Type 'help' for help.
using positional format; enter 'format fraction' for fraction format
1/3
0.(3)
last * 3
1
```

Arbitrary bases give another compact example. In base 12, `.49 + .03` is
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
$ ./uge -q
base 12
.49+.03
0.5
```

Complex arithmetic is built in too:

```text
sqrt(-1)
i
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

A C++ compiler, `make`, and Perl are sufficient:

```sh
make
```

This builds:

- `uge` -- the interactive calculator;
- `ntest` -- interactive/test driver for `N`;
- `ztest` -- interactive/test driver for `Z`;
- `qtest` -- interactive/test driver for `Q`;
- `ctest` -- interactive/test driver for `C`.

Then run:

```sh
./uge
```

The Makefile uses compiler-generated dependency files (`-MMD -MP`).

### Version identification

`gen_version_h.pl` generates `version.h` as part of building `uge`. The version
identifier is chosen in this order:

1. the GitHub Actions tag when building a tagged GitHub ref;
2. an exact local Git tag on `HEAD`;
3. `g` followed by the first 12 hexadecimal digits of the Git commit;
4. `d` followed by a UTC ISO-8601 timestamp when Git metadata is unavailable.

Thus normal release tags can use names such as `v0.1.0`, while untagged Git
builds look like `g0123456789ab` and unpacked source trees without Git metadata
look like `d2026-08-14T00:00:00Z`. The fallback prefixes deliberately do not
begin with `v` and are distinguishable from each other.

Show the compiled identifier with:

```sh
./uge -V
./uge --version
```

`version.h` is generated build output and is removed by `make clean`.

### Installation

The usual prefix and staging variables are supported:

```sh
make install
make install PREFIX="$HOME/.local"
make install DESTDIR=/tmp/package-root PREFIX=/usr
```

The executable is installed as `$(PREFIX)/bin/uge`. For staged package builds,
`DESTDIR` is prepended without becoming part of the installed prefix.

To remove it using the same prefix/staging values:

```sh
make uninstall
```

## Regression tests

Run the complete noninteractive suite with:

```sh
make test
```

The suite has separate layers:

- `tests/regression.cpp` directly exercises `N`, `Z`, `Q`, `C`, and experimental `Ce`, including
  deterministic randomized algebraic/property tests and radix round-trips over
  bases 2, 3, 10, 12, 16, 36, 37, 256, and 65536;
- `tests/uge_regression.sh` drives the calculator as a user would, covering
  exact arithmetic and trig, complex values, output reparsing, arbitrary
  radices, formats, variables, control flow, functions, `local` scope,
  recursion, CLI behavior, and expected-error paths;
- `tests/version_regression.sh` verifies the `v` tag, `g` commit, and `d` date
  version-selection paths plus `uge -V`;
- `tests/install_regression.sh` verifies `PREFIX`/`DESTDIR` installation and
  uninstallation using the built executable.

For memory/undefined-behavior instrumentation, run:

```sh
make sanitize
```

That rebuilds and runs the same suite with AddressSanitizer and
UndefinedBehaviorSanitizer enabled.

`.github/workflows/test.yml` runs the ordinary suite and the sanitizer suite on
GitHub, cross-compiles the Windows x86-64 executable with MinGW-w64, and then
runs that cross-built executable on a Windows runner. Tagged releases are gated
by the ordinary and sanitized suites as well.

The older `ntest`, `ztest`, `qtest`, `ctest`, and experimental `cetest` programs remain useful as
interactive low-level probes; they are not the automated regression suite.

## Releases

Pushing a tag whose name starts with `v` runs
[`.github/workflows/release.yml`](.github/workflows/release.yml). For example:

```sh
git tag v0.1.0
git push origin v0.1.0
```

The workflow first runs both the normal and sanitizer regression suites. If they
pass, it cross-compiles a self-contained Windows x86-64 executable with
MinGW-w64 and builds a statically linked Linux x86-64 executable. The exact
packaged Windows executable is then run on a Windows runner before publication.

The Windows build can be reproduced locally on a system with MinGW-w64 installed:

```sh
make clean
make uge.exe \
    CXX=x86_64-w64-mingw32-g++ \
    CXXFLAGS='-O3 -DNDEBUG -static -static-libgcc -static-libstdc++'
x86_64-w64-mingw32-objdump -p uge.exe
```

The Makefile automatically selects `.exe` when the compiler name contains
`mingw`; the CI workflow also supplies `EXEEXT=.exe` explicitly.
Only after all of those gates pass does the workflow publish a GitHub Release
containing:

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
commit, so it does not contain object files, generated `version.h`, or other
working-tree build output. Building that source regenerates the same tag-based
version when Git metadata is present; without Git metadata it uses the `d...`
date fallback described above.

## Source overview

```text
uge_n.hpp / uge_n.cpp       N implementation
uge_z.hpp / uge_z.cpp       Z implementation
uge_q.hpp / uge_q.cpp       Q implementation
uge_c.hpp / uge_c.cpp       C implementation
uge_ce.hpp / uge_ce.cpp     experimental C-with-error implementation
uge.cpp                     interactive calculator
gen_version_h.pl             build-time version.h generator
ntest.cpp                   N test/interactive driver
ztest.cpp                   Z test/interactive driver
qtest.cpp                   Q test/interactive driver
ctest.cpp                   C test/interactive driver
cetest.cpp                  Ce test/interactive driver
tests/regression.cpp        automated N/Z/Q/C/Ce API regression suite
tests/uge_regression.sh     automated calculator regression suite
tests/version_regression.sh automated version-selection regression
tests/install_regression.sh automated install/uninstall regression
UGE.md                      calculator reference
THEORY.md                   representation and design rationale
.github/workflows/test.yml     push/pull-request regression workflow
.github/workflows/release.yml  tagged-release build/package workflow
```

Uge is open source; see [LICENSE](LICENSE).
