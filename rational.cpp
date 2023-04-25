// in development, so we're a little loosey goosey here...

#include <string>

#include <math.h>
#include <string.h>
#include <assert.h>

#include <string>

#include "rational.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

#define ERR(x) std::string(x " at " AT)

#define BIGPOWEROF2 512

static int initializer(void) {
   assert(sizeof(long long) * 8 == 64);
   assert(sizeof(int128_t) * 8 == 128);
   return 1;
}
int rational_assertions = initializer();

static uint128_t gcd(uint128_t x, uint128_t y) {
   // euclid
   uint128_t a = x;
   uint128_t b = y;
   if (b == 0) {
      a = y;
      b = x;
   }
   if (b == 0) {
      printf("%016lx%016lx %016lx%016lx\n", (uint64_t)(x >> 64), (uint64_t)(x & 0xFFFFFFFFFFFFFFFF), (uint64_t)(y >> 64), (uint64_t)(y & 0xFFFFFFFFFFFFFFFF));
      throw (ERR("divide by zero in gcd"));
   }
   uint128_t c = a % b;
   while (c != 0) {
      a = b;
      b = c;
      if (b == 0) {
         throw (ERR("divide by zero in gcd"));
      }
      c = a % b;
   }
   return b;
}

static uint128_t lcm(uint128_t x, uint128_t y) {
   return (x / gcd(x,y)) * y;
}

void Rational::simplify(void) {
   if (num >= den) {
      whl += num / den;
      num %= den;
   }

   uint128_t g = gcd(num,den);
   num /= g;
   den /= g;
}

Rational::Rational() {
   sign = 1;
   whl = 0;
   num = 0;
   den = 1;
}

Rational::Rational(const Rational &orig) {
   sign = orig.sign;
   whl = orig.whl;
   num = orig.num;
   den = orig.den;
}

Rational& Rational::operator=(const Rational& other) {
   sign = other.sign;
   whl = other.whl;
   num = other.num;
   den = other.den;

   return *this;
}

Rational::Rational(int8_t s, uint128_t w, uint128_t n, uint128_t d) {
   assert(d != 0);
   assert(s == -1 || s == 1);
   sign = s;
   whl = w;
   num = n;
   den = d;

   simplify();
}

static uint128_t llpow10(uint128_t n) {
   if (n == 0) {
      return 1;
   }
   else if (n == 1) {
      return 10;
   }
   else {
      long long a = n / 2LL;
      long long b = n - a;
      return llpow10(a) * llpow10(b);
   }
}

Rational::Rational(const char *orig) {
   char *tick = strchr((char *)orig, '\'');
   char *slash = strchr((char *)orig, '/');
   if (tick || slash) {
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
            whl = atoll(p);
            num = atoll(tick + 1);
         }
         else {
            whl = 0;
            num = atoll(p);
         }
         den = atoll(slash + 1);
      }
      else {
         whl = atoll(p);
         num = 0;
         den = 1;
      }
   }
   else {
      char *freeme = strdup(orig);
      char *p = freeme;

      long long exp = 0;
      long long repetend_num = 0;
      long long repetend_den = 1;

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
         exp = atoll(q);
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
         repetend_num = atoll(q);
      }

      int fraclen = 0;

      if (strchr(p, '.')) {
         char *q = strchr(p, '.');
         *q++ = 0;
         whl = atoll(p);
         num = atoll(q);
         fraclen = strlen(q);
         den = llpow10(fraclen);
      }
      else {
         whl = atoll(p);
         num = 0;
         den = 1;

         fraclen = 0;
      }

      free((void *)freeme);

      repetend_den *= den;

      simplify();

      if (repetend_num) {
         Rational r(sign, 0, repetend_num, repetend_den);
         *this = *this + r;
         simplify();
      }

      if (exp > 0) {
         Rational r(1, llpow10(exp), 0, 1);
         *this = *this * r;
         simplify();
      }
      else if (exp < 0) {
         Rational r(1, 0, 1, llpow10(-exp));
         *this = *this * r;
         simplify();
      }
   }
}

