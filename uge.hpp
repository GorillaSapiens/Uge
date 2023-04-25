#ifndef _INCLUDE_UGE_HPP_
#define _INCLUDE_UGE_HPP_

#include <stdint.h>

#include "integer.hpp"

/// A class to store and manipulate rational numbers.
///
/// In your best scottish accent, "it HUGE!, with a capital U!"
///
/// The number is stored as a sign, a whole number part,
/// and fractional numerator and denomenator parts.
///
/// Methods are given for simple arithmatic and comparison,
/// as well as printing and casting to/from standard types.
class Uge {
   private:
      int8_t sign;
      Integer whl;
      Integer num;
      Integer den;

      void simplify(void);

   public:
      Uge();
      Uge(const Uge &orig); // copy constructor
      Uge(int8_t s, Integer w, Integer n, Integer d);
      Uge(const char *p);
      Uge(double d);
      Uge(int64_t i);

      Uge& operator=(const Uge& other); // assignment operator

      Uge operator + (Uge const & obj) const;
      Uge operator - (Uge const & obj) const;
      Uge operator - () const;
      Uge operator * (Uge const & obj) const;
      Uge operator / (Uge const & obj) const;
      bool operator == (const Uge &other) const;
      bool operator != (const Uge &other) const;
      bool operator < (const Uge &other) const;
      bool operator > (const Uge &other) const;
      bool operator <= (const Uge &other) const;
      bool operator >= (const Uge &other) const;

      Uge& operator+=(const Uge& other);
      Uge& operator-=(const Uge& other);
      Uge& operator*=(const Uge& other);
      Uge& operator/=(const Uge& other);

      char *debu_print(char *buf, size_t buflen) const;
      char *frac_print(char *buf, size_t buflen) const;
      char *deci_print(char *buf, size_t buflen) const;

      explicit operator int64_t () const;
      explicit operator double() const;
      Uge abs(void) const;
      Uge floor(void) const;
      int sgn(void) const;
};

#endif // _INCLUDE_UGE_HPP_
