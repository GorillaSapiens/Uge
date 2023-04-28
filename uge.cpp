#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "err.hpp"
#include "ramprintf.hpp"
#include "gcstr.hpp"
#include "uge.hpp"

// constant used for repeated fraction guess when parsing (double)
#define BIGPOWEROF2 512

// max characters after decimal point when trying to find repeated fraction
#define MAX_DECI 256

// precision bits added for square root calculation
#define SQRT_PRECISION 1024

static Integer gcd(Integer x, Integer y) {
   // euclid
   Integer a = x;
   Integer b = y;
   if (b.isZero()) {
      a = y;
      b = x;
   }
   if (b.isZero()) {
      throw (ERR("divide by zero in gcd"));
   }
   Integer c = a % b;
   while (!c.isZero()) {
      a = b;
      b = c;
      if (b.isZero()) {
         throw (ERR("divide by zero in gcd"));
      }
      c = a % b;
   }
   return b;
}

static Integer lcm(Integer x, Integer y) {
   return (x / gcd(x,y)) * y;
}

void Uge::simplify(void) {
   if (num >= den) {
      whl += num / den;
      num %= den;
   }

   Integer g = gcd(num,den);
   num /= g;
   den /= g;
}

Uge::Uge() {
   sign = 1;
   whl.setZero();
   num.setZero();
   den = 1;
}

Uge::Uge(const Uge &orig) {
   sign = orig.sign;
   whl = orig.whl;
   num = orig.num;
   den = orig.den;
}

Uge& Uge::operator=(const Uge& other) {
   sign = other.sign;
   whl = other.whl;
   num = other.num;
   den = other.den;

   return *this;
}

Uge::Uge(int8_t s, Integer w, Integer n, Integer d) {
   assert(!d.isZero());
   assert(s == -1 || s == 1);
   sign = s;
   whl = w;
   num = n;
   den = d;

   simplify();
}

static Integer llpow10(Integer n) {
   if (n.isZero()) {
      return 1;
   }
   else if (n == 1) {
      return 10;
   }
   else {
      Integer a = n / 2;
      Integer b = n - a;
      return llpow10(a) * llpow10(b);
   }
}

Uge::Uge(const char *orig) {
   char *tick = strchr((char *)orig, '\'');
   char *slash = strchr((char *)orig, '/');
   char *dee = strchr((char *)orig, 'd');

   if (dee) {
      double d = atof(orig + 1);
      *this = Uge(d);
   }
   else if (tick || slash) {
      const char *p = orig;

      sign = 1;
      if (*p == '-') {
         sign = -1;
         p++;
      }
      else if (*p == '+') {
         p++;
      }

      if (slash) {
         if (tick) {
            whl = Integer(p);
            num = Integer(tick + 1);
         }
         else {
            whl.setZero();
            num = Integer(p);
         }
         den = Integer(slash + 1);
      }
      else {
         whl = Integer(p);
         num.setZero();
         den = 1;
      }

      simplify();
   }
   else {
      char *freeme = strdup(orig);
      char *p = freeme;

      bool negexp = false;
      Integer exp;
      exp.setZero();
      Integer repetend_num;
      repetend_num.setZero();
      Integer repetend_den = 1;

      sign = 1;
      if (*p == '-') {
         sign = -1;
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
         exp = Integer(q);
      }

      if (strchr(p, '(')) {
         char *q = strchr(p, '(');
         *q++ = 0;
         char *r = strchr(q, ')');
         if (!r) {
            free((void *)freeme);
            throw(ERR("no matching ')'"));
         }
         repetend_den = llpow10(r - q) - 1;
         repetend_num = Integer(q);
      }

      int fraclen = 0;

      if (strchr(p, '.')) {
         char *q = strchr(p, '.');
         *q++ = 0;
         whl = Integer(p);
         num = Integer(q);
         fraclen = strlen(q);
         den = llpow10(fraclen);
      }
      else {
         whl = Integer(p);
         num.setZero();
         den = 1;

         fraclen = 0;
      }

      free((void *)freeme);

      repetend_den *= den;

      simplify();

      if (!repetend_num.isZero()) {
         Uge r(sign, (uint64_t) 0, repetend_num, repetend_den);
         *this = *this + r;
         simplify();
      }

      if (negexp) {
         Integer x = llpow10(exp);
         Uge r(1, (uint64_t)0, 1, x);
         *this = *this * r;
         simplify();
      }
      else if (!exp.isZero()) {
         Uge r(1, llpow10(exp), (uint64_t)0, 1);
         *this = *this * r;
         simplify();
      }
   }
}

