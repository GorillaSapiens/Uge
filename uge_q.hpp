#ifndef _INCLUDE_UGE_Q_HPP_
#define _INCLUDE_UGE_Q_HPP_

#include <stdint.h>

#include "uge_z.hpp"

namespace uge {
   /// A class to store and manipulate rational numbers.
   ///
   /// In your best scottish accent, "it HUGE!, with a capital U!"
   ///
   /// The number is stored as a sign (the boolean pos),
   /// a whole number part, and fractional numerator and denomenator parts.
   ///
   /// Methods are given for simple arithmatic and comparison,
   /// as well as printing and casting to/from standard types.
   ///
   /// (NB: While this construct could store "negative zero",
   /// this is disallowed.  There would be no special meaning to
   /// such a construct.)
   class Q {
      private:
         bool pos;
         Z whl;
         Z num;
         Z den;

         void simplify(void);

      public:
         Q();
         Q(const Q &orig); // copy constructor
         Q(bool p, Z w, Z n, Z d);
         Q(const char *p);
         Q(double d);
         Q(int64_t i);

         Q& operator=(const Q& other); // assignment operator

         Q operator + () const;
         Q operator - () const;

         Q operator ~ () const; // beware, see note below

         Q operator + (Q const & obj) const;
         Q operator - (Q const & obj) const;
         Q operator * (Q const & obj) const;
         Q operator / (Q const & obj) const;
         Q operator % (Q const & obj) const;

         Q operator & (Q const & obj) const; // beware, see note below
         Q operator | (Q const & obj) const; // beware, see note below
         Q operator ^ (Q const & obj) const; // beware, see note below
         Q operator >> (int64_t bits) const; // beware, see note below
         Q operator << (int64_t bits) const; // beware, see note below

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
         Q& operator%=(const Q& other);

         Q& operator&=(const Q& other); // beware, see note below
         Q& operator|=(const Q& other); // beware, see note below
         Q& operator^=(const Q& other); // beware, see note below
         Q& operator>>= (int64_t bits); // beware, see note below
         Q& operator<<= (int64_t bits); // beware, see note below

         // returns pointer that must be free'd
         char *debu_print(void) const;

         // returns pointer that must be free'd
         char *frac_print(void) const;

         // returns pointer that must be free'd
         char *deci_print(uint64_t max = 1024) const;

         explicit operator int64_t () const;
         explicit operator double() const;
         Q abs(void) const;
         Q floor(void) const;
         int sgn(void) const;

	 // NB: sqrt is a rational approximation to the given precision
         Q sqrt(uint64_t precision) const;
         Q pow(const Q &power, uint64_t precision) const;
   };
};

// NB: bitwise operators ~, &, |, ^, <<, >> only work on whole numbers
// if used on anything else, they are first transformed to whole
// numbers.

#endif // _INCLUDE_UGE_Q_HPP_

// vim: expandtab:noai:ts=3:sw=3
