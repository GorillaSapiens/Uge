#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "gcstr.hpp"
#include "uge_q.hpp"

using namespace uge;

// constant used for repeated fraction guess when parsing (double)
#define BIGPOWEROF2 512

static N gcd(N x, N y) {
   // euclid
   N a = x;
   N b = y;
   if (b.isZero()) {
      a = y;
      b = x;
   }
   if (b.isZero()) {
      throw (UGE_ERR("divide by zero in gcd"));
   }
   N c = a % b;
   while (!c.isZero()) {
      a = b;
      b = c;
      if (b.isZero()) {
         throw (UGE_ERR("divide by zero in gcd"));
      }
      c = a % b;
   }
   return b;
}

void Q::simplify(void) {
   if (num >= den) {
      whl += num / den;
      num %= den;
   }

   N g = gcd(num,den);
   num /= g;
   den /= g;

   if (whl.isZero() && num.isZero()) {
      pos = true;
   }
}

Q::Q() {
   pos = true;
   whl = (int) 0;
   num = (int) 0;
   den = 1;
}

Q::Q(const Q &orig) {
   pos = orig.pos;
   whl = orig.whl;
   num = orig.num;
   den = orig.den;
}

Q& Q::operator=(const Q& other) {
   pos = other.pos;
   whl = other.whl;
   num = other.num;
   den = other.den;

   return *this;
}

Q::Q(bool p, N w, N n, N d) {
   assert(!d.isZero());
   pos = p;
   whl = w;
   num = n;
   den = d;

   simplify();
}

static N llpow(N base, N n) {
   if (n.isZero()) {
      return 1;
   }
   else if (n == 1) {
      return base;
   }
   else {
      N a = n / 2;
      N b = n - a;
      return llpow(base, a) * llpow(base, b);
   }
}

static bool radix_digit(const char *&p, uint64_t radix, uint32_t &digit) {
   if (*p >= '0' && *p <= '9') {
      digit = *p - '0';
      p++;
   }
   else if (*p >= 'A' && *p <= 'Z') {
      digit = *p - 'A' + 10;
      p++;
   }
   else if (*p >= 'a' && *p <= 'z') {
      digit = *p - 'a' + 10;
      p++;
   }
   else if (*p == '{') {
      const char *q = p + 1;
      if (*q < '0' || *q > '9') {
         return false;
      }

      uint64_t value = 0;
      while (*q >= '0' && *q <= '9') {
         value = value * 10 + (*q - '0');
         if (value > 0xffff) {
            return false;
         }
         q++;
      }
      if (*q != '}') {
         return false;
      }

      digit = value;
      p = q + 1;
   }
   else {
      return false;
   }

   return digit < radix;
}

static uint64_t radix_digits(const char *p, uint64_t radix) {
   uint64_t ret = 0;
   uint32_t digit;

   while (*p && radix_digit(p, radix, digit)) {
      ret++;
   }

   return ret;
}

Q::Q(const char *orig, uint64_t radix) {
   // Let N enforce the supported radix range.
   N radix_check("", radix);
   N base(radix);

   char *tick = strchr((char *)orig, '\'');
   char *slash = strchr((char *)orig, '/');

   // The old d<number> escape is explicitly decimal because atof() is.
   if (radix == 10 && *orig == 'd') {
      double d = atof(orig + 1);
      *this = Q(d);
   }
   else if (tick || slash) {
      const char *p = orig;

      pos = true;
      if (*p == '-') {
         pos = false;
         p++;
      }
      else if (*p == '+') {
         p++;
      }

      if (slash) {
         if (tick) {
            whl = N(p, radix);
            num = N(tick + 1, radix);
         }
         else {
            whl = (int) 0;
            num = N(p, radix);
         }
         den = N(slash + 1, radix);
      }
      else {
         whl = N(p, radix);
         num = (int) 0;
         den = 1;
      }

      simplify();
   }
   else {
      char *freeme = strdup(orig);
      char *p = freeme;

      bool negexp = false;
      N exp;
      exp = (int) 0;
      N repetend_num;
      repetend_num = (int) 0;
      N repetend_den = 1;

      pos = true;
      if (*p == '-') {
         pos = false;
         p++;
      }
      else if (*p == '+') {
         p++;
      }

      // e/E is unambiguous only while E is not a valid digit.
      if (radix <= 14 && (strchr(p, 'e') || strchr(p, 'E'))) {
         char *q = strchr(p, 'e');
         if (!q) {
            q = strchr(p, 'E');
         }
         *q++ = 0;
         if (*q == '-') {
            negexp = true;
            q++;
         }
         else if (*q == '+') {
            q++;
         }
         exp = N(q, radix);
      }

      if (strchr(p, '(')) {
         char *q = strchr(p, '(');
         *q++ = 0;
         char *r = strchr(q, ')');
         if (!r) {
            free((void *)freeme);
            throw(UGE_ERR("no matching ')'"));
         }
         *r = 0;
         uint64_t repeatlen = radix_digits(q, radix);
         repetend_den = llpow(base, N(repeatlen)) - 1;
         repetend_num = N(q, radix);
      }

      uint64_t fraclen = 0;

      if (strchr(p, '.')) {
         char *q = strchr(p, '.');
         *q++ = 0;
         whl = N(p, radix);
         num = N(q, radix);
         fraclen = radix_digits(q, radix);
         den = llpow(base, N(fraclen));
      }
      else {
         whl = N(p, radix);
         num = (int) 0;
         den = 1;
      }

      free((void *)freeme);

      repetend_den *= den;

      // Preserve the sign from the input while assembling a repetend.  A
      // value such as -0.(3) has a zero nonrepeating part; simplify() quite
      // correctly canonicalizes that temporary zero as positive, but the
      // repetend still belongs to the originally negative number.
      bool parsed_pos = pos;
      simplify();

      if (!repetend_num.isZero()) {
         Q r(parsed_pos, (uint64_t) 0, repetend_num, repetend_den);
         *this = *this + r;
         simplify();
      }

      if (negexp) {
         N x = llpow(base, exp);
         Q r(1, (uint64_t)0, 1, x);
         *this = *this * r;
         simplify();
      }
      else if (!exp.isZero()) {
         Q r(1, llpow(base, exp), (uint64_t)0, 1);
         *this = *this * r;
         simplify();
      }
   }
}