Uge::Uge(double d) {
   whl.setZero();
   sign = 1;
   if (d < 0.0) {
      sign = -1;
      d = -d;
   }
   if (d > 1.0) {
      whl = int(d);
      d -= (double)((uint64_t)whl); // LOSS OF DATA
   }

   // and now the repeated fraction magic
   // https://en.wikipedia.org/wiki/Euler%27s_continued_fraction_formula
   double i = 1.0/d;
   Integer n = 1;
   Integer dens[1024];
   int spot = 0;
   while(isfinite(i) && i != 0 && n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
      n = (Integer) i;
      i = i - (double)((uint64_t)i);
      i = 1.0 / i;
      if (n > (uint64_t)0 && n < (BIGPOWEROF2 >> spot)) {
         dens[spot++] = n;
      }
   }

   // a + b / c
   Integer a, b, c, nb, nc;
   b.setZero();
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

Uge::Uge(int64_t i) {
   sign = 1;
   if (i < 0) {
      sign = -1;
      i = -i;
   }
   whl = i;
   num.setZero();
   den = 1;
}

Uge Uge::operator + (Uge const & obj) const {
   Uge res;

   if (sign == obj.sign) {
      // add them
      res = *this;
      res.whl += obj.whl;
      Integer l = lcm(res.den, obj.den);
      res.num *= l / res.den;
      res.den *= l / res.den;
      res.num += obj.num * (l / obj.den);
      res.simplify();
   }
   else {
      // subtract them
      Uge other;
      if (sign > 0) {
         res = *this;
         other = obj;
      }
      else {
         res = obj;
         other = *this;
      }

      // res now positive, other is negative
      if (res.whl >= other.whl) {
         res.whl -= other.whl;
      }
      else {
         res.sign = -1;
         res.whl = other.whl - res.whl;
         if (!res.num.isZero()) {
            res.num = res.den - res.num;
            res.whl -= 1;
         }
      }

      if (res.sign > 0) {
         // we're still positive, keep subtracting
         Integer l = lcm(res.den, other.den);
         res.num *= l / res.den;
         res.den *= l / res.den;

         Integer othnum = other.num * (l / other.den);

         if (res.num >= othnum) {
            res.num -= othnum;
         }
         else {
            res.sign = -1;
            res.num = othnum - res.num;
         }
      }
      else {
         // we've gone negative, we add now
         Integer l = lcm(res.den, other.den);
         res.num *= l / res.den;
         res.den *= l / res.den;
         res.num += other.num * (l / res.den);
      }
      res.simplify();
   }

   return res;
}

Uge Uge::operator - (Uge const & obj) const {
   return *this + (-obj);
}

Uge Uge::operator - () const {
   if (whl.isZero() && num.isZero()) {
      return Uge(1, (uint64_t)0, (uint64_t)0, 1);
   }
   else {
      return Uge(-sign, whl, num, den);
   }
}

Uge Uge::operator * (Uge const & obj) const {
   Uge a(1, whl * obj.whl, (uint64_t)0, 1);
   Uge b(1, (uint64_t)0, whl * obj.num, obj.den);
   Uge c(1, (uint64_t)0, obj.whl * num, den);
   Uge d(1, (uint64_t)0, obj.num * num, obj.den * den);
   Uge ret = a + b + c + d;
   if (sign != obj.sign) {
      ret.sign = -1;
   }
   return ret;
}

Uge Uge::operator / (Uge const & obj) const {
   // 1/ (w+n/d)
   // ==
   // 1 / ((w*d+n)/d)
   // ==
   // d / (w*d+n)
   Uge reciprocol(obj.sign, (uint64_t)0, obj.den, obj.whl * obj.den + obj.num);

   return *this * reciprocol;
}

bool Uge::operator == (const Uge &other) const {
   Uge l(*this);
   l.simplify();
   Uge r(other);
   r.simplify();

   return (l.sign == r.sign && l.whl == r.whl && l.num == r.num && l.den == r.den);
}

bool Uge::operator != (const Uge &other) const {
   return (!(*this == other));
}


bool Uge::operator < (const Uge &other) const {
   Uge res = *this - other;
   return (res.sign < 0);
}

bool Uge::operator > (const Uge &other) const {
   Uge res = other - *this;
   return (res.sign < 0);
}


bool Uge::operator <= (const Uge &other) const {
   return (!(*this > other));
}

bool Uge::operator >= (const Uge &other) const {
   return (!(*this < other));
}

Uge& Uge::operator+=(const Uge& other) {
   *this = *this + other;
   return *this;
}

Uge& Uge::operator-=(const Uge& other) {
   *this = *this - other;
   return *this;
}

Uge& Uge::operator*=(const Uge& other) {
   *this = *this * other;
   return *this;
}

Uge& Uge::operator/=(const Uge& other) {
   *this = *this / other;
   return *this;
}

char *Uge::debu_print(void) const {
   char *ret = NULL;

   raprintf(ret, "[%c%s'%s/%s]",
      sign > 0 ? '+' : '-',
      GCSTR whl.dprint(),
      GCSTR num.dprint(),
      GCSTR den.dprint());

   return ret;
}

char *Uge::frac_print(void) const {
   if (whl.isZero() && num.isZero()) {
      return strdup("0");
   }
   char mark[2] = {0, 0};
   if (sign < 0) {
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

typedef struct remchain_s {
   int digit;
   Integer *remainder;
   struct remchain_s *next;
} remchain_t;

char *Uge::deci_print(void) const {
   char *ret = NULL;

   if (sign < 0) {
      raprintf(ret, "-");
   }

   raprintf(ret, "%s", GCSTR whl.print());

   if (num.isZero()) {
      return ret;
   }

   raprintf(ret, ".");

   remchain_t *head = NULL;
   remchain_t *tail = NULL;
   int remchain_count = 0;

   Integer remainder = num;

   head = tail = (remchain_t *) malloc(sizeof(remchain_t));
   remchain_count++;
   head->digit = -1;
   head->remainder = new Integer(remainder);
   head->next = NULL;

   while(1) {
      remainder *= 10;
      Integer digit = remainder / den;
      remainder = remainder % den;

      tail->next = (remchain_t *) malloc(sizeof(remchain_t));
      remchain_count++;
      tail->next->digit = (uint64_t) digit;
      tail->next->remainder = new Integer(remainder);
      tail->next->next = NULL;
      tail = tail->next;

      if (remainder.isZero() || remchain_count > MAX_DECI) {
         for (remchain_t *tmp = head; tmp; tmp = tmp->next) {
            if (tmp->digit >= 0) {
               raprintf(ret, "%c", '0' + tmp->digit);
            }
         }
         if (remchain_count > MAX_DECI) {
            raprintf(ret, "...");
         }
         goto fini;
      }
      else {
         for (remchain_t *tmp = head; tmp != tail; tmp = tmp->next) {
            if (*(tmp->remainder) == *(tail->remainder)) {
               for (remchain_t *tmp2 = head; tmp2; tmp2 = tmp2->next) {
                  if (tmp2->digit >= 0) {
                     raprintf(ret, "%c", '0' + tmp2->digit);
                  }
                  if (tmp2 == tmp) {
                     raprintf(ret, "(");
                  }
               }
               raprintf(ret, ")");
               goto fini;
            }
         }
      }
   }

   fini:

   remchain_t *next;
   remchain_t *tmp;
   for (tmp = head, next = tmp->next; tmp; tmp = next) {
      next = tmp->next;
      delete tmp->remainder;
      free((void *)tmp);
   }

   return ret;
}

Uge::operator double() const {
   return ((double)sign *
      ((double)((uint64_t)whl) +
      ((double)((uint64_t)num) /
       (double)((uint64_t)den))));
}

Uge::operator int64_t() const {
   return ((int64_t)sign * (uint64_t)whl);
}

Uge Uge::abs(void) const {
   Uge ret(*this);
   ret.sign = 1;
   return ret;
}

Uge Uge::floor(void) const {
   Uge ret(*this);
   if (ret.sign > 0) {
      ret.num.setZero();
      ret.den = 1;
   }
   if (ret.sign < 0 && ret.num > (uint64_t)0) {
      ret.whl += 1;
      ret.num.setZero();
      ret.den = 1;
   }
   return ret;
}

int Uge::sgn(void) const {
   if(num.isZero() && whl.isZero()) { return 0; }
   return sign;
}

Uge Uge::sqrt(void) const {
   if (sign < 0) {
      throw(ERR("square root of negative number."));
   }

   Integer n = num + den * whl;
   Integer d = den;
   Integer m = 1;
   m <<= SQRT_PRECISION;

   n *= m;
   d *= m;

   n = n.sqrt();
   d = d.sqrt();

   return Uge(1, (int)0, n, d); // constructor will simplify()
}
