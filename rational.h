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
      Rational operator + (Rational const & obj);
      Rational operator - (Rational const & obj);
      Rational operator * (Rational const & obj);
      Rational operator / (Rational const & obj);
      bool operator == (const Rational &other);
      bool operator != (const Rational &other);
      bool operator < (const Rational &other);
      bool operator > (const Rational &other);
      bool operator <= (const Rational &other);
      bool operator >= (const Rational &other);
      void print(void);
      void prettyprint(void);
      operator double() const;
};