Q::Q(double d) {
   whl = (int) 0;
   pos = true;
   if (d < 0.0) {
      pos = false;
      d = -d;
   }
   if (d > 1.0) {
      whl = int(d);
      d -= (double)((uint64_t)whl); // LOSS OF DATA
   }

   // and now the repeated fraction magic
   // https://en.wikipedia.org/wiki/Euler%27s_continued_fraction_formula
   double i = 1.0/d;
   N n = 1;
   N dens[1024];
   int spot = 0;
   while(isfinite(i) && i != 0 && n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
      n = (N) i;
      i = i - (double)((uint64_t)i);
      i = 1.0 / i;
      if (n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
         dens[spot++] = n;
      }
   }

   // a + b / c
   N a, b, c, nb, nc;
   b = (int) 0;
   c = 1;

   while (spot > 0) {
      a = dens[--spot];
      nc = a * c + b;
      nb = c;
      c = nc;
      b = nb;
   }

   num = b;
   den = c;
}

Q::Q(int64_t i) {
   pos = true;
   if (i < 0) {
      pos = false;
      i = -i;
   }
   whl = i;
   num = (int) 0;
   den = 1;
}

Q Q::operator + (Q const & obj) const { // addition operator
   Q res;

   // this could probably be done faster, at the cost 
   // of greater complexity

   N dd = den * obj.den;
   N l = whl * dd + num * obj.den;
   N r = obj.whl * dd + obj.num * den;

   if (pos == obj.pos) {
      res = Q(pos, (int)0, l + r, dd);
   }
   else {
      if (l > r) {
         res = Q(pos, (int)0, l - r, dd);
      }
      else {
         res = Q(obj.pos, (int) 0, r - l, dd);
      }
   }
   res.simplify();

   return res;
}

Q Q::operator - (Q const & obj) const { // subtraction operator
   return *this + (-obj);
}

Q Q::operator + () const { // unary plus

   // one wonders what the point is...
   // https://stackoverflow.com/questions/727516/what-does-the-unary-plus-operator-do

   return *this;
}

Q Q::operator - () const { // unary minus
   if (whl.isZero() && num.isZero()) {
      return Q(true, (uint64_t)0, (uint64_t)0, 1);
   }
   else {
      return Q(!pos, whl, num, den);
   }
}

Q Q::operator ~ () const { // unary bitwise not, truncates to integer
   if (pos) {
      return Q(false, whl + 1, (int)0, 1);
   }
   else {
      return Q(true, whl - 1, (int)0, 1);
   }
}

Q Q::operator * (Q const & obj) const {
   Q a(1, whl * obj.whl, (uint64_t)0, 1);
   Q b(1, (uint64_t)0, whl * obj.num, obj.den);
   Q c(1, (uint64_t)0, obj.whl * num, den);
   Q d(1, (uint64_t)0, obj.num * num, obj.den * den);
   Q ret = a + b + c + d;
   if (pos != obj.pos) {
      ret.pos = false;
   }
   return ret;
}

