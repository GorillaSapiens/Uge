#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>

#if __GNUC__

typedef __int128 sBIG_t;
typedef unsigned __int128 uBIG_t;

typedef int64_t sREG_t;
typedef uint64_t uREG_t;

#else

typedef int64_t sBIG_t;
typedef uint64_t uBIG_t;

typedef int32_t sREG_t;
typedef uint32_t uREG_t;

#endif

extern "C" {
   char *gets(char *);
}

static sBIG_t gcd(sBIG_t x, sBIG_t y) {
   // euclid
   sBIG_t a = x;
   sBIG_t b = y;
   sBIG_t c = a % b;
   while (c != 0) {
      a = b;
      b = c;
      c = a % b;
   }
   return b;
}

static sBIG_t lcm(sBIG_t x, sBIG_t y) {
   return (x * y) / gcd(x,y);
}

class Rational {
   private:
      int8_t sign;
      uREG_t whl;
      uREG_t num;
      uREG_t den;

   public:
      Rational() {
         sign = 1;
         whl = 0;
         num = 0;
         den = 1;
      }

      Rational(int8_t s, uREG_t w, uREG_t n, uREG_t d) {
         assert(d != 0);
         assert(s == -1 || s == 1);
         sign = s;
         whl = w;
         num = n;
         den = d;
      }

      Rational operator + (Rational const & obj) {
         Rational res;

         // TODO FIX can probably be done in a way that avoids overflow
         sBIG_t n =
            (((uBIG_t) whl *     (uBIG_t) den *     (uBIG_t) obj.den ) * (sBIG_t) sign) +
            (((uBIG_t) obj.whl * (uBIG_t) den *     (uBIG_t) obj.den ) * (sBIG_t) obj.sign) +
            (((uBIG_t) num *     (uBIG_t) obj.den                    ) * (sBIG_t) sign) +
            (((uBIG_t) obj.num * (uBIG_t) den                        ) * (sBIG_t) obj.sign);
         uBIG_t d = (uBIG_t) den * (uBIG_t) obj.den;

         int8_t s;
         if (n >= 0) {
            s = 1;
         }
         else {
            s = -1;
            n = -n;
         }

         uBIG_t g = gcd(n,d);
         n /= g;
         d /= g;

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      Rational operator - (Rational const & obj) {
         Rational res;

         // TODO FIX can probably be done in a way that avoids overflow
         sBIG_t n =
            (((uBIG_t)whl     * (uBIG_t)den * (uBIG_t)obj.den) * (sBIG_t) sign) -
            (((uBIG_t)obj.whl * (uBIG_t)den * (uBIG_t)obj.den) * (sBIG_t) obj.sign) +
            (((uBIG_t)num     * (uBIG_t)obj.den)               * (sBIG_t) sign) -
            (((uBIG_t)obj.num * (uBIG_t)den)                   * (sBIG_t) obj.sign);
         uBIG_t d = den * obj.den;

         int8_t s;
         if (n >= 0) {
            s = 1;
         }
         else {
            s = -1;
            n = -n;
         }

         uBIG_t g = gcd(n,d);
         n /= g;
         d /= g;

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      Rational operator * (Rational const & obj) {
         Rational res;

         // TODO FIX can probably be done in a way that avoids overflow
         uBIG_t n =
            ((uBIG_t)whl     * (uBIG_t)obj.whl * (uBIG_t)den * (uBIG_t)obj.den) +
            ((uBIG_t)whl     * (uBIG_t)obj.num * (uBIG_t)den)  +
            ((uBIG_t)obj.whl * (uBIG_t)num     * (uBIG_t)obj.den)  +
            ((uBIG_t)num     * (uBIG_t)obj.num);
         uBIG_t d = (uBIG_t)den * (uBIG_t)obj.den;

         uBIG_t g = gcd(n,d);
         n /= g;
         d /= g;

         int8_t s;
         if (sign == obj.sign) {
            s = 1;
         }
         else {
            s = -1;
         }

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      Rational operator / (Rational const & obj) {
         Rational res;

         // TODO FIX can probably be done in a way that avoids overflow
         uBIG_t n =
            (uBIG_t)obj.den * ((uBIG_t)whl * (uBIG_t)den + (uBIG_t)num);
         uBIG_t d =
            (uBIG_t)den * ((uBIG_t)obj.whl * (uBIG_t)obj.den + (uBIG_t)obj.num);

         uBIG_t g = gcd(n,d);
         n /= g;
         d /= g;

         int8_t s;
         if (sign == obj.sign) {
            s = 1;
         }
         else {
            s = -1;
         }

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      void print(void) {
         printf("%c%ld:%ld/%ld", sign > 0 ? '+' : '-', whl, labs(num), labs(den));
      }

      operator double() const {
         return ((double)sign * ((double)whl + ((double)num / (double)den)));
      }
};

int main(int argc, char **argv) {
   char buf[1024];
   uint64_t a,b,c,d,e,f;
   char sa, sd;
   char x;
   while (gets(buf)) {
      if (9 == sscanf(buf, "%c%lu:%lu/%lu %c %c%lu:%lu/%lu\n",
                     &sa, &a, &b, &c, &x, &sd, &d, &e, &f)) {
         Rational l(sa == '+' ? 1 : -1, a, b, c);
         Rational r(sd == '+' ? 1 : -1, d, e, f);

         l.print(); printf(" %c ", x); r.print(); printf("\n");
         if (x == '+') {
            Rational x = l + r; x.print(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '-') {
            Rational x = l - r; x.print(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '*') {
            Rational x = l * r; x.print(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '/') {
            Rational x = l / r; x.print(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         printf("\n");
      }
   }
}
