#include <stdlib.h>

#include <string>
#include <vector>

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

   static Q precision_epsilon(uint64_t precision) {
      if (precision > (uint64_t)INT64_MAX) {
         throw(UGE_ERR("precision too large"));
      }
      N d((uint64_t)1);
      d <<= (int64_t)precision;
      return Q(true, N((uint64_t)0), N((uint64_t)1), d);
   }

   static uint64_t guarded_precision(uint64_t precision, uint64_t guard) {
      if (precision > (uint64_t)INT64_MAX ||
          guard > (uint64_t)INT64_MAX - precision) {
         throw(UGE_ERR("precision too large"));
      }
      return precision + guard;
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

   static bool intervals_disjoint(const Q &ac, const Q &ae,
                                  const Q &bc, const Q &be) {
      return ac + ae < bc - be || bc + be < ac - ae;
   }

   static std::vector<C> sample_box(const Ce &x) {
      std::vector<Q> rs;
      std::vector<Q> is;
      if (x.error().sgn() != 0) {
         rs.push_back(x.error());
         rs.push_back(-x.error());
      }
      else {
         rs.push_back(Q((int64_t)0));
      }
      if (x.ierror().sgn() != 0) {
         is.push_back(x.ierror());
         is.push_back(-x.ierror());
      }
      else {
         is.push_back(Q((int64_t)0));
      }

      // Sample only the boundary corners/endpoints.  The center has already
      // been evaluated by the caller, so including the all-zero displacement
      // here merely repeats an expensive transcendental evaluation.
      std::vector<C> ret;
      for (size_t r = 0; r < rs.size(); r++) {
         for (size_t i = 0; i < is.size(); i++) {
            if (rs[r].sgn() == 0 && is[i].sgn() == 0) {
               continue;
            }
            ret.push_back(C(x.real() + rs[r], x.imag() + is[i]));
         }
      }
      return ret;
   }

   static Q numerical_error(const Q &center, uint64_t precision) {
      // C/Q transcendental routines quantize at the requested binary
      // precision.  Carry a deliberately generous numerical allowance rather
      // than recomputing every result at precision+32, which is extremely
      // expensive for arbitrary-precision trig.  Scale by result magnitude so
      // large transcendental values retain a useful relative cushion too.
      const Q cushion((int64_t)32);
      return precision_epsilon(precision) * cushion *
             (Q((int64_t)1) + center.abs());
   }

   static uint64_t probe_precision(uint64_t precision) {
      // A low-precision second evaluation is enough to recognize C's
      // precision-independent exact special cases.  For low requested
      // precisions, probe slightly above instead.
      if (precision > 32) {
         return 32;
      }
      return guarded_precision(precision, 8);
   }

   template <typename Eval>
   static Ce unary_estimate(const Ce &x, uint64_t precision, Eval eval) {
      C center = eval(x.value(), precision);

      // Exact special cases are precision-independent.  Detect them with a
      // cheap low-precision probe instead of an expensive precision+32 pass.
      bool re_stable = false;
      bool im_stable = false;
      if (x.exact()) {
         C probe = eval(x.value(), probe_precision(precision));
         re_stable = probe.real() == center.real();
         im_stable = probe.imag() == center.imag();
         if (re_stable && im_stable) {
            return Ce(center);
         }
      }

      Q re_error = re_stable ? Q((int64_t)0) :
                               numerical_error(center.real(), precision);
      Q im_error = im_stable ? Q((int64_t)0) :
                               numerical_error(center.imag(), precision);

      if (!x.exact()) {
         std::vector<C> samples = sample_box(x);
         Q observed_re((int64_t)0);
         Q observed_im((int64_t)0);
         for (size_t n = 0; n < samples.size(); n++) {
            C y = eval(samples[n], precision);
            observed_re = qmax(observed_re,
                               (y.real() - center.real()).abs());
            observed_im = qmax(observed_im,
                               (y.imag() - center.imag()).abs());
         }
         if (observed_re.sgn() != 0) {
            re_error += observed_re * Q((int64_t)2);
         }
         if (observed_im.sgn() != 0) {
            im_error += observed_im * Q((int64_t)2);
         }
      }

      return Ce(center, re_error, im_error);
   }

   template <typename Eval>
   static Ce unary_real_lipschitz(const Ce &x, uint64_t precision,
                                  const Q &factor, Eval eval) {
      if (!x.is_real()) {
         return unary_estimate(x, precision, eval);
      }

      C center = eval(x.value(), precision);
      if (x.exact()) {
         C probe = eval(x.value(), probe_precision(precision));
         if (probe == center) {
            return Ce(center);
         }
      }

      // For real inputs whose derivative magnitude is bounded by factor,
      // componentwise input uncertainty propagates directly without extra
      // transcendental evaluations at the interval endpoints.
      Q re_error = numerical_error(center.real(), precision) +
                   factor * x.error();
      Q im_error = numerical_error(center.imag(), precision);
      if (center.imag().sgn() == 0 && x.ierror().sgn() == 0) {
         im_error = Q((int64_t)0);
      }
      return Ce(center, re_error, im_error);
   }

   template <typename Eval>
   static Ce binary_estimate(const Ce &a, const Ce &b,
                             uint64_t precision, Eval eval) {
      C center = eval(a.value(), b.value(), precision);

      bool re_stable = false;
      bool im_stable = false;
      if (a.exact() && b.exact()) {
         C probe = eval(a.value(), b.value(), probe_precision(precision));
         re_stable = probe.real() == center.real();
         im_stable = probe.imag() == center.imag();
         if (re_stable && im_stable) {
            return Ce(center);
         }
      }

      Q re_error = re_stable ? Q((int64_t)0) :
                               numerical_error(center.real(), precision);
      Q im_error = im_stable ? Q((int64_t)0) :
                               numerical_error(center.imag(), precision);

      if (!a.exact() || !b.exact()) {
         std::vector<C> as = sample_box(a);
         std::vector<C> bs = sample_box(b);
         if (as.empty()) as.push_back(a.value());
         if (bs.empty()) bs.push_back(b.value());

         Q observed_re((int64_t)0);
         Q observed_im((int64_t)0);
         for (size_t i = 0; i < as.size(); i++) {
            for (size_t j = 0; j < bs.size(); j++) {
               C y = eval(as[i], bs[j], precision);
               observed_re = qmax(observed_re,
                                  (y.real() - center.real()).abs());
               observed_im = qmax(observed_im,
                                  (y.imag() - center.imag()).abs());
            }
         }
         if (observed_re.sgn() != 0) {
            re_error += observed_re * Q((int64_t)2);
         }
         if (observed_im.sgn() != 0) {
            im_error += observed_im * Q((int64_t)2);
         }
      }

      return Ce(center, re_error, im_error).reconstruct();
   }

   static Ce constant_estimate(uint64_t precision,
                               C (*eval)(uint64_t)) {
      C center = eval(precision);
      return Ce(center,
                numerical_error(center.real(), precision),
                center.imag().sgn() == 0 ? Q((int64_t)0) :
                                           numerical_error(center.imag(), precision));
   }

   static bool spans_zero(const Q &center, const Q &error) {
      return center - error <= Q((int64_t)0) &&
             center + error >= Q((int64_t)0);
   }

   static bool crosses_negative_real_axis(const Ce &x) {
      return !x.exact() &&
             x.real() - x.error() < Q((int64_t)0) &&
             spans_zero(x.imag(), x.ierror());
   }
}