Q Q::operator / (Q const & obj) const {
   // 1/ (w+n/d)
   // ==
   // 1 / ((w*d+n)/d)
   // ==
   // d / (w*d+n)
   Q reciprocol(obj.pos, (uint64_t)0, obj.den, obj.whl * obj.den + obj.num);

   return *this * reciprocol;
}

Q Q::operator % (Q const & obj) const {
   Q res = *this / obj;
   res.whl = (int)0;
   res = res * obj;
   return res;
}

Q Q::operator & (Q const & obj) const { // truncates to integer
   if (pos) {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, false, false, N::BOOL_AND, false), (int)0, 1);
      }
      else {
         return Q(true, whl.apply(obj.whl, false, true, N::BOOL_AND, false), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, true, false, N::BOOL_AND, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, true, true, N::BOOL_AND, true), (int)0, 1);
      }
   }
}

Q Q::operator | (Q const & obj) const { // truncates to integer
   if (pos) {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, false, false, N::BOOL_OR, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, false, true, N::BOOL_OR, true), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(false, whl.apply(obj.whl, true, false, N::BOOL_OR, true), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, true, true, N::BOOL_OR, true), (int)0, 1);
      }
   }
}

Q Q::operator ^ (Q const & obj) const { // truncates to integer
   if (pos) {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, false, false, N::BOOL_XOR, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, false, true, N::BOOL_XOR, true), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(false, whl.apply(obj.whl, true, false, N::BOOL_XOR, true), (int)0, 1);
      }
      else {
         return Q(true, whl.apply(obj.whl, true, true, N::BOOL_XOR, false), (int)0, 1);
      }
   }
}

Q Q::operator >> (int64_t smallbits) const {
   Q result = *this;
   return result >>= smallbits;
}

Q Q::operator << (int64_t smallbits) const {
   Q result = *this;
   return result <<= smallbits;
}

bool Q::operator == (const Q &other) const {
   Q l(*this);
   l.simplify();
   Q r(other);
   r.simplify();

   return (l.pos == r.pos && l.whl == r.whl && l.num == r.num && l.den == r.den);
}

bool Q::operator != (const Q &other) const {
   return (!(*this == other));
}


bool Q::operator < (const Q &other) const {
   Q res = *this - other;
   return (!res.pos);
}

bool Q::operator > (const Q &other) const {
   Q res = other - *this;
   return (!res.pos);
}


bool Q::operator <= (const Q &other) const {
   return (!(*this > other));
}

bool Q::operator >= (const Q &other) const {
   return (!(*this < other));
}

Q& Q::operator+=(const Q& other) {
   *this = *this + other;
   return *this;
}

Q& Q::operator-=(const Q& other) {
   *this = *this - other;
   return *this;
}

Q& Q::operator*=(const Q& other) {
   *this = *this * other;
   return *this;
}

Q& Q::operator/=(const Q& other) {
   *this = *this / other;
   return *this;
}

Q& Q::operator%=(const Q& other) {
   *this = *this % other;
   return *this;
}

Q& Q::operator&=(const Q& other) {
   *this = *this & other;
   return *this;
}

Q& Q::operator|=(const Q& other) {
   *this = *this | other;
   return *this;
}

Q& Q::operator^=(const Q& other) {
   *this = *this ^ other;
   return *this;
}

Q& Q::operator >>= (int64_t bits) {
   num = (int) 0;
   den = 1;
   whl >>= bits;
   return *this;
}

Q& Q::operator <<= (int64_t bits) {
   num = (int) 0;
   den = 1;
   whl <<= bits;
   return *this;
}

char *Q::debu_print(void) const {
   char *ret = NULL;

   raprintf(ret, "[%c%s'%s/%s]",
      pos ? '+' : '-',
      GCSTR whl.dprint(),
      GCSTR num.dprint(),
      GCSTR den.dprint());

   return ret;
}

char *Q::frac_print(uint64_t radix) const {
   if (whl.isZero() && num.isZero()) {
      return strdup("0");
   }
   char mark[2] = {0, 0};
   if (!pos) {
      mark[0] = '-';
   }
   if (num.isZero()) {
      char *p;
      char *ret = mprintf("%s%s", mark, p = /*assign*/ whl.print(radix));
      free((void *)p);
      return ret;
   }
   else if (whl.isZero()) {
      char *ret = mprintf("%s%s/%s", mark,
          GCSTR num.print(radix),
          GCSTR den.print(radix));
      return ret;
   }
   else {
      char *ret = mprintf("%s%s'%s/%s", mark,
          GCSTR whl.print(radix),
          GCSTR num.print(radix),
          GCSTR den.print(radix));
      return ret;
   }
}

