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

static Z gcd(Z x, Z y) {
   // euclid
   Z a = x;
   Z b = y;
   if (b.isZero()) {
      a = y;
      b = x;
   }
   if (b.isZero()) {
      throw (UGE_ERR("divide by zero in gcd"));
   }
   Z c = a % b;
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

   Z g = gcd(num,den);
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

Q::Q(bool p, Z w, Z n, Z d) {
   assert(!d.isZero());
   pos = p;
   whl = w;
   num = n;
   den = d;

   simplify();
}

static Z llpow(Z base, Z n) {
   if (n.isZero()) {
      return 1;
   }
   else if (n == 1) {
      return base;
   }
   else {
      Z a = n / 2;
      Z b = n - a;
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
   // Let Z enforce the supported radix range.
   Z radix_check("", radix);
   Z base(radix);

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
            whl = Z(p, radix);
            num = Z(tick + 1, radix);
         }
         else {
            whl = (int) 0;
            num = Z(p, radix);
         }
         den = Z(slash + 1, radix);
      }
      else {
         whl = Z(p, radix);
         num = (int) 0;
         den = 1;
      }

      simplify();
   }
   else {
      char *freeme = strdup(orig);
      char *p = freeme;

      bool negexp = false;
      Z exp;
      exp = (int) 0;
      Z repetend_num;
      repetend_num = (int) 0;
      Z repetend_den = 1;

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
         exp = Z(q, radix);
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
         repetend_den = llpow(base, Z(repeatlen)) - 1;
         repetend_num = Z(q, radix);
      }

      uint64_t fraclen = 0;

      if (strchr(p, '.')) {
         char *q = strchr(p, '.');
         *q++ = 0;
         whl = Z(p, radix);
         num = Z(q, radix);
         fraclen = radix_digits(q, radix);
         den = llpow(base, Z(fraclen));
      }
      else {
         whl = Z(p, radix);
         num = (int) 0;
         den = 1;
      }

      free((void *)freeme);

      repetend_den *= den;

      simplify();

      if (!repetend_num.isZero()) {
         Q r(pos, (uint64_t) 0, repetend_num, repetend_den);
         *this = *this + r;
         simplify();
      }

      if (negexp) {
         Z x = llpow(base, exp);
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
   Z n = 1;
   Z dens[1024];
   int spot = 0;
   while(isfinite(i) && i != 0 && n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
      n = (Z) i;
      i = i - (double)((uint64_t)i);
      i = 1.0 / i;
      if (n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
         dens[spot++] = n;
      }
   }

   // a + b / c
   Z a, b, c, nb, nc;
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

   Z dd = den * obj.den;
   Z l = whl * dd + num * obj.den;
   Z r = obj.whl * dd + obj.num * den;

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
         return Q(true, whl.apply(obj.whl, false, false, Z::BOOL_AND, false), (int)0, 1);
      }
      else {
         return Q(true, whl.apply(obj.whl, false, true, Z::BOOL_AND, false), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, true, false, Z::BOOL_AND, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, true, true, Z::BOOL_AND, true), (int)0, 1);
      }
   }
}

Q Q::operator | (Q const & obj) const { // truncates to integer
   if (pos) {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, false, false, Z::BOOL_OR, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, false, true, Z::BOOL_OR, true), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(false, whl.apply(obj.whl, true, false, Z::BOOL_OR, true), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, true, true, Z::BOOL_OR, true), (int)0, 1);
      }
   }
}

Q Q::operator ^ (Q const & obj) const { // truncates to integer
   if (pos) {
      if (obj.pos) {
         return Q(true, whl.apply(obj.whl, false, false, Z::BOOL_XOR, false), (int)0, 1);
      }
      else {
         return Q(false, whl.apply(obj.whl, false, true, Z::BOOL_XOR, true), (int)0, 1);
      }
   }
   else {
      if (obj.pos) {
         return Q(false, whl.apply(obj.whl, true, false, Z::BOOL_XOR, true), (int)0, 1);
      }
      else {
         return Q(true, whl.apply(obj.whl, true, true, Z::BOOL_XOR, false), (int)0, 1);
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
static bool radix_lengths(const Z &den, uint64_t radix,
                          Z &lead, Z &repeat, uint64_t max) {
   static const Z one(1);
   Z remainder = den;
   Z base(radix);
   bool maxlead = false;
   bool maxrepeat = false;


   // Each radix digit can cancel one gcd(remainder, radix).  When the
   // remainder becomes coprime to the radix, what remains must repeat.
   while (remainder != one) {
      Z g = gcd(remainder, base);
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
   Z power = base % remainder;
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

   // Let Z enforce the supported radix range for nonzero Q values.
   Z radix_check("", radix);

   if (!pos) {
      raprintf(ret, "-");
   }

   raprintf(ret, "%s", GCSTR whl.print(radix));

   if (num.isZero()) {
      return ret;
   }

   raprintf(ret, ".");

   Z lead;
   Z repeat;

   bool bad = radix_lengths(den, radix, lead, repeat, max);

   Z remainder = num * radix;
   Z digit;

   for (Z i; i < lead; i += 1) {
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
      for (Z i; i < repeat; i += 1) {
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

Q::operator double() const {
   return ((double)(pos ? 1 : -1) *
      ((double)((uint64_t)whl) +
      ((double)((uint64_t)num) /
       (double)((uint64_t)den))));
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

Q Q::sqrt(uint64_t precision) const {
   if (!pos) {
      throw(UGE_ERR("square root of negative number."));
   }

   Z n = num + den * whl;
   Z d = den;
   Z m = 1;
   m <<= precision;

   n *= m;
   d *= m;

   n = n.sqrt();
   d = d.sqrt();

   return Q(1, (int)0, n, d); // constructor will simplify()
}

Q Q::pow(const Q &power, uint64_t precision) const {
   Z pn = power.num + power.den * power.whl;
   Z pd = power.den;

   Z n = (num + den * whl).pow(pn);
   Z d = den.pow(pn);

   bool retpos = true;
   if (!pos) {
      if (!(pn % 2).isZero()) {
         retpos = false;
      }
   }

   if (!retpos && (pd % 2).isZero()) {
      throw (UGE_ERR("even root of negative number"));
   }

   Z nr = n.root(pd);
   Z dr = d.root(pd);

   if (nr.pow(pd) != n || dr.pow(pd) != d) {
      Z m = 1;
      m <<= precision;

      nr = (n * m).root(pd);
      dr = (d * m).root(pd);
   }

   return Q(retpos, (int)0, nr, dr); // constructor will simplify()
}

// vim: expandtab:noai:ts=3:sw=3
