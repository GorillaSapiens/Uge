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

static Z llpow10(Z n) {
   if (n.isZero()) {
      return 1;
   }
   else if (n == 1) {
      return 10;
   }
   else {
      Z a = n / 2;
      Z b = n - a;
      return llpow10(a) * llpow10(b);
   }
}

Q::Q(const char *orig) {
   char *tick = strchr((char *)orig, '\'');
   char *slash = strchr((char *)orig, '/');
   char *dee = strchr((char *)orig, 'd');

   if (dee) {
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
            whl = Z(p);
            num = Z(tick + 1);
         }
         else {
            whl = (int) 0;
            num = Z(p);
         }
         den = Z(slash + 1);
      }
      else {
         whl = Z(p);
         num = (int) 0;
         den = 1;
      }

      simplify();
   }
   else {
      char *freeme = strdup(orig);
      char *p = freeme;

      for (char *q = p; *q; q++) {
         switch(*q) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '(':
            case ')':
            case 'e':
            case 'E':
            case '+':
            case '-':
            case '.':
               break;
            default:
               *q = 0;
         }
      }

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

      if (strchr(p, 'e') || strchr(p, 'E')) {
         char *q = strchr(p, 'e');
         if (!q) {
            q = strchr(p, 'E');
         }
         *q++ = 0;
         if (*q == '-') {
            negexp = true;
            q++;
         }
         exp = Z(q);
      }

      if (strchr(p, '(')) {
         char *q = strchr(p, '(');
         *q++ = 0;
         char *r = strchr(q, ')');
         if (!r) {
            free((void *)freeme);
            throw(UGE_ERR("no matching ')'"));
         }
         repetend_den = llpow10(r - q) - 1;
         repetend_num = Z(q);
      }

      int fraclen = 0;

      if (strchr(p, '.')) {
         char *q = strchr(p, '.');
         *q++ = 0;
         whl = Z(p);
         num = Z(q);
         fraclen = strlen(q);
         den = llpow10(fraclen);
      }
      else {
         whl = Z(p);
         num = (int) 0;
         den = 1;

         fraclen = 0;
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
         Z x = llpow10(exp);
         Q r(1, (uint64_t)0, 1, x);
         *this = *this * r;
         simplify();
      }
      else if (!exp.isZero()) {
         Q r(1, llpow10(exp), (uint64_t)0, 1);
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

char *Q::frac_print(void) const {
   if (whl.isZero() && num.isZero()) {
      return strdup("0");
   }
   char mark[2] = {0, 0};
   if (!pos) {
      mark[0] = '-';
   }
   if (num.isZero()) {
      char *p;
      char *ret = mprintf("%s%s", mark, p = /*assign*/ whl.print());
      free((void *)p);
      return ret;
   }
   else if (whl.isZero()) {
      char *ret = mprintf("%s%s/%s", mark,
          GCSTR num.print(),
          GCSTR den.print());
      return ret;
   }
   else {
      char *ret = mprintf("%s%s'%s/%s", mark,
          GCSTR whl.print(),
          GCSTR num.print(),
          GCSTR den.print());
      return ret;
   }
}

char *Q::deci_print(uint64_t max) const {
   static const Z zero;
   char *ret = NULL;

   if (!pos) {
      raprintf(ret, "-");
   }

   raprintf(ret, "%s", GCSTR whl.print());

   if (num.isZero()) {
      return ret;
   }

   raprintf(ret, ".");

   Z lead;
   Z repeat;

   bool bad = den.deci_lengths(lead, repeat, max);

   Z remainder = num * 10;
   Z digit;

   for (Z i; i < lead; i += 1) {
      digit = remainder / den;
      remainder = remainder % den;
      raprintf(ret, "%s", GCSTR digit.print());
      remainder *= 10;
   }
   if (bad && lead >= max) {
      raprintf(ret, "...");
   }
   else {
      if (repeat != zero) {
         raprintf(ret, "(");
         for (Z i; i < repeat; i += 1) {
            digit = remainder / den;
            remainder = remainder % den;
            raprintf(ret, "%s", GCSTR digit.print());
            remainder *= 10;
         }
         if (bad && repeat >= max) {
            raprintf(ret, "...");
         }
         raprintf(ret, ")");
      }
   }

   return ret;
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