Rational::Rational(double d) {
   whl = 0;
   sign = 1;
   if (d < 0.0) {
      sign = -1;
      d = -d;
   }
   if (d > 1.0) {
      whl = int(d);
      d -= (double)whl;
   }

   // and now the repeated fraction magic
   // https://en.wikipedia.org/wiki/Euler%27s_continued_fraction_formula
   double i = 1.0/d;
   uint128_t n = 1;
   uint128_t dens[1024];
   int spot = 0;
   while(isfinite(i) && i != 0 && n > 0 && n < (BIGPOWEROF2 >> spot)) {
      n = (uint128_t) i;
      i = i - (double)((uint128_t)i);
      i = 1.0 / i;
      if (n > 0 && n < (BIGPOWEROF2 >> spot)) {
         dens[spot++] = n;
      }
   }

   // a + b / c
   uint128_t a, b, c, nb, nc;
   b = 0;
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

Rational::Rational(int128_t i) {
   sign = 1;
   if (i < 0) {
      sign = -1;
      i = -i;
   }
   whl = i;
   num = 0;
   den = 1;
}

Rational Rational::operator + (Rational const & obj) const {
   Rational res;

   if (sign == obj.sign) {
      // add them
      res = *this;
      res.whl += obj.whl;
      uint128_t l = lcm(res.den, obj.den);
      res.num *= l / res.den;
      res.den *= l / res.den;
      res.num += obj.num * (l / obj.den);
      res.simplify();
   }
   else {
      // subtract them
      Rational other;
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
      }

      if (res.sign > 0) {
         // we're still positive, keep subtracting
         uint128_t l = lcm(res.den, other.den);
         res.num *= l / res.den;
         res.den *= l / res.den;

         uint128_t othnum = other.num * (l / other.den);

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
         uint128_t l = lcm(res.den, other.den);
         res.num *= l / res.den;
         res.den *= l / res.den;
         res.num += other.num * (l / res.den);
      }
      res.simplify();
   }

   return res;
}

Rational Rational::operator - (Rational const & obj) const {
   return *this + (-obj);
}

Rational Rational::operator - () const {
   if (whl == 0 && num == 0) {
      return Rational(1, 0, 0, 1);
   }
   else {
      return Rational(-sign, whl, num, den);
   }
}

Rational Rational::operator * (Rational const & obj) const {
   Rational a(1, whl * obj.whl, 0, 1);
   Rational b(1, 0, whl * obj.num, obj.den);
   Rational c(1, 0, obj.whl * num, den);
   Rational d(1, 0, obj.num * num, obj.den * den);
   Rational ret = a + b + c + d;
   if (sign != obj.sign) {
      ret.sign = -1;
   }
   return ret;
}

Rational Rational::operator / (Rational const & obj) const {
   // 1/ (w+n/d)
   // ==
   // 1 / ((w*d+n)/d)
   // ==
   // d / (w*d+n)
   Rational reciprocol(obj.sign, 0, obj.den, obj.whl * obj.den + obj.num);

   return *this * reciprocol;
}

bool Rational::operator == (const Rational &other) const {
   Rational l(*this);
   l.simplify();
   Rational r(other);
   r.simplify();

   return (l.sign == r.sign && l.whl == r.whl && l.num == r.num && l.den == r.den);
}

bool Rational::operator != (const Rational &other) const {
   return (!(*this == other));
}


bool Rational::operator < (const Rational &other) const {
   Rational res = *this - other;
   return (res.sign < 0);
}

bool Rational::operator > (const Rational &other) const {
   Rational res = other - *this;
   return (res.sign < 0);
}


bool Rational::operator <= (const Rational &other) const {
   return (!(*this > other));
}

bool Rational::operator >= (const Rational &other) const {
   return (!(*this < other));
}

/*      UINT64_MAX 18446744073709551615ULL */
#define P10_UINT64 10000000000000000000ULL   /* 19 zeroes */
#define P10_LEN 19

static int print_u128_u(uint128_t u128, char *buf)
{
   uint64_t l1 = (u128 / ((uint128_t) P10_UINT64)) / ((uint128_t) P10_UINT64);
   uint64_t l2 = (u128 / ((uint128_t) P10_UINT64)) % ((uint128_t) P10_UINT64);
   uint64_t l3 = u128 % ((uint128_t) P10_UINT64);

   if (l1) {
      return sprintf(buf, "%lu%0*lu%0*lu", l1, P10_LEN, l2, P10_LEN, l3);
   }
   else {
      if (l2) {
         return sprintf(buf, "%lu%0*lu", l2, P10_LEN, l3);
      }
      else {
         return sprintf(buf, "%lu", l3);
      }
   }
}

