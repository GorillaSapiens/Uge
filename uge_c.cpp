#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "gcstr.hpp"
#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "uge_c.hpp"

using namespace uge;

static uint64_t guarded_precision(uint64_t precision, uint64_t guard) {
   if (precision > (uint64_t)INT64_MAX ||
       guard > (uint64_t)INT64_MAX - precision) {
      throw(UGE_ERR("precision too large"));
   }
   return precision + guard;
}

static void hyperbolic(const Q &x, uint64_t precision, Q &s, Q &c) {
   Q ep = x.e(precision);
   Q en = (-x).e(precision);
   Q two((int64_t)2);
   s = (ep - en) / two;
   c = (ep + en) / two;
}

C::C()
   : re((int64_t)0), im((int64_t)0) {
}

C::C(const C &orig)
   : re(orig.re), im(orig.im) {
}

C::C(const Q &real)
   : re(real), im((int64_t)0) {
}

C::C(const Q &real, const Q &imag)
   : re(real), im(imag) {
}

C::C(const char *p, uint64_t radix)
   : re(p, radix), im((int64_t)0) {
}

C::C(double d)
   : re(d), im((int64_t)0) {
}

C::C(int64_t i)
   : re(i), im((int64_t)0) {
}

C& C::operator=(const C& other) {
   re = other.re;
   im = other.im;
   return *this;
}

const Q &C::real(void) const {
   return re;
}

const Q &C::imag(void) const {
   return im;
}

bool C::is_real(void) const {
   return im.sgn() == 0;
}

bool C::is_zero(void) const {
   return re.sgn() == 0 && im.sgn() == 0;
}

const Q &C::require_real(void) const {
   if (!is_real()) {
      throw(UGE_ERR("operation requires a real value"));
   }
   return re;
}

C C::operator + () const {
   return *this;
}

C C::operator - () const {
   return C(-re, -im);
}

C C::operator ~ () const {
   return C(~require_real());
}

C C::operator + (C const &obj) const {
   return C(re + obj.re, im + obj.im);
}

C C::operator - (C const &obj) const {
   return C(re - obj.re, im - obj.im);
}

C C::operator * (C const &obj) const {
   return C(re * obj.re - im * obj.im,
            re * obj.im + im * obj.re);
}

C C::operator / (C const &obj) const {
   Q d = obj.re * obj.re + obj.im * obj.im;
   if (d.sgn() == 0) {
      throw(UGE_ERR("division by zero"));
   }

   return C((re * obj.re + im * obj.im) / d,
            (im * obj.re - re * obj.im) / d);
}

C C::operator % (C const &obj) const {
   return C(require_real() % obj.require_real());
}

C C::operator & (C const &obj) const {
   return C(require_real() & obj.require_real());
}

C C::operator | (C const &obj) const {
   return C(require_real() | obj.require_real());
}

C C::operator ^ (C const &obj) const {
   return C(require_real() ^ obj.require_real());
}

C C::operator >> (int64_t bits) const {
   return C(require_real() >> bits);
}

C C::operator << (int64_t bits) const {
   return C(require_real() << bits);
}

bool C::operator == (const C &other) const {
   return re == other.re && im == other.im;
}

bool C::operator != (const C &other) const {
   return !(*this == other);
}

bool C::operator < (const C &other) const {
   return require_real() < other.require_real();
}

bool C::operator > (const C &other) const {
   return require_real() > other.require_real();
}

bool C::operator <= (const C &other) const {
   return require_real() <= other.require_real();
}

bool C::operator >= (const C &other) const {
   return require_real() >= other.require_real();
}

C& C::operator+=(const C& other) {
   *this = *this + other;
   return *this;
}

C& C::operator-=(const C& other) {
   *this = *this - other;
   return *this;
}

C& C::operator*=(const C& other) {
   *this = *this * other;
   return *this;
}

C& C::operator/=(const C& other) {
   *this = *this / other;
   return *this;
}

C& C::operator%=(const C& other) {
   *this = *this % other;
   return *this;
}

C& C::operator&=(const C& other) {
   *this = *this & other;
   return *this;
}

C& C::operator|=(const C& other) {
   *this = *this | other;
   return *this;
}

C& C::operator^=(const C& other) {
   *this = *this ^ other;
   return *this;
}