// Find the non-repeating prefix and repeating period lengths for 1/den
// in the requested radix.  Returns true if max truncated either search.
static bool radix_lengths(const N &den, uint64_t radix,
                          N &lead, N &repeat, uint64_t max) {
   static const N one(1);
   N remainder = den;
   N base(radix);
   bool maxlead = false;
   bool maxrepeat = false;


   // Each radix digit can cancel one gcd(remainder, radix).  When the
   // remainder becomes coprime to the radix, what remains must repeat.
   while (remainder != one) {
      N g = gcd(remainder, base);
      if (g == one) {
         break;
      }
      remainder /= g;
      lead += 1;

      if (lead >= max && remainder != one) {
         maxlead = true;
         lead = max;
         break;
      }
   }

   if (maxlead || remainder == one) {
      return maxlead;
   }

   // remainder is now coprime to radix.  The repeat length is the
   // multiplicative order of radix modulo remainder.
   N power = base % remainder;
   repeat = 1;

   while (power != one) {
      if (repeat >= max) {
         maxrepeat = true;
         repeat = max;
         break;
      }
      power = (power * base) % remainder;
      repeat += 1;
   }

   return maxrepeat;
}

char *Q::print(uint64_t radix, uint64_t max) const {
   char *ret = NULL;

   if (whl.isZero() && num.isZero()) {
      return strdup("0");
   }

   // Let N enforce the supported radix range for nonzero Q values.
   N radix_check("", radix);

   if (!pos) {
      raprintf(ret, "-");
   }

   raprintf(ret, "%s", GCSTR whl.print(radix));

   if (num.isZero()) {
      return ret;
   }

   raprintf(ret, ".");

   N lead;
   N repeat;

   bool bad = radix_lengths(den, radix, lead, repeat, max);

   N remainder = num * radix;
   N digit;

   for (N i; i < lead; i += 1) {
      digit = remainder / den;
      remainder %= den;
      raprintf(ret, "%s", GCSTR digit.print(radix));
      remainder *= radix;
   }
   if (bad && lead >= max) {
      raprintf(ret, "...");
   }
   else if (!repeat.isZero()) {
      raprintf(ret, "(");
      for (N i; i < repeat; i += 1) {
         digit = remainder / den;
         remainder %= den;
         raprintf(ret, "%s", GCSTR digit.print(radix));
         remainder *= radix;
      }
      if (bad && repeat >= max) {
         raprintf(ret, "...");
      }
      raprintf(ret, ")");
   }

   return ret;
}

char *Q::deci_print(uint64_t max) const {
   return print(10, max);
}

static void n_double_parts(const N &value, double &mantissa, int64_t &exponent) {
   // Keep the most-significant 53 bits, which is all a binary64 can use.
   // Reducing N before converting avoids uint64_t truncation of large values.
   N v(value);
   const N limit(UINT64_C(0x1fffffffffffff));
   const N chunk_limit = limit << 16;
   exponent = 0;

   while (v > chunk_limit) {
      v >>= 16;
      exponent += 16;
   }
   while (v > limit) {
      v >>= 1;
      exponent++;
   }

   mantissa = (double)((uint64_t)v);
}

static double n_as_double(const N &value) {
   double mantissa;
   int64_t exponent;
   n_double_parts(value, mantissa, exponent);
   if (mantissa == 0.0) return 0.0;
   if (exponent > 2048) return HUGE_VAL;
   return ldexp(mantissa, (int)exponent);
}

static double n_ratio_as_double(const N &num, const N &den) {
   if (num.isZero()) return 0.0;

   double nm, dm;
   int64_t ne, de;
   n_double_parts(num, nm, ne);
   n_double_parts(den, dm, de);
   int64_t exponent = ne - de;
   if (exponent > 2048) return HUGE_VAL;
   if (exponent < -2048) return 0.0;
   return ldexp(nm / dm, (int)exponent);
}

Q::operator double() const {
   double magnitude = n_as_double(whl) + n_ratio_as_double(num, den);
   return pos ? magnitude : -magnitude;
}

Q::operator int64_t() const {
   return ((int64_t)(pos ? 1 : -1) * (uint64_t)whl);
}

Q Q::abs(void) const {
   Q ret(*this);
   ret.pos = true;
   return ret;
}

Q Q::floor(void) const {
   Q ret(*this);
   if (ret.pos) {
      ret.num = (int) 0;
      ret.den = 1;
   }
   if (!ret.pos && ret.num > (uint64_t)0) {
      ret.whl += 1;
      ret.num = (int) 0;
      ret.den = 1;
   }
   return ret;
}

int Q::sgn(void) const {
   if(num.isZero() && whl.isZero()) { return 0; }
   return pos ? 1 : -1;
}

static uint64_t guarded_precision(uint64_t precision, uint64_t guard) {
   if (precision > (uint64_t)INT64_MAX ||
       guard > (uint64_t)INT64_MAX - precision) {
      throw(UGE_ERR("precision too large"));
   }
   return precision + guard;
}