void Ce::require_nonnegative_error(const Q &q) {
   if (q.sgn() < 0) {
      throw(UGE_ERR("Ce error bound must be nonnegative"));
   }
}

void Ce::require_exact(const char *operation) const {
   if (!exact()) {
      throw(UGE_ERR(std::string(operation) +
                    " requires an exact Ce value"));
   }
}

void Ce::require_definitely_real(const char *operation) const {
   (void)operation;
   if (!is_real()) {
      // Preserve C's public diagnostic for calculator compatibility.
      throw(UGE_ERR("operation requires a real value"));
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

Ce::Ce(const Q &real, const Q &imag)
   : val(real, imag), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(int64_t i)
   : val(i), err((int64_t)0), ierr((int64_t)0) {
}

Ce::Ce(double d)
   : val(d), err((int64_t)0), ierr((int64_t)0) {
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
   return val.imag().sgn() == 0 && ierr.sgn() == 0;
}

bool Ce::is_zero(void) const {
   return exact() && val.is_zero();
}

bool Ce::contains(const C &candidate) const {
   return (candidate.real() - val.real()).abs() <= err &&
          (candidate.imag() - val.imag()).abs() <= ierr;
}

Ce Ce::recenter(const C &new_value) const {
   Q new_error = err + (new_value.real() - val.real()).abs();
   Q new_ierror = ierr + (new_value.imag() - val.imag()).abs();
   return Ce(new_value, new_error, new_ierror);
}

Ce Ce::reconstruct(uint64_t max_denominator) const {
   if (exact()) {
      return *this;
   }

   Q new_real = real().reconstruct(err, max_denominator);
   Q new_imag = imag().reconstruct(ierr, max_denominator);
   if (new_real == real() && new_imag == imag()) {
      return *this;
   }
   return recenter(C(new_real, new_imag));
}

Ce Ce::operator + () const {
   return *this;
}

Ce Ce::operator - () const {
   return Ce(-val, err, ierr);
}

Ce Ce::operator ~ () const {
   require_exact("bitwise complement");
   return Ce(~val);
}

Ce Ce::operator + (Ce const &obj) const {
   return Ce(val + obj.val, err + obj.err, ierr + obj.ierr).reconstruct();
}

Ce Ce::operator - (Ce const &obj) const {
   return Ce(val - obj.val, err + obj.err, ierr + obj.ierr).reconstruct();
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

   return Ce(val * obj.val, re_error, im_error).reconstruct();
}

Ce Ce::operator / (Ce const &obj) const {
   const Q &c = obj.val.real();
   const Q &d = obj.val.imag();

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

Ce Ce::operator % (Ce const &obj) const {
   require_exact("modulo");
   obj.require_exact("modulo");
   return Ce(val % obj.val);
}

Ce Ce::operator & (Ce const &obj) const {
   require_exact("bitwise and");
   obj.require_exact("bitwise and");
   return Ce(val & obj.val);
}

Ce Ce::operator | (Ce const &obj) const {
   require_exact("bitwise or");
   obj.require_exact("bitwise or");
   return Ce(val | obj.val);
}

Ce Ce::operator ^ (Ce const &obj) const {
   require_exact("bitwise xor");
   obj.require_exact("bitwise xor");
   return Ce(val ^ obj.val);
}

Ce Ce::operator >> (int64_t bits) const {
   require_exact("right shift");
   return Ce(val >> bits);
}

Ce Ce::operator << (int64_t bits) const {
   require_exact("left shift");
   return Ce(val << bits);
}

bool Ce::operator == (const Ce &other) const {
   // Equality is reflexive for an identical numerical estimate.  This also
   // preserves identities such as pi == pi() without pretending that two
   // merely overlapping but different estimates are equal.
   if (val == other.val && err == other.err && ierr == other.ierr) {
      return true;
   }
   if (exact() && other.exact()) {
      return false;
   }
   if (intervals_disjoint(real(), err, other.real(), other.err) ||
       intervals_disjoint(imag(), ierr, other.imag(), other.ierr)) {
      return false;
   }
   throw(UGE_ERR("equality is ambiguous within Ce error bounds"));
}

bool Ce::operator != (const Ce &other) const {
   if (val == other.val && err == other.err && ierr == other.ierr) {
      return false;
   }
   if (exact() && other.exact()) {
      return true;
   }
   if (intervals_disjoint(real(), err, other.real(), other.err) ||
       intervals_disjoint(imag(), ierr, other.imag(), other.ierr)) {
      return true;
   }
   throw(UGE_ERR("inequality is ambiguous within Ce error bounds"));
}

bool Ce::operator < (const Ce &other) const {
   require_definitely_real("ordering");
   other.require_definitely_real("ordering");
   if (real() + err < other.real() - other.err) {
      return true;
   }
   if (real() - err >= other.real() + other.err) {
      return false;
   }
   throw(UGE_ERR("ordering is ambiguous within Ce error bounds"));
}

bool Ce::operator > (const Ce &other) const {
   return other < *this;
}

bool Ce::operator <= (const Ce &other) const {
   require_definitely_real("ordering");
   other.require_definitely_real("ordering");
   if (real() + err <= other.real() - other.err) {
      return true;
   }
   if (real() - err > other.real() + other.err) {
      return false;
   }
   throw(UGE_ERR("ordering is ambiguous within Ce error bounds"));
}

bool Ce::operator >= (const Ce &other) const {
   return other <= *this;
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

Ce& Ce::operator%=(const Ce& other) {
   *this = *this % other;
   return *this;
}

Ce& Ce::operator&=(const Ce& other) {
   *this = *this & other;
   return *this;
}

Ce& Ce::operator|=(const Ce& other) {
   *this = *this | other;
   return *this;
}

Ce& Ce::operator^=(const Ce& other) {
   *this = *this ^ other;
   return *this;
}

Ce& Ce::operator>>=(int64_t bits) {
   *this = *this >> bits;
   return *this;
}

Ce& Ce::operator<<=(int64_t bits) {
   *this = *this << bits;
   return *this;
}

Ce Ce::conj(void) const {
   return Ce(val.conj(), err, ierr);
}

Ce Ce::norm(void) const {
   Q re_error = product_error(real(), err, real(), err) +
                product_error(imag(), ierr, imag(), ierr);
   return Ce(C(val.norm()), re_error, Q((int64_t)0)).reconstruct();
}

Ce Ce::abs(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return C(z.abs(p)); });
}

Ce Ce::arg(uint64_t precision) const {
   if (!exact()) {
      Q rlo = abs_low(real(), err);
      Q ilo = abs_low(imag(), ierr);
      if ((rlo * rlo + ilo * ilo).sgn() == 0) {
         throw(UGE_ERR("argument undefined when Ce bounds include zero"));
      }
      if (crosses_negative_real_axis(*this)) {
         throw(UGE_ERR("argument bounds cross the principal branch cut"));
      }
   }
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return C(z.arg(p)); });
}

