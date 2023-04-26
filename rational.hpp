#ifndef _INCLUDE_RATIONAL_HPP_
#define _INCLUDE_RATIONAL_HPP_

#include <stdint.h>

typedef __int128  int128_t;
typedef unsigned __int128 uint128_t;

/// A class to store and manipulate rational numbers.
///
/// The number is stored as a sign, a whole number part,
/// and fractional numerator and denomenator parts.
///
/// Methods are given for simple arithmatic and comparison,
/// as well as printing and casting to/from standard types.
class Rational {
   private:
      int8_t sign;
      uint128_t whl;
      uint128_t num;
      uint128_t den;

      void simplify(void);

   public:
      Rational();
      Rational(const Rational &orig); // copy constructor
      Rational(int8_t s, uint128_t w, uint128_t n, uint128_t d);
      Rational(const char *p);
      Rational(double d);
      Rational(int128_t d);

      Rational& operator=(const Rational& other); // assignment operator

      Rational operator + (Rational const & obj) const;
      Rational operator - (Rational const & obj) const;
      Rational operator - () const;
      Rational operator * (Rational const & obj) const;
      Rational operator / (Rational const & obj) const;
      bool operator == (const Rational &other) const;
      bool operator != (const Rational &other) const;
      bool operator < (const Rational &other) const;
      bool operator > (const Rational &other) const;
      bool operator <= (const Rational &other) const;
      bool operator >= (const Rational &other) const;

      // returns a pointer which must be free'd
      char *debu_print(void) const;

      // returns a pointer which must be free'd
      char *frac_print(void) const;

      // returns a pointer which must be free'd
      char *deci_print(void) const;

      explicit operator int128_t() const;
      explicit operator double() const;
      Rational abs(void) const;
      Rational floor(void) const;
      int sgn(void) const;
};

#endif // _INCLUDE_RATIONAL_HPP_
