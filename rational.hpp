#ifndef _INCLUDE_RATIONAL_HPP_
#define _INCLUDE_RATIONAL_HPP_

#include <stdint.h>

typedef __int128  int128_t;
typedef unsigned __int128 uint128_t;

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

      char *debu_print(char *buf, size_t buflen) const;
      char *frac_print(char *buf, size_t buflen) const;
      char *deci_print(char *buf, size_t buflen) const;

      explicit operator int128_t() const;
      explicit operator double() const;
      Rational abs(void) const;
      Rational floor(void) const;
      int sgn(void) const;
};

#endif // _INCLUDE_RATIONAL_HPP_