static Q precision_epsilon(uint64_t precision) {
   if (precision > (uint64_t)INT64_MAX) {
      throw(UGE_ERR("precision too large"));
   }

   N d = 1;
   d <<= (int64_t)precision;
   return Q(true, N((uint64_t)0), N((uint64_t)1), d);
}

// Truncate a rational to a binary fixed-point grid.  Approximation routines
// use this to prevent exact intermediate fractions from accumulating enormous
// and useless denominators (for example, products of Taylor factorials).
static Q binary_trunc(const Q &q, uint64_t precision) {
   if (q.sgn() == 0) {
      return Q((int64_t)0);
   }

   Q eps = precision_epsilon(precision);
   Q mag = (q.abs() / eps).floor() * eps;
   return q.sgn() < 0 ? -mag : mag;
}

static uint64_t n_bit_length(N z) {
   uint64_t ret = 0;
   while (!z.isZero()) {
      z >>= 1;
      ret++;
   }
   return ret;
}

// Taylor series for atan(x), used only with |x| <= 1/3.  At that
// magnitude the terms decrease quickly, and the alternating-series
// remainder is bounded by the first omitted term.
static Q atan_series(const Q &x, uint64_t precision) {
   if (x.sgn() == 0) {
      return Q((int64_t)0);
   }

   Q eps = precision_epsilon(precision);
   Q xr = binary_trunc(x, precision);
   Q x2 = binary_trunc(xr * xr, precision);
   Q term = xr;
   Q sum = xr;

   for (uint64_t k = 1; ; k++) {
      if (k > (UINT64_MAX - 1) / 2) {
         throw(UGE_ERR("atan series overflow"));
      }
      uint64_t a = 2 * k - 1;
      uint64_t b = 2 * k + 1;
      term = binary_trunc(
         -(term * x2 * Q((int64_t)a) / Q((int64_t)b)), precision);
      sum = binary_trunc(sum + term, precision);
      if (term.abs() < eps) {
         return sum;
      }
   }
}

Q Q::pi(uint64_t precision) {
   // Machin's formula:
   // pi = 16 atan(1/5) - 4 atan(1/239)
   // Eight guard bits are enough to cover the small integer multipliers
   // and the two truncation errors.
   uint64_t work = guarded_precision(precision, 8);
   Q a = atan_series(Q((int64_t)1) / Q((int64_t)5), work);
   Q b = atan_series(Q((int64_t)1) / Q((int64_t)239), work);
   Q ret = Q((int64_t)16) * a - Q((int64_t)4) * b;

   // The series result is already an approximation.  Collapse its very
   // large exact denominator onto the requested binary precision grid so
   // using pi as an input to another approximation does not cause needless
   // denominator growth.
   return binary_trunc(ret, precision);
}

Q Q::tau(uint64_t precision) {
   // tau is derived from the same pi approximation; multiplying by two is
   // exact and does not require a second transcendental calculation.
   return Q((int64_t)2) * Q::pi(precision);
}

static Q atan_positive(const Q &x, const Q &p, uint64_t precision) {
   Q one((int64_t)1);
   Q half = one / Q((int64_t)2);

   if (x > one) {
      return p / Q((int64_t)2) - atan_positive(one / x, p, precision);
   }

   if (x > half) {
      // atan(x) = pi/4 + atan((x-1)/(x+1)); for 1/2 < x <= 1
      // the transformed argument has magnitude at most 1/3.
      Q reduced = (x - one) / (x + one);
      return p / Q((int64_t)4) + atan_series(reduced, precision);
   }

   return atan_series(x, precision);
}

Q Q::atan(uint64_t precision) const {
   if (sgn() == 0) {
      return Q((int64_t)0);
   }

   uint64_t work = guarded_precision(precision, 12);
   Q p = Q::pi(work);
   Q ret = atan_positive(abs(), p, work);
   return pos ? ret : -ret;
}

Q Q::atan2(const Q &x, uint64_t precision) const {
   int ys = sgn();
   int xs = x.sgn();

   if (!ys && !xs) {
      throw(UGE_ERR("atan2 undefined for (0,0)"));
   }

   uint64_t work = guarded_precision(precision, 16);
   Q p = Q::pi(work);

   if (!xs) {
      return ys > 0 ? p / Q((int64_t)2) : -p / Q((int64_t)2);
   }

   if (!ys) {
      return xs > 0 ? Q((int64_t)0) : p;
   }

   Q ratio = *this / x;
   Q ret = atan_positive(ratio.abs(), p, work);
   if (ratio.sgn() < 0) {
      ret = -ret;
   }
   if (xs > 0) {
      return ret;
   }
   return ys > 0 ? ret + p : ret - p;
}