Ce Ce::floor(void) const {
   require_definitely_real("floor");
   Q lo = real() - err;
   Q hi = real() + err;
   Q flo = lo.floor();
   Q fhi = hi.floor();
   if (flo != fhi) {
      throw(UGE_ERR("floor is ambiguous within Ce error bounds"));
   }
   return Ce(C(flo));
}

int Ce::sgn(void) const {
   require_definitely_real("sgn");
   Q lo = real() - err;
   Q hi = real() + err;
   if (lo.sgn() > 0) {
      return 1;
   }
   if (hi.sgn() < 0) {
      return -1;
   }
   if (lo.sgn() == 0 && hi.sgn() == 0) {
      return 0;
   }
   throw(UGE_ERR("sign is ambiguous within Ce error bounds"));
}

Ce Ce::sqrt(uint64_t precision) const {
   if (crosses_negative_real_axis(*this)) {
      throw(UGE_ERR("square-root bounds cross the principal branch cut"));
   }
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.sqrt(p); });
}

Ce Ce::ln(uint64_t precision) const {
   if (!exact()) {
      Q rlo = abs_low(real(), err);
      Q ilo = abs_low(imag(), ierr);
      if ((rlo * rlo + ilo * ilo).sgn() == 0) {
         throw(UGE_ERR("logarithm undefined when Ce bounds include zero"));
      }
      if (crosses_negative_real_axis(*this)) {
         throw(UGE_ERR("logarithm bounds cross the principal branch cut"));
      }
   }
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.ln(p); });
}

