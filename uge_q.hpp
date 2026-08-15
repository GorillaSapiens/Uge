#ifndef _INCLUDE_UGE_Q_HPP_
#define _INCLUDE_UGE_Q_HPP_

#include <stdint.h>

#include "uge_n.hpp"

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
         N whl;
         N num;
         N den;

         void simplify(void);

      public:
         Q();
         Q(const Q &orig); // copy constructor
         Q(bool p, N w, N n, N d);
         Q(const char *p, uint64_t radix = 10);
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
         char *frac_print(uint64_t radix = 10) const;

         // returns pointer that must be free'd
         char *print(uint64_t radix = 10, uint64_t max = 1024) const;

         // decimal compatibility wrapper
         // returns pointer that must be free'd
         char *deci_print(uint64_t max = 1024) const;

         explicit operator int64_t () const;
         explicit operator double() const;
         Q abs(void) const;
         Q floor(void) const;
         int sgn(void) const;

         // Return a simpler rational center, when one can be found inside
         // +/- radius using a continued-fraction convergent whose denominator
         // does not exceed max_denominator.  If no such simpler value exists,
         // return *this unchanged.  This is intended for approximate-value
         // reconstruction; exact arithmetic should not call it with a
         // nonzero radius.
         Q reconstruct(const Q &radius,
                       uint64_t max_denominator = 1000000) const;

         // These return rational approximations to the given precision.
         Q sqrt(uint64_t precision) const;
         Q pow(const Q &power, uint64_t precision) const;
         Q sin(uint64_t precision) const;
         Q cos(uint64_t precision) const;
         Q tan(uint64_t precision) const;
         Q atan(uint64_t precision) const;
         Q atan2(const Q &x, uint64_t precision) const;

         // Trigonometric variants whose arguments/results are normalized by
         // pi (half-turns) or tau (turns).  Exact rational argument reduction
         // and exact special values are used before approximation.
         Q sinpi(uint64_t precision) const;
         Q cospi(uint64_t precision) const;
         Q tanpi(uint64_t precision) const;
         Q atanpi(uint64_t precision) const;
         Q atan2pi(const Q &x, uint64_t precision) const;
         Q sintau(uint64_t precision) const;
         Q costau(uint64_t precision) const;
         Q tantau(uint64_t precision) const;
         Q atantau(uint64_t precision) const;
         Q atan2tau(const Q &x, uint64_t precision) const;

         // Degree convenience variants.  These are thin wrappers around the
         // tau-normalized functions: forward trig divides degrees by 360;
         // inverse trig multiplies turns by 360.
         Q sindeg(uint64_t precision) const;
         Q cosdeg(uint64_t precision) const;
         Q tandeg(uint64_t precision) const;
         Q atandeg(uint64_t precision) const;
         Q atan2deg(const Q &x, uint64_t precision) const;

         Q ln(uint64_t precision) const;
         Q e(uint64_t precision) const;
         static Q pi(uint64_t precision);
         static Q tau(uint64_t precision);
   };
};

// NB: bitwise operators ~, &, |, ^, <<, >> only work on whole numbers
// if used on anything else, they are first transformed to whole
// numbers.

#endif // _INCLUDE_UGE_Q_HPP_

// vim: expandtab:noai:ts=3:sw=3
