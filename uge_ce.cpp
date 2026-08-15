#include <stdlib.h>

#include <string>

#include "gcstr.hpp"
#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "uge_ce.hpp"

using namespace uge;

namespace {
   struct Interval {
      Q lo;
      Q hi;

      Interval(const Q &l, const Q &h) : lo(l), hi(h) {}
   };

   static Q qmin(const Q &a, const Q &b) {
      return a < b ? a : b;
   }

   static Q qmax(const Q &a, const Q &b) {
      return a > b ? a : b;
   }

   static Q qmax4(const Q &a, const Q &b, const Q &c, const Q &d) {
      return qmax(qmax(a, b), qmax(c, d));
   }

   static Q qmin4(const Q &a, const Q &b, const Q &c, const Q &d) {
      return qmin(qmin(a, b), qmin(c, d));
   }

   static Q product_error(const Q &a, const Q &ea,
                          const Q &b, const Q &eb) {
      // If x=a+da and y=b+db, with |da|<=ea and |db|<=eb, then
      // |xy-ab| <= |a|eb + |b|ea + ea*eb.
      return a.abs() * eb + b.abs() * ea + ea * eb;
   }

   static Q abs_low(const Q &center, const Q &error) {
      Q low = center.abs() - error;
      return low.sgn() < 0 ? Q((int64_t)0) : low;
   }

   static Interval divide_by_positive_interval(const Q &center,
                                               const Q &error,
                                               const Q &dlo,
                                               const Q &dhi) {
      // Enclose (center +/- error) / [dlo,dhi], where 0 < dlo <= dhi.
      Q nlo = center - error;
      Q nhi = center + error;
      Q a = nlo / dlo;
      Q b = nlo / dhi;
      Q c = nhi / dlo;
      Q d = nhi / dhi;
      return Interval(qmin4(a, b, c, d), qmax4(a, b, c, d));
   }

   static Q interval_radius(const Q &center, const Interval &i) {
      return qmax((center - i.lo).abs(), (i.hi - center).abs());
   }
}

void Ce::require_nonnegative_error(const Q &q) {
   if (q.sgn() < 0) {
      throw(UGE_ERR("Ce error bound must be nonnegative"));
   }
}

Ce::Ce()
   : val(), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(const Ce &orig)
   : val(orig.val), err(orig.err), ierr(orig.ierr) {
}

Ce::Ce(const C &value)
   : val(value), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(const Q &real)
   : val(real), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(int64_t i)
   : val(i), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(const char *p, uint64_t radix)
   : val(p, radix), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(const C &value, const Q &error, const Q &imag_error)
   : val(value), err(error), ierr(imag_error) {
   require_nonnegative_error(err);
   require_nonnegative_error(ierr);
}

Ce& Ce::operator=(const Ce& other) {
   val = other.val;
   err = other.err;
   ierr = other.ierr;
   return *this;
}

const C &Ce::value(void) const {
   return val;
}

const Q &Ce::real(void) const {
   return val.real();
}

const Q &Ce::imag(void) const {
   return val.imag();
}

const Q &Ce::error(void) const {
   return err;
}

const Q &Ce::ierror(void) const {
   return ierr;
}

bool Ce::exact(void) const {
   return err.sgn() == 0 && ierr.sgn() == 0;
}

bool Ce::real_exact(void) const {
   return err.sgn() == 0;
}

bool Ce::imag_exact(void) const {
   return ierr.sgn() == 0;
}

bool Ce::is_real(void) const {
   // The value is definitely real only if the imaginary center and its
   // uncertainty are both zero.
   return val.imag().sgn() == 0 && ierr.sgn() == 0;
}

bool Ce::is_zero(void) const {
   // "Definitely exactly zero", not merely an interval containing zero.
   return exact() && val.is_zero();
}

bool Ce::contains(const C &candidate) const {
   return (candidate.real() - val.real()).abs() <= err &&
          (candidate.imag() - val.imag()).abs() <= ierr;
}

Ce Ce::recenter(const C &new_value) const {
   // Triangle inequality: widening by the center displacement guarantees
   // that every value enclosed before recentering remains enclosed after it.
   Q new_error = err + (new_value.real() - val.real()).abs();
   Q new_ierror = ierr + (new_value.imag() - val.imag()).abs();
   return Ce(new_value, new_error, new_ierror);
}

Ce Ce::operator + () const {
   return *this;
}

Ce Ce::operator - () const {
   return Ce(-val, err, ierr);
}

Ce Ce::operator + (Ce const &obj) const {
   return Ce(val + obj.val, err + obj.err, ierr + obj.ierr);
}

Ce Ce::operator - (Ce const &obj) const {
   return Ce(val - obj.val, err + obj.err, ierr + obj.ierr);
}

Ce Ce::operator * (Ce const &obj) const {
   const Q &a = val.real();
   const Q &b = val.imag();
   const Q &c = obj.val.real();
   const Q &d = obj.val.imag();

   Q re_error = product_error(a, err, c, obj.err) +
                product_error(b, ierr, d, obj.ierr);
   Q im_error = product_error(a, err, d, obj.ierr) +
                product_error(b, ierr, c, obj.err);

   return Ce(val * obj.val, re_error, im_error);
}

Ce Ce::operator / (Ce const &obj) const {
   const Q &c = obj.val.real();
   const Q &d = obj.val.imag();

   // Bound the denominator c^2+d^2 over the divisor's uncertainty box.
   Q clo = abs_low(c, obj.err);
   Q dlo = abs_low(d, obj.ierr);
   Q chi = c.abs() + obj.err;
   Q dhi = d.abs() + obj.ierr;
   Q den_lo = clo * clo + dlo * dlo;
   Q den_hi = chi * chi + dhi * dhi;

   if (den_lo.sgn() == 0) {
      throw(UGE_ERR("division by value whose error bounds include zero"));
   }

   Q center_den = c * c + d * d;
   C reciprocal(c / center_den, -d / center_den);

   Interval rr = divide_by_positive_interval(c, obj.err, den_lo, den_hi);
   Interval ri = divide_by_positive_interval(-d, obj.ierr, den_lo, den_hi);
   Q reciprocal_error = interval_radius(reciprocal.real(), rr);
   Q reciprocal_ierror = interval_radius(reciprocal.imag(), ri);

   return *this * Ce(reciprocal, reciprocal_error, reciprocal_ierror);
}

Ce& Ce::operator+=(const Ce& other) {
   *this = *this + other;
   return *this;
}

Ce& Ce::operator-=(const Ce& other) {
   *this = *this - other;
   return *this;
}

Ce& Ce::operator*=(const Ce& other) {
   *this = *this * other;
   return *this;
}

Ce& Ce::operator/=(const Ce& other) {
   *this = *this / other;
   return *this;
}

Ce Ce::conj(void) const {
   return Ce(val.conj(), err, ierr);
}

char *Ce::debu_print(void) const {
   return mprintf("[value=%s,error=%s,ierror=%s]",
                  GCSTR val.debu_print(),
                  GCSTR err.debu_print(),
                  GCSTR ierr.debu_print());
}

char *Ce::frac_print(uint64_t radix) const {
   return val.frac_print(radix);
}

char *Ce::print(uint64_t radix, uint64_t max) const {
   return val.print(radix, max);
}

char *Ce::deci_print(uint64_t max) const {
   return val.deci_print(max);
}

// vim: expandtab:noai:ts=3:sw=3