static Q sin_series(const Q &x, uint64_t precision) {
   if (x.sgn() == 0) {
      return Q((int64_t)0);
   }

   Q eps = precision_epsilon(precision);
   Q xr = binary_trunc(x, precision);
   Q x2 = binary_trunc(xr * xr, precision);
   Q term = xr;
   Q sum = xr;

   for (uint64_t k = 1; ; k++) {
      if (k > (UINT64_MAX - 1) / 2) {
         throw(UGE_ERR("sin series overflow"));
      }
      uint64_t a = 2 * k;
      uint64_t b = 2 * k + 1;
      term = binary_trunc(
         -(term * x2 / Q((int64_t)a) / Q((int64_t)b)), precision);
      sum = binary_trunc(sum + term, precision);
      if (term.abs() < eps) {
         return sum;
      }
   }
}

static Q cos_series(const Q &x, uint64_t precision) {
   Q eps = precision_epsilon(precision);
   Q xr = binary_trunc(x, precision);
   Q x2 = binary_trunc(xr * xr, precision);
   Q term((int64_t)1);
   Q sum((int64_t)1);

   for (uint64_t k = 1; ; k++) {
      if (k > UINT64_MAX / 2) {
         throw(UGE_ERR("cos series overflow"));
      }
      uint64_t a = 2 * k - 1;
      uint64_t b = 2 * k;
      term = binary_trunc(
         -(term * x2 / Q((int64_t)a) / Q((int64_t)b)), precision);
      sum = binary_trunc(sum + term, precision);
      if (term.abs() < eps) {
         return sum;
      }
   }
}

static Q reduce_angle(const Q &x, uint64_t precision) {
   Q p = Q::pi(precision);
   Q period = Q((int64_t)2) * p;
   Q turns = (x / period).floor();
   Q r = x - turns * period;

   // floor() above leaves r in [0,2*pi); use the symmetric interval.
   if (r > p) {
      r -= period;
   }
   return r;
}

Q Q::sin(uint64_t precision) const {
   if (sgn() == 0) {
      return Q((int64_t)0);
   }

   // Range reduction by an approximate pi multiplies pi's error by the
   // number of turns.  Keep enough extra bits for the integer magnitude
   // of the argument, plus a fixed cushion for the series arithmetic.
   uint64_t work = guarded_precision(
      precision, guarded_precision(n_bit_length(whl), 32));
   return sin_series(reduce_angle(*this, work), work);
}

Q Q::cos(uint64_t precision) const {
   if (sgn() == 0) {
      return Q((int64_t)1);
   }

   uint64_t work = guarded_precision(
      precision, guarded_precision(n_bit_length(whl), 32));
   return cos_series(reduce_angle(*this, work), work);
}

Q Q::tan(uint64_t precision) const {
   if (sgn() == 0) {
      return Q((int64_t)0);
   }

   uint64_t work = guarded_precision(
      precision, guarded_precision(n_bit_length(whl), 40));
   Q r = reduce_angle(*this, work);
   Q c = cos_series(r, work);
   if (c.sgn() == 0) {
      throw(UGE_ERR("tangent undefined"));
   }
   return binary_trunc(sin_series(r, work) / c, work);
}


// Return the exact fractional part of a number of turns in [0,1).
static Q turn_fraction(const Q &x) {
   return x - x.floor();
}

static Q qratio(int64_t n, int64_t d) {
   return Q(n) / Q(d);
}

Q Q::sintau(uint64_t precision) const {
   Q r = turn_fraction(*this);
   Q zero((int64_t)0);
   Q one((int64_t)1);
   Q half = qratio(1, 2);
   Q quarter = qratio(1, 4);

   // Exact rational values for rational turns.  These are mathematical
   // identities, not results of the approximation machinery.
   if (r == zero || r == half) return zero;
   if (r == quarter) return one;
   if (r == qratio(3, 4)) return -one;
   if (r == qratio(1, 12) || r == qratio(5, 12)) return half;
   if (r == qratio(7, 12) || r == qratio(11, 12)) return -half;

   // Use the symmetric interval (-1/2,1/2], then reflect exactly into
   // [-1/4,1/4].  Only after that do we introduce an approximation to tau.
   if (r > half) r -= one;
   if (r > quarter) r = half - r;
   else if (r < -quarter) r = -half - r;

   uint64_t work = guarded_precision(precision, 32);
   Q angle = binary_trunc(r * Q::tau(work), work);
   return sin_series(angle, work);
}

