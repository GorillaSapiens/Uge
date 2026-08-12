# Theory and representation

This package was created to fight some of the perceived problems with
floating point arithmetic. Floating point numbers as implemented on most
modern computers suffer from rounding errors. It is difficult for humans to
understand why a calculation that is mathematically exact can turn into a long
nearby positional value, but it does not have to be this way for rational
arithmetic.

Uge introduces three classes: `Z`, `Q`, and `C`. Their names are motivated by
familiar mathematical number-set notation, but the correspondence is worth
stating precisely:

- `Z` represents **ℤ≥0**, the nonnegative integers, rather than all of ℤ. It is
  the unsigned magnitude layer used by the higher-level types.
- `Q` represents **ℚ**, the signed rational numbers, exactly.
- `C` is the complex layer inspired by **ℂ**. Its real and imaginary components
  are `Q` values, so ℚ + iℚ is exact and irrational components are represented
  by rational approximations when required.

## `Z`: arbitrary-precision nonnegative integers (ℤ≥0)

`Z` is an unsigned integer that expands as needed to contain any nonnegative
integer. It includes zero but does not represent negative values; sign is added
by `Q` rather than stored in the magnitude layer.

`Z` works by maintaining an array of `uint16_t` and expanding or shrinking that
array as needed. Think of it like a school child's paper page: when a larger
number is needed, she simply writes another digit. Instead of working in base
10, `Z` works internally in base 65536. Child's play for a computer.

## `Q`: exact rational numbers (ℚ)

`Q` represents numbers as four quantities: a sign (positive or negative), a
whole number (an unsigned integer of type `Z`), and a fractional part expressed
as a whole-number numerator and whole-number denominator (both unsigned
integers of type `Z`).

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

This sums up `Z`, `Q`, and `C`. As with all things open source, the details can
be found in the code.
