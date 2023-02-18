#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>

#ifdef RM128
#ifndef __GNUC__
#error "RM128 only supported under GCC"
#endif
#include "y.h"
#endif

// gcc specific
#ifdef RM128
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

typedef int128_t intBIG_t;
typedef uint128_t uintBIG_t;
#else
typedef int64_t intBIG_t;
typedef uint64_t uintBIG_t;
#endif

extern "C" {
   char *gets(char *);
}

static intBIG_t gcd(intBIG_t x, intBIG_t y) {
   // euclid
   intBIG_t a = x;
   intBIG_t b = y;
//   if (y > x) {
//      a = y;
//      b = x;
//   }
   intBIG_t c = a % b;
   while (c != 0) {
      a = b;
      b = c;
      c = a % b;
   }
   return b;
}

static intBIG_t lcm(intBIG_t x, intBIG_t y) {
   return (x * y) / gcd(x,y);
}

class Rational {
   private:
      int8_t sign;
      uintBIG_t whl;
      uintBIG_t num;
      uintBIG_t den;

   public:
      Rational() {
         sign = 1;
         whl = 0;
         num = 0;
         den = 1;
      }

      Rational(int8_t s, uintBIG_t w, uintBIG_t n, uintBIG_t d) {
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
         intBIG_t n =
            whl * den * obj.den * sign +
            obj.whl * den * obj.den * obj.sign +
            num * obj.den * sign +
            obj.num * den * obj.sign;
         intBIG_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         intBIG_t g = gcd(n,d);
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
         intBIG_t n =
            whl * den * obj.den * sign -
            obj.whl * den * obj.den * obj.sign +
            num * obj.den * sign -
            obj.num * den * obj.sign;
         intBIG_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         intBIG_t g = gcd(n,d);
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
         intBIG_t n =
            whl * obj.whl * den * obj.den +
            whl * obj.num * den  +
            obj.whl * num * obj.den  +
            num * obj.num;
         n *= sign * obj.sign;
         intBIG_t d = den * obj.den;

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         intBIG_t g = gcd(n,d);
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
         intBIG_t n =
            obj.den * sign * (whl * den + num);
         intBIG_t d =
            den * obj.sign * (obj.whl * obj.den + obj.num);

         int8_t s;
         if (n * d >= 0) {
            s = 1;
         }
         else {
            s = -1;
         }

         intBIG_t g = gcd(n,d);
         n /= g;
         d /= g;

         res.sign = s;
         res.whl = n / d;
         res.num = n % d;
         res.den = d;

         return res;
      }

      void print(void) {
#ifdef RM128
         char whl_buffer[64]; Y_handler(whl_buffer, whl);
         char num_buffer[64]; Y_handler(num_buffer, num);
         char den_buffer[64]; Y_handler(den_buffer, den);
         printf("%c%s:%s/%s", sign > 0 ? '+' : '-', whl_buffer, num_buffer, den_buffer);
#else
         printf("%c%ld:%ld/%ld", sign > 0 ? '+' : '-', whl, labs(num), labs(den));
#endif
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
         printf("\n");
      }
   }
}
