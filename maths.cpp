// in development, so we're a little loosey goosey here...

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
   sBIG_t c = a % b; // TODO FIX divide by zero
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

      void simplify(void) {
         uBIG_t nn = (uBIG_t) whl * (uBIG_t) den + (uBIG_t) num;
         uBIG_t dd = (uBIG_t) den;

         uBIG_t g = gcd(nn,dd);
         nn /= g;
         dd /= g;

         whl = nn / dd;  // TODO FIX possible divide by zero here
         num = nn % dd;  // TODO FIX possible divide by zero here
         den = dd;
      }

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

         simplify();
      }

      Rational(const char *p) {
         // i hate writing hand parsers
         sign = 1;
         if (*p == '-') {
            sign = -1;
            p++;
         }
         if (*p == '+') {
            p++;
         }
         // ok
         char *colon = strchr((char *)p, ':');
         char *slash = strchr((char *)p, '/');
         if (!colon) {
            if (!slash) {
               whl = atoi(p);
               num = 0;
               den = 1;
            }
            else {
               whl = 0;
               sscanf(p, "%ld/%ld", &num, &den);
            }
         }
         else {
            whl = atoi(p);
            sscanf(colon + 1, "%ld/%ld", &num, &den);
         }

         simplify();
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
         res.whl = n / d;  // TODO FIX possible divide by zero here
         res.num = n % d;  // TODO FIX possible divide by zero here
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
         res.whl = n / d;  // TODO FIX possible divide by zero here
         res.num = n % d;  // TODO FIX possible divide by zero here
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
         res.whl = n / d;  // TODO FIX possible divide by zero here
         res.num = n % d;  // TODO FIX possible divide by zero here
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
         res.whl = n / d;  // TODO FIX possible divide by zero here
         res.num = n % d;  // TODO FIX possible divide by zero here
         res.den = d;

         return res;
      }

      void print(void) {
         printf("%c%ld:%ld/%ld", sign > 0 ? '+' : '-', whl, labs(num), labs(den));
      }

      void prettyprint(void) {
         if (whl == 0 && num == 0) {
            printf("0");
            return;
         }
         if (sign < 0) {
            printf("-");
         }
         if (num == 0) {
            printf("%ld", whl);
            return;
         }
         if (whl == 0) {
            printf("%ld/%ld", num, den);
            return;
         }
         printf("%ld:%ld/%ld", whl, num, den);
      }

      operator double() const {
         return ((double)sign * ((double)whl + ((double)num / (double)den)));
      }
};

int main(int argc, char **argv) {
   char buf[1024];
   char x;
   while (gets(buf)) {
      char bufl[512];
      char bufr[512];
      if (3 == sscanf(buf, "%s %c %s\n", bufl, &x, bufr)) {
         Rational l(bufl);
         Rational r(bufr);

         l.print(); printf(" %c ", x); r.print(); printf("\n");
         l.prettyprint(); printf(" %c ", x); r.prettyprint(); printf("\n");

         if (x == '+') {
            Rational x = l + r; x.print(); printf("\n");
            x.prettyprint(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '-') {
            Rational x = l - r; x.print(); printf("\n");
            x.prettyprint(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '*') {
            Rational x = l * r; x.print(); printf("\n");
            x.prettyprint(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         if (x == '/') {
            Rational x = l / r; x.print(); printf("\n");
            x.prettyprint(); printf("\n");
            printf("%0.16f\n", (double) x);
         }
         printf("\n");
      }
   }
}
