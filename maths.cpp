#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

extern "C" {
   char *gets(char *);
}

static int64_t gcd(int64_t x, int64_t y) {
   // euclid
   int64_t a = x;
   int64_t b = y;
//   if (y > x) {
//      a = y;
//      b = x;
//   }
   int64_t c = a % b;
   while (c != 0) {
      a = b;
      b = c;
      c = a % b;
   }
   return b;
}

static int64_t lcm(int64_t x, int64_t y) {
   return (x * y) / gcd(x,y);
}

class Rational {
   private:
      int8_t sign;
      uint64_t whl;
      uint64_t num;
      uint64_t den;

   public:
      Rational() {
         sign = 1;
         whl = 0;
         num = 0;
         den = 1;
      }

      Rational(int8_t s, uint64_t w, uint64_t n, uint64_t d) {
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
         int64_t n =
            whl * den * obj.den * sign +
            obj.whl * den * obj.den * obj.sign +
            num * obj.den * sign +
            obj.num * den * obj.sign;
         int64_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         int64_t g = gcd(n,d);
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
         int64_t n =
            whl * den * obj.den * sign -
            obj.whl * den * obj.den * obj.sign +
            num * obj.den * sign -
            obj.num * den * obj.sign;
         int64_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         int64_t g = gcd(n,d);
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
         int64_t n =
            whl * obj.whl * den * obj.den +
            whl * obj.num * den  +
            obj.whl * num * obj.den  +
            num * obj.num;
         n *= sign * obj.sign;
         int64_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         int64_t g = gcd(n,d);
         n /= g;
         d /= g;

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      Rational operator / (Rational const & obj) {
         Rational res;

         // TODO FIX can probably be done in a way that avoids overflow
         int64_t n =
            obj.den * sign * (whl * den + num);
         int64_t d =
            den * obj.sign * (obj.whl * obj.den + obj.num);

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         int64_t g = gcd(n,d);
         n /= g;
         d /= g;

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      void print(void) {
         printf("%c%ld:%ld/%ld", sign > 0 ? '+' : '-', whl, labs(num), den);
      }
};

int main(int argc, char **argv) {
   char buf[1024];
   int a,b,c,d,e,f;
   char sa, sd;
   char x;
   while (gets(buf)) {
      if (9 == sscanf(buf, "%c%d:%d/%d %c %c%d:%d/%d\n",
                     &sa, &a, &b, &c, &x, &sd, &d, &e, &f)) {
         Rational l(sa == '+' ? 1 : -1, a, b, c);
         Rational r(sd == '+' ? 1 : -1, d, e, f);

         l.print(); printf(" %c ", x); r.print(); printf("\n");
         if (x == '+') {
            Rational x = l + r; x.print(); printf("\n");
         }
         if (x == '-') {
            Rational x = l - r; x.print(); printf("\n");
         }
         if (x == '*') {
            Rational x = l * r; x.print(); printf("\n");
         }
         if (x == '/') {
            Rational x = l / r; x.print(); printf("\n");
         }

      }
   }
}
