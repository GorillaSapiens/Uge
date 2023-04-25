# RationalMath

A rational math package.

Because the ins and outs of floating point numbers can be confusing to
some folks new to computer science.

This package works by representing numbers as

(sign) * ( whole + (numerator / denominator))

operators for the common functions of addition, subtraction,
multiplication, and division are provided.  fractions and denominators
are preserved throughout as much as possible.

various comparison operators (==, !=, >, <, >=, and <=) are provided.

"debu\_print" (includes all details), "frac\_print" (human readable
fraction), and "deci\_print" (human readable repeated decimal) methods
are provided, as well as conversion to (double) and (int).

abs(), floor(), and sgn() methods are also provided.

noticiably absent are functions which may return irrational results:
trig functions, logs, powers, etc...

on modern GNU/Linux machines which support 128 bit integers, the
represented range is from +2^128 to -2^128, with 256 bits of precision.