Ce Ce::e(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.e(p); });
}

Ce Ce::pow(const Ce &power, uint64_t precision) const {
   if ((!exact() || !power.exact()) && crosses_negative_real_axis(*this)) {
      throw(UGE_ERR("power base bounds cross the principal branch cut"));
   }
   return binary_estimate(*this, power, precision,
      [](const C &z, const C &w, uint64_t p) { return z.pow(w, p); });
}

Ce Ce::sin(uint64_t precision) const {
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.sin(p); });
}

Ce Ce::cos(uint64_t precision) const {
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.cos(p); });
}

Ce Ce::tan(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.tan(p); });
}

Ce Ce::atan(uint64_t precision) const {
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.atan(p); });
}

Ce Ce::atan2(const Ce &x, uint64_t precision) const {
   require_definitely_real("atan2");
   x.require_definitely_real("atan2");
   if (!exact() || !x.exact()) {
      if (spans_zero(real(), err) && spans_zero(x.real(), x.err)) {
         throw(UGE_ERR("atan2 bounds include (0,0)"));
      }
   }
   return binary_estimate(*this, x, precision,
      [](const C &y, const C &xx, uint64_t p) { return y.atan2(xx, p); });
}

Ce Ce::sinpi(uint64_t precision) const {
   // |d sin(pi*x)/dx| <= pi < 4 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)4),
      [](const C &z, uint64_t p) { return z.sinpi(p); });
}

