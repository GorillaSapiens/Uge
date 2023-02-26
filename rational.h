#ifndef _INCLUDE_RATIONAL_H_
#define _INCLUDE_RATIONAL_H_
// in development, so we're a little loosey goosey here...

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>

#if __GNUC__

typedef __int128 sBIG_t;
typedef unsigned __int128 uBIG_t;

typedef int64_t sREG_t;
typedef uint64_t uREG_t;

#else

typedef int64_t sBIG_t;
typedef uint64_t uBIG_t;

typedef int32_t sREG_t;
typedef uint32_t uREG_t;

#endif

class Rational {
   private:
      int8_t sign;
      uREG_t whl;
      uREG_t num;
      uREG_t den;

      void simplify(void);

   public:
      Rational();
      Rational(const Rational &orig);
      Rational(int8_t s, uREG_t w, uREG_t n, uREG_t d);
      Rational(const char *p);

      Rational operator + (Rational const & obj) const;
      Rational operator - (Rational const & obj) const;
      Rational operator * (Rational const & obj) const;
      Rational operator / (Rational const & obj) const;
      bool operator == (const Rational &other) const;
      bool operator != (const Rational &other) const;
      bool operator < (const Rational &other) const;
      bool operator > (const Rational &other) const;
      bool operator <= (const Rational &other) const;
      bool operator >= (const Rational &other) const;

      void print(char *buf, size_t buflen) const;
      void shortprint(char *buf, size_t buflen) const;

      explicit operator sREG_t() const;
      explicit operator double() const;
      Rational abs(void) const;
      Rational floor(void) const;
      int sgn(void) const;
};

#endif // _INCLUDE_RATIONAL_H_
