#ifndef _INCLUDE_UGE_C_HPP_
#define _INCLUDE_UGE_C_HPP_

#include <stdint.h>

#include "uge_q.hpp"

namespace uge {
   /// A class to store and manipulate complex numbers.
   ///
   /// Both the real and imaginary components are Q values.  Consequently,
   /// ordinary complex arithmetic remains exact whenever the component
   /// arithmetic is exact; transcendental functions return rational
   /// approximations in the two components.
   class C {
      private:
         Q re;
         Q im;

         const Q &require_real(void) const;

      public:
         C();
         C(const C &orig);
         C(const Q &real);
         C(const Q &real, const Q &imag);
         C(const char *p, uint64_t radix = 10);
         C(double d);
         C(int64_t i);

         C& operator=(const C& other);

         const Q &real(void) const;
         const Q &imag(void) const;
         bool is_real(void) const;
         bool is_zero(void) const;

         C operator + () const;
         C operator - () const;
         C operator ~ () const;

         C operator + (C const &obj) const;
         C operator - (C const &obj) const;
         C operator * (C const &obj) const;
         C operator / (C const &obj) const;
         C operator % (C const &obj) const;

         C operator & (C const &obj) const;
         C operator | (C const &obj) const;
         C operator ^ (C const &obj) const;
         C operator >> (int64_t bits) const;
         C operator << (int64_t bits) const;

         bool operator == (const C &other) const;
         bool operator != (const C &other) const;

         // Complex numbers have no natural ordering.  These operators are
         // provided for calculator compatibility and require real operands.
         bool operator < (const C &other) const;
         bool operator > (const C &other) const;
         bool operator <= (const C &other) const;
         bool operator >= (const C &other) const;

         C& operator+=(const C& other);
         C& operator-=(const C& other);
         C& operator*=(const C& other);
         C& operator/=(const C& other);
         C& operator%=(const C& other);

         C& operator&=(const C& other);
         C& operator|=(const C& other);
         C& operator^=(const C& other);
         C& operator>>=(int64_t bits);
         C& operator<<=(int64_t bits);

         // returns pointer that must be free'd
         char *debu_print(void) const;

         // returns pointer that must be free'd
         char *frac_print(uint64_t radix = 10) const;

         // returns pointer that must be free'd
         char *print(uint64_t radix = 10, uint64_t max = 1024) const;

         // decimal compatibility wrapper
         // returns pointer that must be free'd
         char *deci_print(uint64_t max = 1024) const;

         C conj(void) const;
         Q norm(void) const;
         Q abs(uint64_t precision) const;
         Q arg(uint64_t precision) const;

         // floor() and sgn() are defined only for real values.  They exist
         // mainly so a C-valued calculator can retain its current interface.
         C floor(void) const;
         int sgn(void) const;

         // Principal square root and principal logarithm.
         C sqrt(uint64_t precision) const;
         C ln(uint64_t precision) const;

         // exp(this), named e() to match Q and the uge calculator.
         C e(uint64_t precision) const;

         // Principal-value complex power.  Integral powers are performed by
         // exact repeated squaring before falling back to exp(power*ln(this)).
         C pow(const C &power, uint64_t precision) const;

         C sin(uint64_t precision) const;
         C cos(uint64_t precision) const;
         C tan(uint64_t precision) const;
         C atan(uint64_t precision) const;

         // atan2 is meaningful here only for real y and x; complex operands
         // are rejected rather than assigning a nonstandard interpretation.
         C atan2(const C &x, uint64_t precision) const;

         // pi/tau-normalized trig.  For complex z these mean sin(pi*z),
         // cos(pi*z), etc., while preserving Q's exact normalized handling
         // of the real component where possible.
         C sinpi(uint64_t precision) const;
         C cospi(uint64_t precision) const;
         C tanpi(uint64_t precision) const;
         C atanpi(uint64_t precision) const;
         C atan2pi(const C &x, uint64_t precision) const;

         C sintau(uint64_t precision) const;
         C costau(uint64_t precision) const;
         C tantau(uint64_t precision) const;
         C atantau(uint64_t precision) const;
         C atan2tau(const C &x, uint64_t precision) const;

         C sindeg(uint64_t precision) const;
         C cosdeg(uint64_t precision) const;
         C tandeg(uint64_t precision) const;
         C atandeg(uint64_t precision) const;
         C atan2deg(const C &x, uint64_t precision) const;

         static C pi(uint64_t precision);
         static C tau(uint64_t precision);
   };
};

#endif // _INCLUDE_UGE_C_HPP_

// vim: expandtab:noai:ts=3:sw=3
