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

"print" (includes all details) and "prettyprint" (human readable)
methods are provided, as well as conversion to (double) and (int).

abs(), floor(), and sgn() methods are also provided.

noticiably absent are functions which may return irrational results:
trig functions, logs, powers, etc...

on modern GNU/Linux machines which support 128 bit integers, the
represented range is from +2^64 to -2^64, with 128 bits of precision.
on other machines without 128 bit integers, the represented range is
+2^32 to -2^32, with 64 bits of precision.