C& C::operator>>=(int64_t bits) {
   *this = *this >> bits;
   return *this;
}

C& C::operator<<=(int64_t bits) {
   *this = *this << bits;
   return *this;
}

char *C::debu_print(void) const {
   return mprintf("[%s,%si]", GCSTR re.debu_print(), GCSTR im.debu_print());
}

static char *complex_print(const Q &re, const Q &im, uint64_t radix,
                           uint64_t max, bool fraction) {
   if (im.sgn() == 0) {
      return fraction ? re.frac_print(radix) : re.print(radix, max);
   }

   Q one((int64_t)1);
   Q imagmag = im.abs();
   char *ret = NULL;

   if (re.sgn() != 0) {
      raprintf(ret, "%s", GCSTR (fraction ? re.frac_print(radix)
                                         : re.print(radix, max)));
      raprintf(ret, "%c", im.sgn() < 0 ? '-' : '+');
   }
   else if (im.sgn() < 0) {
      raprintf(ret, "-");
   }

   if (imagmag != one) {
      raprintf(ret, "%s", GCSTR (fraction ? imagmag.frac_print(radix)
                                          : imagmag.print(radix, max)));
   }
   raprintf(ret, "i");

   return ret;
}

char *C::frac_print(uint64_t radix) const {
   return complex_print(re, im, radix, 0, true);
}

char *C::print(uint64_t radix, uint64_t max) const {
   return complex_print(re, im, radix, max, false);
}

char *C::deci_print(uint64_t max) const {
   return print(10, max);
}

C C::conj(void) const {
   return C(re, -im);
}

Q C::norm(void) const {
   return re * re + im * im;
}

Q C::abs(uint64_t precision) const {
   return norm().sqrt(precision);
}

Q C::arg(uint64_t precision) const {
   if (is_zero()) {
      throw(UGE_ERR("argument undefined for zero"));
   }
   return im.atan2(re, precision);
}

C C::floor(void) const {
   return C(require_real().floor());
}

int C::sgn(void) const {
   return require_real().sgn();
}

C C::sqrt(uint64_t precision) const {
   if (im.sgn() == 0) {
      if (re.sgn() >= 0) {
         return C(re.sqrt(precision));
      }
      return C(Q((int64_t)0), (-re).sqrt(precision));
   }

   // Stable principal-square-root formula.  Avoid subtracting two nearly
   // equal approximate quantities when the result is close to an axis.
   uint64_t work = guarded_precision(precision, 16);
   Q mag = norm().sqrt(work);
   Q two((int64_t)2);

   if (re.sgn() >= 0) {
      Q x = ((mag + re) / two).sqrt(work);
      Q y = im / (two * x);
      return C(x, y);
   }

   Q y = ((mag - re) / two).sqrt(work);
   if (im.sgn() < 0) {
      y = -y;
   }
   Q x = im.abs() / (two * y.abs());
   return C(x, y);
}

C C::ln(uint64_t precision) const {
   if (is_zero()) {
      throw(UGE_ERR("logarithm of zero"));
   }

   uint64_t work = guarded_precision(precision, 16);

   // log(z) = 1/2 log(|z|^2) + i arg(z), principal branch.
   Q realpart = norm().ln(work) / Q((int64_t)2);
   Q imagpart = im.atan2(re, work);
   return C(realpart, imagpart);
}

