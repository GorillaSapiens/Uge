#ifndef _INCLUDE_UGE_CE_HPP_
#define _INCLUDE_UGE_CE_HPP_

#include <stdint.h>

#include "uge_c.hpp"

namespace uge {
   /// A complex numeric estimate with independent componentwise error bounds.
   ///
   /// value is the rational-component complex center of the estimate.
   /// error and ierror are nonnegative absolute bounds on the real and
   /// imaginary components respectively:
   ///
   ///   |Re(actual) - Re(value)| <= error
   ///   |Im(actual) - Im(value)| <= ierror
   ///
   /// An exact Ce has both error bounds equal to zero.  Ce deliberately does
   /// not encode symbolic provenance; it only carries a numerical enclosure.
   class Ce {
      private:
         C val;
         Q err;
         Q ierr;

         static void require_nonnegative_error(const Q &q);
         void require_exact(const char *operation) const;
         void require_definitely_real(const char *operation) const;

      public:
         Ce();
         Ce(const Ce &orig);

         // Exact-value constructors: both error bounds are zero.
         Ce(const C &value);
         Ce(const Q &real);
         Ce(const Q &real, const Q &imag);
         Ce(int64_t i);
         Ce(double d);
         Ce(const char *p, uint64_t radix = 10);

         // Estimated-value constructor.  Both errors must be nonnegative.
         Ce(const C &value, const Q &error, const Q &ierror);

         Ce& operator=(const Ce& other);

         const C &value(void) const;
         const Q &real(void) const;
         const Q &imag(void) const;
         const Q &error(void) const;
         const Q &ierror(void) const;

         bool exact(void) const;
         bool real_exact(void) const;
         bool imag_exact(void) const;
         bool is_real(void) const;
         bool is_zero(void) const;

         // True if the supplied exact value lies inside this enclosure.
         bool contains(const C &candidate) const;

         // Move the center without shrinking the represented enclosure.
         // This is useful for rational reconstruction/simplification.
         Ce recenter(const C &new_value) const;

         // Recenter approximate components on a much simpler rational when a
         // continued-fraction convergent with a small denominator lies inside
         // the known componentwise error bound.  Error bounds are widened by
         // the center displacement; reconstruction never makes a Ce exact.
         Ce reconstruct(uint64_t max_denominator = 1000000) const;

         Ce operator + () const;
         Ce operator - () const;
         Ce operator ~ () const;

         Ce operator + (Ce const &obj) const;
         Ce operator - (Ce const &obj) const;
         Ce operator * (Ce const &obj) const;
         Ce operator / (Ce const &obj) const;
         Ce operator % (Ce const &obj) const;

         Ce operator & (Ce const &obj) const;
         Ce operator | (Ce const &obj) const;
         Ce operator ^ (Ce const &obj) const;
         Ce operator >> (int64_t bits) const;
         Ce operator << (int64_t bits) const;

         bool operator == (const Ce &other) const;
         bool operator != (const Ce &other) const;
         bool operator < (const Ce &other) const;
         bool operator > (const Ce &other) const;
         bool operator <= (const Ce &other) const;
         bool operator >= (const Ce &other) const;

         Ce& operator+=(const Ce& other);
         Ce& operator-=(const Ce& other);
         Ce& operator*=(const Ce& other);
         Ce& operator/=(const Ce& other);
         Ce& operator%=(const Ce& other);
         Ce& operator&=(const Ce& other);
         Ce& operator|=(const Ce& other);
         Ce& operator^=(const Ce& other);
         Ce& operator>>=(int64_t bits);
         Ce& operator<<=(int64_t bits);

         // These print the center value.  debu_print() also shows the bounds.
         // Returned pointers must be free'd by the caller.
         char *debu_print(void) const;
         char *frac_print(uint64_t radix = 10) const;
         char *print(uint64_t radix = 10, uint64_t max = 1024) const;
         char *deci_print(uint64_t max = 1024) const;

         Ce conj(void) const;
         Ce norm(void) const;
         Ce abs(uint64_t precision) const;
         Ce arg(uint64_t precision) const;

         Ce floor(void) const;
         int sgn(void) const;

         Ce sqrt(uint64_t precision) const;
         Ce ln(uint64_t precision) const;
         Ce e(uint64_t precision) const;
         Ce pow(const Ce &power, uint64_t precision) const;

         Ce sin(uint64_t precision) const;
         Ce cos(uint64_t precision) const;
         Ce tan(uint64_t precision) const;
         Ce atan(uint64_t precision) const;
         Ce atan2(const Ce &x, uint64_t precision) const;

         Ce sinpi(uint64_t precision) const;
         Ce cospi(uint64_t precision) const;
         Ce tanpi(uint64_t precision) const;
         Ce atanpi(uint64_t precision) const;
         Ce atan2pi(const Ce &x, uint64_t precision) const;

         Ce sintau(uint64_t precision) const;
         Ce costau(uint64_t precision) const;
         Ce tantau(uint64_t precision) const;
         Ce atantau(uint64_t precision) const;
         Ce atan2tau(const Ce &x, uint64_t precision) const;

         Ce sindeg(uint64_t precision) const;
         Ce cosdeg(uint64_t precision) const;
         Ce tandeg(uint64_t precision) const;
         Ce atandeg(uint64_t precision) const;
         Ce atan2deg(const Ce &x, uint64_t precision) const;

         static Ce pi(uint64_t precision);
         static Ce tau(uint64_t precision);
   };
};

#endif // _INCLUDE_UGE_CE_HPP_

// vim: expandtab:noai:ts=3:sw=3
