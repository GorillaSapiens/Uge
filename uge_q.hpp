#ifndef _INCLUDE_UGE_Q_HPP_
#define _INCLUDE_UGE_Q_HPP_

#include <stdint.h>

#include "uge_z.hpp"

namespace uge {
   /// A class to store and manipulate rational numbers.
   ///
   /// In your best scottish accent, "it HUGE!, with a capital U!"
   ///
   /// The number is stored as a sign, a whole number part,
   /// and fractional numerator and denomenator parts.
   ///
   /// Methods are given for simple arithmatic and comparison,
   /// as well as printing and casting to/from standard types.
   class Q {
      private:
         int8_t sign;
         Z whl;
         Z num;
         Z den;

         void simplify(void);

      public:
         Q();
         Q(const Q &orig); // copy constructor
         Q(int8_t s, Z w, Z n, Z d);
         Q(const char *p);
         Q(double d);
         Q(int64_t i);

         Q& operator=(const Q& other); // assignment operator

         Q operator + (Q const & obj) const;
         Q operator - (Q const & obj) const;
         Q operator - () const;
         Q operator * (Q const & obj) const;
         Q operator / (Q const & obj) const;
         bool operator == (const Q &other) const;
         bool operator != (const Q &other) const;
         bool operator < (const Q &other) const;
         bool operator > (const Q &other) const;
         bool operator <= (const Q &other) const;
         bool operator >= (const Q &other) const;

         Q& operator+=(const Q& other);
         Q& operator-=(const Q& other);
         Q& operator*=(const Q& other);
         Q& operator/=(const Q& other);

         // returns pointer that must be free'd
         char *debu_print(void) const;

         // returns pointer that must be free'd
         char *frac_print(void) const;

         // returns pointer that must be free'd
         char *deci_print(void) const;

         explicit operator int64_t () const;
         explicit operator double() const;
         Q abs(void) const;
         Q floor(void) const;
         int sgn(void) const;
         Q sqrt(void) const;
   };
};

#endif // _INCLUDE_UGE_Q_HPP_