char *Rational::debu_print(char *buf, size_t buflen) const {
   char whl_buf[128];
   char num_buf[128];
   char den_buf[128];

   print_u128_u(whl, whl_buf);
   print_u128_u(num, num_buf);
   print_u128_u(den, den_buf);

   snprintf(buf, buflen, "%c%s'%s/%s",
         sign > 0 ? '+' : '-', whl_buf, num_buf, den_buf);
   return buf;
}

char *Rational::frac_print(char *buf, size_t buflen) const {
   if (whl == 0 && num == 0) {
      snprintf(buf, buflen, "0");
      return buf;
   }
   char mark[2] = {0, 0};
   if (sign < 0) {
      mark[0] = '-';
   }
   if (num == 0) {
      char whl_buf[128];
      print_u128_u(whl, whl_buf);
      snprintf(buf, buflen, "%s%s", mark, whl_buf);
      return buf;
   }
   else if (whl == 0) {
      char num_buf[128];
      char den_buf[128];
      print_u128_u(num, num_buf);
      print_u128_u(den, den_buf);
      snprintf(buf, buflen, "%s%s/%s", mark, num_buf, den_buf);
      return buf;
   }
   else {
      char whl_buf[128];
      char num_buf[128];
      char den_buf[128];
      print_u128_u(whl, whl_buf);
      print_u128_u(num, num_buf);
      print_u128_u(den, den_buf);
      snprintf(buf, buflen, "%s%s'%s/%s", mark, whl_buf, num_buf, den_buf);
      return buf;
   }
}

typedef struct remchain_s {
   int digit;
   uint128_t remainder;
   struct remchain_s *next;
} remchain_t;

char *Rational::deci_print(char *buf, size_t buflen) const {
   char *ret = buf;

   if (sign < 0) {
      *buf++ = '-';
      buflen--;
      *buf = 0;
   }
   int rc = print_u128_u(whl, buf);
   buf += rc;
   buflen -= rc;

   if (num == 0) {
      return ret;
   }

   *buf++ = '.';
   buflen--;
   *buf = 0;

   remchain_t *head = NULL;
   remchain_t *tail = NULL;

   uint128_t remainder = num;

   head = tail = (remchain_t *) malloc(sizeof(remchain_t));
   head->digit = -1;
   head->remainder = remainder;
   head->next = NULL;

   while(1) {
      remainder *= 10;
      uint128_t digit = remainder / den;
      remainder = remainder % den;

      tail->next = (remchain_t *) malloc(sizeof(remchain_t));
      tail->next->digit = digit;
      tail->next->remainder = remainder;
      tail->next->next = NULL;
      tail = tail->next;

      if (remainder == 0) {
         for (remchain_t *tmp = head; tmp; tmp = tmp->next) {
            if (tmp->digit >= 0) {
               *buf++ = '0' + tmp->digit;
               buflen--;
               *buf = 0;
            }
         }
         return ret;
      }
      else {
         for (remchain_t *tmp = head; tmp != tail; tmp = tmp->next) {
            if (tmp->remainder == tail->remainder) {
               for (remchain_t *tmp2 = head; tmp2; tmp2 = tmp2->next) {
                  if (tmp2->digit >= 0) {
                     *buf++ = '0' + tmp2->digit;
                     buflen--;
                     *buf = 0;
                  }
                  if (tmp2 == tmp) {
                     *buf++ = '(';
                     buflen--;
                     *buf = 0;
                  }
               }
               *buf++ = ')';
               buflen--;
               *buf = 0;
               return ret;
            }
         }
      }
   }

   return ret;
}

Rational::operator double() const {
   return ((double)sign * ((double)whl + ((double)num / (double)den)));
}

Rational::operator int128_t() const {
   return ((int128_t)sign * (int128_t)whl);
}

Rational Rational::abs(void) const {
   Rational ret(*this);
   ret.sign = 1;
   return ret;
}

Rational Rational::floor(void) const {
   Rational ret(*this);
   if (ret.sign > 0) { ret.num = 0; ret.den = 1; }
   if (ret.sign < 0 && ret.num > 0) { ret.whl++; ret.num = 0; ret.den = 1; }
   return ret;
}

int Rational::sgn(void) const {
   if(num == 0 && whl == 0) { return 0; }
   return sign;
}