Q Q::costau(uint64_t precision) const {
   Q r = turn_fraction(*this);
   Q zero((int64_t)0);
   Q one((int64_t)1);
   Q half = qratio(1, 2);
   Q quarter = qratio(1, 4);

   if (r == zero) return one;
   if (r == half) return -one;
   if (r == quarter || r == qratio(3, 4)) return zero;
   if (r == qratio(1, 6) || r == qratio(5, 6)) return half;
   if (r == qratio(1, 3) || r == qratio(2, 3)) return -half;

   if (r > half) r -= one;

   int sign = 1;
   if (r < zero) r = -r;       // cosine is even
   if (r > quarter) {
      r = half - r;
      sign = -1;
   }

   uint64_t work = guarded_precision(precision, 32);
   Q angle = binary_trunc(r * Q::tau(work), work);
   Q ret = cos_series(angle, work);
   return sign < 0 ? -ret : ret;
}

Q Q::tantau(uint64_t precision) const {
   Q one((int64_t)1);
   Q half = qratio(1, 2);
   Q quarter = qratio(1, 4);
   Q eighth = qratio(1, 8);

   // Tangent has period one half-turn.  Reduce that period exactly.
   Q turns = (*this / half).floor();
   Q r = *this - turns * half;       // [0,1/2)
   if (r > quarter) r -= half;       // (-1/4,1/4]

   if (r.sgn() == 0) return Q((int64_t)0);
   if (r == quarter || r == -quarter) {
      throw(UGE_ERR("tangent undefined"));
   }
   if (r == eighth) return one;
   if (r == -eighth) return -one;

   uint64_t work = guarded_precision(precision, 40);
   Q angle = binary_trunc(r * Q::tau(work), work);
   Q c = cos_series(angle, work);
   if (c.sgn() == 0) {
      throw(UGE_ERR("tangent undefined"));
   }
   return binary_trunc(sin_series(angle, work) / c, work);
}

Q Q::sinpi(uint64_t precision) const {
   return (*this / Q((int64_t)2)).sintau(precision);
}

Q Q::cospi(uint64_t precision) const {
   return (*this / Q((int64_t)2)).costau(precision);
}

Q Q::tanpi(uint64_t precision) const {
   return (*this / Q((int64_t)2)).tantau(precision);
}

Q Q::atantau(uint64_t precision) const {
   if (sgn() == 0) return Q((int64_t)0);

   Q one((int64_t)1);
   if (abs() == one) {
      Q eighth = qratio(1, 8);
      return sgn() < 0 ? -eighth : eighth;
   }

   uint64_t work = guarded_precision(precision, 20);
   Q p = Q::pi(work);
   Q ret = atan_positive(abs(), p, work);
   if (sgn() < 0) ret = -ret;
   return binary_trunc(ret / (Q((int64_t)2) * p), work);
}

Q Q::atanpi(uint64_t precision) const {
   return Q((int64_t)2) * atantau(precision);
}

Q Q::atan2tau(const Q &x, uint64_t precision) const {
   int ys = sgn();
   int xs = x.sgn();

   if (!ys && !xs) {
      throw(UGE_ERR("atan2 undefined for (0,0)"));
   }

   Q zero((int64_t)0);
   Q quarter = qratio(1, 4);
   Q half = qratio(1, 2);

   // Axes are exact in turns.
   if (!xs) return ys > 0 ? quarter : -quarter;
   if (!ys) return xs > 0 ? zero : half;

   Q ret = (*this / x).atantau(precision);
   if (xs > 0) return ret;
   return ys > 0 ? ret + half : ret - half;
}

Q Q::atan2pi(const Q &x, uint64_t precision) const {
   return Q((int64_t)2) * atan2tau(x, precision);
}

Q Q::sindeg(uint64_t precision) const {
   return (*this / Q((int64_t)360)).sintau(precision);
}

Q Q::cosdeg(uint64_t precision) const {
   return (*this / Q((int64_t)360)).costau(precision);
}

Q Q::tandeg(uint64_t precision) const {
   return (*this / Q((int64_t)360)).tantau(precision);
}

Q Q::atandeg(uint64_t precision) const {
   return Q((int64_t)360) * atantau(precision);
}

Q Q::atan2deg(const Q &x, uint64_t precision) const {
   return Q((int64_t)360) * atan2tau(x, precision);
}

// ln(x) = 2 * (z + z^3/3 + z^5/5 + ...), z=(x-1)/(x+1).
// This helper is used only for 1 <= x <= 2, so 0 <= z <= 1/3.
static Q ln_series(const Q &x, uint64_t precision) {
   Q one((int64_t)1);
   if (x == one) {
      return Q((int64_t)0);
   }

   Q eps = precision_epsilon(precision);
   Q z = binary_trunc((x - one) / (x + one), precision);
   Q z2 = binary_trunc(z * z, precision);
   Q power = z;
   Q sum = z;
   uint64_t d = 1;

   for (;;) {
      if (d > UINT64_MAX - 2) {
         throw(UGE_ERR("ln series overflow"));
      }
      d += 2;
      power = binary_trunc(power * z2, precision);
      sum = binary_trunc(sum + power / Q((int64_t)d), precision);

      if (d > UINT64_MAX - 2) {
         throw(UGE_ERR("ln series overflow"));
      }
      Q next = binary_trunc(
         (power * z2) / Q((int64_t)(d + 2)), precision);
      Q bound = Q((int64_t)2) * next / (one - z2);
      if (bound < eps) {
         return Q((int64_t)2) * sum;
      }
   }
}

