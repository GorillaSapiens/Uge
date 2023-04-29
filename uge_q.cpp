#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "uge_err.hpp"
#include "uge_ramprintf.hpp"
#include "uge_gcstr.hpp"
#include "uge_q.hpp"

using namespace uge;

// constant used for repeated fraction guess when parsing (double)
#define BIGPOWEROF2 512

// max characters after decimal point when trying to find repeated fraction
#define MAX_DECI 256

// precision bits added for square root calculation
#define SQRT_PRECISION 1024

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

// static Z lcm(Z x, Z y) {
//    return (x / gcd(x,y)) * y;
// }

void Q::simplify(void) {
   if (num >= den) {
      whl += num / den;
      num %= den;
   }

   Z g = gcd(num,den);
   num /= g;
   den /= g;
}

Q::Q() {
   sign = 1;
   whl.setZero();
   num.setZero();
   den = 1;
}

Q::Q(const Q &orig) {
   sign = orig.sign;
   whl = orig.whl;
   num = orig.num;
   den = orig.den;
}

Q& Q::operator=(const Q& other) {
   sign = other.sign;
   whl = other.whl;
   num = other.num;
   den = other.den;

   return *this;
}

Q::Q(int8_t s, Z w, Z n, Z d) {
   assert(!d.isZero());
   assert(s == -1 || s == 1);
   sign = s;
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
            whl = Z(p);
            num = Z(tick + 1);
         }
         else {
            whl.setZero();
            num = Z(p);
         }
         den = Z(slash + 1);
      }
      else {
         whl = Z(p);
         num.setZero();
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
      exp.setZero();
      Z repetend_num;
      repetend_num.setZero();
      Z repetend_den = 1;

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
         num.setZero();
         den = 1;

         fraclen = 0;
      }

      free((void *)freeme);

      repetend_den *= den;

      simplify();

      if (!repetend_num.isZero()) {
         Q r(sign, (uint64_t) 0, repetend_num, repetend_den);
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

Q::Q(int64_t i) {
   sign = 1;
   if (i < 0) {
      sign = -1;
      i = -i;
   }
   whl = i;
   num.setZero();
   den = 1;
}

Q Q::operator + (Q const & obj) const {
   Q res;

   Z dd = den * obj.den;
   Z l = whl * dd + num * obj.den;
   Z r = obj.whl * dd + obj.num * den;

   if (sign == obj.sign) {
      res = Q(sign, (int)0, l + r, dd);
   }
   else {
      if (l > r) {
         res = Q(sign, (int)0, l - r, dd);
      }
      else {
         res = Q(obj.sign, (int) 0, r - l, dd);
      }
   }
   res.simplify();

   return res;
}

Q Q::operator - (Q const & obj) const {
   return *this + (-obj);
}

Q Q::operator - () const {
   if (whl.isZero() && num.isZero()) {
      return Q(1, (uint64_t)0, (uint64_t)0, 1);
   }
   else {
      return Q(-sign, whl, num, den);
   }
}

Q Q::operator * (Q const & obj) const {
   Q a(1, whl * obj.whl, (uint64_t)0, 1);
   Q b(1, (uint64_t)0, whl * obj.num, obj.den);
   Q c(1, (uint64_t)0, obj.whl * num, den);
   Q d(1, (uint64_t)0, obj.num * num, obj.den * den);
   Q ret = a + b + c + d;
   if (sign != obj.sign) {
      ret.sign = -1;
   }
   return ret;
}

Q Q::operator / (Q const & obj) const {
   // 1/ (w+n/d)
   // ==
   // 1 / ((w*d+n)/d)
   // ==
   // d / (w*d+n)
   Q reciprocol(obj.sign, (uint64_t)0, obj.den, obj.whl * obj.den + obj.num);

   return *this * reciprocol;
}

bool Q::operator == (const Q &other) const {
   Q l(*this);
   l.simplify();
   Q r(other);
   r.simplify();

   return (l.sign == r.sign && l.whl == r.whl && l.num == r.num && l.den == r.den);
}

bool Q::operator != (const Q &other) const {
   return (!(*this == other));
}


bool Q::operator < (const Q &other) const {
   Q res = *this - other;
   return (res.sign < 0);
}

bool Q::operator > (const Q &other) const {
   Q res = other - *this;
   return (res.sign < 0);
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

char *Q::debu_print(void) const {
   char *ret = NULL;

   raprintf(ret, "[%c%s'%s/%s]",
      sign > 0 ? '+' : '-',
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
   Z *remainder;
   struct remchain_s *next;
} remchain_t;

char *Q::deci_print(void) const {
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

   Z remainder = num;

   head = tail = (remchain_t *) malloc(sizeof(remchain_t));
   remchain_count++;
   head->digit = -1;
   head->remainder = new Z(remainder);
   head->next = NULL;

   while(1) {
      remainder *= 10;
      Z digit = remainder / den;
      remainder = remainder % den;

      tail->next = (remchain_t *) malloc(sizeof(remchain_t));
      remchain_count++;
      tail->next->digit = (uint64_t) digit;
      tail->next->remainder = new Z(remainder);
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

Q::operator double() const {
   return ((double)sign *
      ((double)((uint64_t)whl) +
      ((double)((uint64_t)num) /
       (double)((uint64_t)den))));
}

Q::operator int64_t() const {
   return ((int64_t)sign * (uint64_t)whl);
}

Q Q::abs(void) const {
   Q ret(*this);
   ret.sign = 1;
   return ret;
}

Q Q::floor(void) const {
   Q ret(*this);
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

int Q::sgn(void) const {
   if(num.isZero() && whl.isZero()) { return 0; }
   return sign;
}

Q Q::sqrt(void) const {
   if (sign < 0) {
      throw(UGE_ERR("square root of negative number."));
   }

   Z n = num + den * whl;
   Z d = den;
   Z m = 1;
   m <<= SQRT_PRECISION;

   n *= m;
   d *= m;

   n = n.sqrt();
   d = d.sqrt();

   return Q(1, (int)0, n, d); // constructor will simplify()
}