C C::e(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.e(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q scale = re.e(work);
   return C(scale * im.cos(work), scale * im.sin(work));
}

static C integer_power(C base, Q power) {
   Q zero((int64_t)0);
   Q one((int64_t)1);
   Q two((int64_t)2);
   bool invert = power < zero;

   if (invert) {
      if (base.is_zero()) {
         throw(UGE_ERR("zero cannot be raised to a negative power"));
      }
      power = -power;
   }

   C ret(one);
   while (power.sgn() != 0) {
      if ((power % two).sgn() != 0) {
         ret *= base;
      }
      power = (power / two).floor();
      if (power.sgn() != 0) {
         base *= base;
      }
   }

   if (invert) {
      return C(one) / ret;
   }
   return ret;
}

C C::pow(const C &power, uint64_t precision) const {
   // Preserve exact arithmetic for every integral exponent, including
   // integral powers of genuinely complex rational values.
   if (power.im.sgn() == 0 && power.re == power.re.floor()) {
      return integer_power(*this, power.re);
   }

   // Preserve Q's exact/root-aware implementation where the operation stays
   // on the nonnegative real axis.
   if (is_real() && power.is_real() && re.sgn() >= 0) {
      if (power.re.sgn() < 0) {
         if (re.sgn() == 0) {
            throw(UGE_ERR("zero cannot be raised to a negative power"));
         }
         return C(Q((int64_t)1) / re.pow(-power.re, precision));
      }
      return C(re.pow(power.re, precision));
   }

   // Principal value: z^w = exp(w Log(z)).
   return (power * ln(guarded_precision(precision, 16))).e(precision);
}

C C::sin(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.sin(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im, work, sh, ch);
   return C(re.sin(work) * ch, re.cos(work) * sh);
}

C C::cos(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.cos(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im, work, sh, ch);
   return C(re.cos(work) * ch, -(re.sin(work) * sh));
}

C C::tan(uint64_t precision) const {
   uint64_t work = guarded_precision(precision, 16);
   return sin(work) / cos(work);
}

C C::atan(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.atan(precision));
   }

   // atan(z) = [Log(1+i*z) - Log(1-i*z)] / (2*i), principal value.
   uint64_t work = guarded_precision(precision, 24);
   C one(Q((int64_t)1));
   C i(Q((int64_t)0), Q((int64_t)1));
   C d = (one + i * *this).ln(work) - (one - i * *this).ln(work);
   return C(Q((int64_t)0), Q((int64_t)-1) / Q((int64_t)2)) * d;
}

C C::atan2(const C &x, uint64_t precision) const {
   return C(require_real().atan2(x.require_real(), precision));
}

C C::sinpi(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.sinpi(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im * Q::pi(work), work, sh, ch);
   return C(re.sinpi(work) * ch, re.cospi(work) * sh);
}

C C::cospi(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.cospi(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im * Q::pi(work), work, sh, ch);
   return C(re.cospi(work) * ch, -(re.sinpi(work) * sh));
}

C C::tanpi(uint64_t precision) const {
   uint64_t work = guarded_precision(precision, 16);
   return sinpi(work) / cospi(work);
}

C C::atanpi(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.atanpi(precision));
   }
   uint64_t work = guarded_precision(precision, 16);
   return atan(work) / C(Q::pi(work));
}

C C::atan2pi(const C &x, uint64_t precision) const {
   return C(require_real().atan2pi(x.require_real(), precision));
}

C C::sintau(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.sintau(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im * Q::tau(work), work, sh, ch);
   return C(re.sintau(work) * ch, re.costau(work) * sh);
}

C C::costau(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.costau(precision));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q sh, ch;
   hyperbolic(im * Q::tau(work), work, sh, ch);
   return C(re.costau(work) * ch, -(re.sintau(work) * sh));
}

C C::tantau(uint64_t precision) const {
   uint64_t work = guarded_precision(precision, 16);
   return sintau(work) / costau(work);
}

C C::atantau(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.atantau(precision));
   }
   uint64_t work = guarded_precision(precision, 16);
   return atan(work) / C(Q::tau(work));
}

C C::atan2tau(const C &x, uint64_t precision) const {
   return C(require_real().atan2tau(x.require_real(), precision));
}

C C::sindeg(uint64_t precision) const {
   return (*this / C(Q((int64_t)360))).sintau(precision);
}

C C::cosdeg(uint64_t precision) const {
   return (*this / C(Q((int64_t)360))).costau(precision);
}

C C::tandeg(uint64_t precision) const {
   return (*this / C(Q((int64_t)360))).tantau(precision);
}

C C::atandeg(uint64_t precision) const {
   if (im.sgn() == 0) {
      return C(re.atandeg(precision));
   }
   return atantau(precision) * C(Q((int64_t)360));
}

C C::atan2deg(const C &x, uint64_t precision) const {
   return C(require_real().atan2deg(x.require_real(), precision));
}

C C::pi(uint64_t precision) {
   return C(Q::pi(precision));
}

C C::tau(uint64_t precision) {
   return C(Q::tau(precision));
}

// vim: expandtab:noai:ts=3:sw=3