Q Q::ln(uint64_t precision) const {
   if (!pos || sgn() == 0) {
      throw(UGE_ERR("logarithm of non-positive number"));
   }

   Q one((int64_t)1);
   Q two((int64_t)2);
   if (*this == one) {
      return Q((int64_t)0);
   }

   Q m = *this;
   int64_t shifts = 0;
   while (m >= two) {
      m /= two;
      if (shifts == INT64_MAX) {
         throw(UGE_ERR("ln argument too large"));
      }
      shifts++;
   }
   while (m < one) {
      m *= two;
      if (shifts == INT64_MIN) {
         throw(UGE_ERR("ln argument too small"));
      }
      shifts--;
   }

   uint64_t shiftmag = shifts < 0 ? (uint64_t)(-(shifts + 1)) + 1
                                  : (uint64_t)shifts;
   uint64_t shiftbits = 0;
   for (uint64_t t = shiftmag; t; t >>= 1) shiftbits++;

   uint64_t work = guarded_precision(precision, 16);
   uint64_t ln2work = guarded_precision(work, shiftbits);
   Q ret = ln_series(m, work);
   if (shifts) {
      ret += Q(shifts) * ln_series(two, ln2work);
   }
   return ret;
}

// Taylor series for exp(x), used only with |x| <= 1/2.  Once the next
// term is small enough, the remaining tail is less than twice that term.
static Q exp_series(const Q &x, uint64_t precision) {
   Q eps = precision_epsilon(precision);
   Q one((int64_t)1);
   Q xr = binary_trunc(x, precision);
   Q sum = one;
   Q term = one;

   for (uint64_t n = 1; ; n++) {
      if (n > (uint64_t)INT64_MAX - 2) {
         throw(UGE_ERR("exp series overflow"));
      }
      term = binary_trunc(term * xr / Q((int64_t)n), precision);
      sum = binary_trunc(sum + term, precision);

      Q next = binary_trunc(term * xr / Q((int64_t)(n + 1)), precision);
      if (Q((int64_t)2) * next.abs() < eps) {
         return sum;
      }
   }
}

Q Q::e(uint64_t precision) const {
   if (sgn() == 0) {
      return Q((int64_t)1);
   }

   Q half = Q((int64_t)1) / Q((int64_t)2);
   Q r = *this;
   uint64_t squarings = 0;

   // exp(x) = exp(x / 2^k)^(2^k).  Keeping the series argument close
   // to zero makes convergence fast; the extra working bits absorb the
   // error amplification from the subsequent squarings.
   while (r.abs() > half) {
      r /= Q((int64_t)2);
      if (squarings == UINT64_MAX) {
         throw(UGE_ERR("exponent too large"));
      }
      squarings++;
   }

   uint64_t work = guarded_precision(
      precision, guarded_precision(squarings, 24));
   Q ret = exp_series(r, work);
   while (squarings--) {
      ret = binary_trunc(ret * ret, work);
   }
   return ret;
}

Q Q::sqrt(uint64_t precision) const {
   if (!pos) {
      throw(UGE_ERR("square root of negative number."));
   }

   N n = num + den * whl;
   N d = den;
   N m = 1;
   m <<= precision;

   n *= m;
   d *= m;

   n = n.sqrt();
   d = d.sqrt();

   return Q(1, (int)0, n, d); // constructor will simplify()
}

Q Q::pow(const Q &power, uint64_t precision) const {
   N pn = power.num + power.den * power.whl;
   N pd = power.den;

   N n = (num + den * whl).pow(pn);
   N d = den.pow(pn);

   bool retpos = true;
   if (!pos) {
      if (!(pn % 2).isZero()) {
         retpos = false;
      }
   }

   if (!retpos && (pd % 2).isZero()) {
      throw (UGE_ERR("even root of negative number"));
   }

   N nr = n.root(pd);
   N dr = d.root(pd);

   if (nr.pow(pd) != n || dr.pow(pd) != d) {
      N m = 1;
      m <<= precision;

      nr = (n * m).root(pd);
      dr = (d * m).root(pd);
   }

   return Q(retpos, (int)0, nr, dr); // constructor will simplify()
}

// vim: expandtab:noai:ts=3:sw=3
