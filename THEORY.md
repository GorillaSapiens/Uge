# Theory and representation

This package was created to fight some of the perceived problems with
floating point arithmetic. Floating point numbers as implemented on most
modern computers suffer from rounding errors. It is difficult for humans to
understand why a calculation that is mathematically exact can turn into a long
nearby positional value, but it does not have to be this way for rational
arithmetic.

Uge provides four mathematical numeric classes, `N`, `Z`, `Q`, and `C`, plus
an experimental numerical-estimate wrapper, `Ce`. The first four names are
motivated by familiar mathematical number-set notation; `Ce` deliberately is
not a number-set name. The relationships are worth stating precisely:

- `N` represents **ℕ₀**, the natural numbers including zero. Uge uses
  ℕ₀ = {0, 1, 2, ...}; `N` is the unsigned magnitude layer used by the
  higher-level types.
- `Z` represents **ℤ**, the signed integers, exactly. It is an `N` magnitude
  plus a sign boolean. `Z` is provided as a standalone library type and is not
  used by the calculator.
- `Q` represents **ℚ**, the signed rational numbers, exactly. `Q` uses `N`
  directly rather than being layered on `Z`.
- `C` is the complex layer inspired by **ℂ**. Its real and imaginary components
  are `Q` values, so ℚ + iℚ is exact and irrational components are represented
  by rational approximations when required.
- `Ce` wraps a `C` center with two nonnegative `Q` error bounds, one for the
  real component and one for the imaginary component. It is an experimental
  computational type intended to retain the uncertainty that `C` necessarily
  loses when an irrational or transcendental result is approximated. The
  calculator does not use `Ce` yet.

## `N`: arbitrary-precision natural numbers (ℕ₀)

`N` is an unsigned integer that expands as needed to contain any element of
ℕ₀ = {0, 1, 2, ...}. It includes zero but does not represent negative values.
It is the magnitude building block used by both `Z` and `Q`.

`N` works by maintaining an array of `uint16_t` and expanding or shrinking that
array as needed. Think of it like a school child's paper page: when a larger
number is needed, she simply writes another digit. Instead of working in base
10, `N` works internally in base 65536. Child's play for a computer.

## `Z`: arbitrary-precision signed integers (ℤ)

`Z` adds exactly one piece of information to `N`: a sign boolean. Its magnitude
is an `N`, so every value in ℤ can be represented without a fixed machine-word
limit. Zero is always canonicalized as nonnegative; there is no distinct
negative-zero value.

Addition and subtraction compare magnitudes when signs differ. Multiplication
and division operate on the `N` magnitudes and combine the signs. Signed
division truncates toward zero, and the remainder has the dividend's sign.
Bitwise `~`, `&`, `|`, and `^` use infinite-width two's-complement semantics;
right shift is an arithmetic shift. Integer powers accept a nonnegative `N`
exponent, and odd roots of negative values retain the negative sign while even
roots of negative values are rejected.

`Z` exists for completeness as the natural signed-integer partner to `N`. The
calculator does not use it. `Q` already stores its own sign, whole part,
numerator, and denominator, so `Q` continues to use `N` directly rather than
wrapping those components in `Z`.

## `Ce`: complex values with retained numerical error

`Ce` is deliberately different from `N`, `Z`, `Q`, and `C`: it is not meant to
name a mathematical set. It stores a `C` center plus two nonnegative rational
error bounds:

```text
C value
Q error     absolute real-component error
Q ierror    absolute imaginary-component error
```

An exact `Ce` has both bounds equal to zero. Exact rational/complex arithmetic
therefore stays exact. Approximation-producing operations compare their result
at the requested precision with a guarded-precision evaluation and retain the
difference as numerical uncertainty. Existing input uncertainty is propagated
exactly/conservatively through ordinary arithmetic. For nonlinear functions the
current experimental implementation uses guarded-precision evaluation plus a
padded sampling of the input error box; this is intended as a practical
numerical enclosure for experimentation, not as proof-certified interval
arithmetic. Exact special cases inherited from `C` remain exact when requested
and guarded evaluations agree.

The distinction is useful for values such as `sqrt(-2)`: its real center and
real error are exactly zero, while the imaginary component contains the
rational approximation to `sqrt(2)` and `ierror` records its uncertainty.
Likewise, multiplying two `Ce` approximations to `sqrt(3)` produces an interval
that contains exact `3`, without claiming that the rational center itself was
exactly `sqrt(3)`. `recenter()` can move an enclosure to a simpler center while
widening the error bounds, which is intended to support later rational
reconstruction experiments without turning an approximation into a false exact
result.