Ce Ce::cospi(uint64_t precision) const {
   // |d cos(pi*x)/dx| <= pi < 4 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)4),
      [](const C &z, uint64_t p) { return z.cospi(p); });
}

Ce Ce::tanpi(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.tanpi(p); });
}

Ce Ce::atanpi(uint64_t precision) const {
   // |d atan(x)/pi dx| < 1 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.atanpi(p); });
}

Ce Ce::atan2pi(const Ce &x, uint64_t precision) const {
   require_definitely_real("atan2pi");
   x.require_definitely_real("atan2pi");
   return binary_estimate(*this, x, precision,
      [](const C &y, const C &xx, uint64_t p) { return y.atan2pi(xx, p); });
}

Ce Ce::sintau(uint64_t precision) const {
   // |d sin(tau*x)/dx| <= tau < 7 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)7),
      [](const C &z, uint64_t p) { return z.sintau(p); });
}

Ce Ce::costau(uint64_t precision) const {
   // |d cos(tau*x)/dx| <= tau < 7 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)7),
      [](const C &z, uint64_t p) { return z.costau(p); });
}

Ce Ce::tantau(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.tantau(p); });
}

Ce Ce::atantau(uint64_t precision) const {
   // |d atan(x)/tau dx| < 1 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.atantau(p); });
}

Ce Ce::atan2tau(const Ce &x, uint64_t precision) const {
   require_definitely_real("atan2tau");
   x.require_definitely_real("atan2tau");
   return binary_estimate(*this, x, precision,
      [](const C &y, const C &xx, uint64_t p) { return y.atan2tau(xx, p); });
}

Ce Ce::sindeg(uint64_t precision) const {
   // pi/180 < 1, so 1 is a conservative real Lipschitz bound.
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.sindeg(p); });
}

Ce Ce::cosdeg(uint64_t precision) const {
   // pi/180 < 1, so 1 is a conservative real Lipschitz bound.
   return unary_real_lipschitz(*this, precision, Q((int64_t)1),
      [](const C &z, uint64_t p) { return z.cosdeg(p); });
}

Ce Ce::tandeg(uint64_t precision) const {
   return unary_estimate(*this, precision,
      [](const C &z, uint64_t p) { return z.tandeg(p); });
}

Ce Ce::atandeg(uint64_t precision) const {
   // 180/pi < 58 for real x.
   return unary_real_lipschitz(*this, precision, Q((int64_t)58),
      [](const C &z, uint64_t p) { return z.atandeg(p); });
}

Ce Ce::atan2deg(const Ce &x, uint64_t precision) const {
   require_definitely_real("atan2deg");
   x.require_definitely_real("atan2deg");
   return binary_estimate(*this, x, precision,
      [](const C &y, const C &xx, uint64_t p) { return y.atan2deg(xx, p); });
}

Ce Ce::pi(uint64_t precision) {
   return constant_estimate(precision, &C::pi);
}

Ce Ce::tau(uint64_t precision) {
   // Keep tau centered on exactly twice the same pi approximation and carry
   // exactly twice pi's uncertainty, matching the C/Q implementation.
   return Ce((int64_t)2) * Ce::pi(precision);
}

char *Ce::debu_print(void) const {
   return mprintf("[value=%s,error=%s,ierror=%s,exact=%s]",
                  GCSTR val.debu_print(),
                  GCSTR err.debu_print(),
                  GCSTR ierr.debu_print(),
                  exact() ? "true" : "false");
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
