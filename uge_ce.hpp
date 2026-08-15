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

      public:
         Ce();
         Ce(const Ce &orig);

         // Exact-value constructors: both error bounds are zero.
         Ce(const C &value);
         Ce(const Q &real);
         Ce(int64_t i);
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
         // This is intended for future rational reconstruction/simplification.
         Ce recenter(const C &new_value) const;

         Ce operator + () const;
         Ce operator - () const;

         Ce operator + (Ce const &obj) const;
         Ce operator - (Ce const &obj) const;
         Ce operator * (Ce const &obj) const;
         Ce operator / (Ce const &obj) const;

         Ce& operator+=(const Ce& other);
         Ce& operator-=(const Ce& other);
         Ce& operator*=(const Ce& other);
         Ce& operator/=(const Ce& other);

         Ce conj(void) const;

         // These print the center value.  debu_print() also shows the bounds.
         // Returned pointers must be free'd by the caller.
         char *debu_print(void) const;
         char *frac_print(uint64_t radix = 10) const;
         char *print(uint64_t radix = 10, uint64_t max = 1024) const;
         char *deci_print(uint64_t max = 1024) const;
   };
};

#endif // _INCLUDE_UGE_CE_HPP_

// vim: expandtab:noai:ts=3:sw=3