`Ce` exposes the same arithmetic, complex helpers, powers, and ordinary/π/τ/
degree trigonometric families as `C`. Discontinuous integer-like operations
(bitwise operations, modulo, and shifts) require exact inputs. Ordering,
`floor()`, and `sgn()` reject cases whose error intervals make the answer
ambiguous. Principal-branch operations reject uncertain rectangles that cross
known branch cuts or include singular points.

At this stage `Ce` is a library experiment. `uge` itself remains `C`-valued.

## `Q`: exact rational numbers (ℚ)

`Q` represents numbers as four quantities: a sign (positive or negative), a
whole number (an unsigned integer of type `N`), and a fractional part expressed
as a whole-number numerator and whole-number denominator (both unsigned
integers of type `N`).

This may seem odd compared with IEEE 754 and other standards, but it solves a
critical problem: common fractions do not suffer rounding merely because their
positional representation repeats.

Generally, addition, subtraction, multiplication, and division are carried out
as a grade-school treatment of fractions would suggest, including common
denominators where needed.

The advantage is especially visible with repeating positional representations.
In base 2, base 10, or any other integer base, every rational number either
terminates or has a repeating sequence. In base 10, `1/3` is written as
`0.333333...`, or more precisely by Uge as `0.(3)`, where parentheses mark the
repeating pattern. One eleventh is `0.(09)`.

The common fraction `1/5` is `0.2` in base 10 and terminates. In base 2 it is
`0.(0011)`, a repeating fraction. A fixed-size floating-point representation
must eventually round such a value. By retaining the numerator and denominator,
`Q` is independent of whether a particular radix gives the value a terminating
or repeating positional spelling.

Functions are provided for printing values both as fractions and in positional
formats. For repeating fractions, parentheses denote the repeating portion. For
example, `1/28` prints in base 10 as `0.03(571428)`. In some cases it would take
an exceptionally long time to discover or print the full repeating pattern; in
that case output stops and ends in `...`. No precision has been lost from the
underlying `Q`; only the textual expansion was curtailed.

## `C`: rational-component complex numbers

`C` represents complex values. A complex number has the form

```text
a + bi
```

where `a` is the real component, `b` is the imaginary component, and `i` is the
square root of -1. `C` stores both `a` and `b` as `Q` values. A real number is
therefore simply a `C` whose imaginary component is zero. Values in ℚ + iℚ are
represented exactly. A general element of ℂ whose components are irrational
requires rational approximation of those components.

Because both components are rational values, ordinary complex addition,
subtraction, multiplication, and division remain exact whenever the `Q`
operations used to perform them are exact. For example,

```text
1/3 + 2/7i
```

can be stored exactly. No floating-point representation is introduced merely
because a number has an imaginary component.

`C` also provides complex square root, exponential, logarithmic, power, and
trigonometric operations. These functions can produce irrational results, so,
just as with the corresponding `Q` functions, irrational components are
represented by rational approximations computed to a requested working
precision. The important distinction is that exact rational structure is
preserved whenever possible; floating point is not used as the underlying
representation.

Some complex functions are inherently multi-valued. Where a single answer is
required, `C` uses the conventional principal value. In particular, complex
square root and logarithm use their principal branches.

Complex numbers do not have a natural ordering. Comparisons such as less-than
and greater-than therefore only make sense in `C` when both operands are real.
Operations that are fundamentally integer or real operations likewise require
a zero imaginary component.

Printing a `C` suppresses an imaginary component when it is zero, so ordinary
real calculations still look like ordinary real calculations. When the
imaginary component is nonzero, it is printed using `i`. Thus `C` can be used
as a general numeric value without forcing users who are working only with real
numbers to see complex-number syntax.

## Exactness and irrational values

Some readers have commented that `Q` is flawed because it cannot truly
represent irrational numbers. This is true, but neither can a finite IEEE 754
value truly represent an irrational number. `Q` can, however, represent every
rational number exactly, including rational numbers whose positional
representation repeats in the chosen radix.

`C` extends the same representation model to the complex plane: its real and
imaginary components are `Q` values, exact when rational and rational
approximations when a transcendental operation requires approximation.

The normalized real sine/cosine core carries exact rational information across
the Pythagorean identity when doing so is numerically safe.  For example,
`costau(1/6)` is exactly `1/2`, so `sintau(1/6)` can be formed as the rational
approximation `sqrt(1 - (1/2)^2)`.  Because `Q::sqrt()` is deterministic at a
given precision, this is the same approximation relationship used by
`sqrt(3)`, and subsequent exact rational arithmetic can cancel it.  This is
numeric reuse, not symbolic algebra: no unevaluated radical is stored.

`Q::sqrt(precision)` scales its integer root calculation by `2^(2*precision)`
so that taking the square root retains approximately the requested number of
fractional binary bits.

This sums up `N`, `Z`, `Q`, and `C`. As with all things open source, the details can
be found in the code.
