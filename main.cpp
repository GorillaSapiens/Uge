#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>
#include <assert.h>

#include "rational.hpp"

// not sure why this is needed, maybe deprecateed
extern "C" {
   char *gets(char *);
}

int main(int argc, char **argv) {
   char buf[1024];
   char x;
   while (gets(buf)) {
      char bufl[512];
      char op[32];
      char bufr[512];

      char outputl[1024];
      char outputr[1024];

      if (3 == sscanf(buf, "%s %s %s\n", bufl, op, bufr)) {
         Rational l(bufl);
         Rational r(bufr);

         printf("%s %s %s\n",
            l.print(outputl, sizeof(outputl)), op, r.print(outputr, sizeof(outputr)));

         printf("%s %s %s\n",
            l.shortprint(outputl, sizeof(outputl)), op, r.shortprint(outputr, sizeof(outputr)));


         try {
            if (!strcmp(op, "+")) {
               Rational x = l + r;
               double result = (double) l + (double) r;
               printf("%s\n", x.print(outputl, sizeof(outputl)));
               printf("%s\n", x.shortprint(outputl, sizeof(outputl)));
               printf("%0.16f %s %0.16f =\n%0.16f // double\n", (double) l, op, (double) r, result);
               printf("%0.16f // rational\n", (double) x);
            }
            if (!strcmp(op, "-")) {
               Rational x = l - r;
               double result = (double) l - (double) r;
               printf("%s\n", x.print(outputl, sizeof(outputl)));
               printf("%s\n", x.shortprint(outputl, sizeof(outputl)));
               printf("%0.16f %s %0.16f =\n%0.16f // double\n", (double) l, op, (double) r, result);
               printf("%0.16f // rational\n", (double) x);
            }
            if (!strcmp(op, "*")) {
               Rational x = l * r;
               double result = (double) l * (double) r;
               printf("%s\n", x.print(outputl, sizeof(outputl)));
               printf("%s\n", x.shortprint(outputl, sizeof(outputl)));
               printf("%0.16f %s %0.16f =\n%0.16f // double\n", (double) l, op, (double) r, result);
               printf("%0.16f // rational\n", (double) x);
            }
            if (!strcmp(op, "/")) {
               Rational x = l / r;
               double result = (double) l / (double) r;
               printf("%s\n", x.print(outputl, sizeof(outputl)));
               printf("%s\n", x.shortprint(outputl, sizeof(outputl)));
               printf("%0.16f %s %0.16f =\n%0.16f // double\n", (double) l, op, (double) r, result);
               printf("%0.16f // rational\n", (double) x);
            }
            if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("%s %s %s == %s\n",
                  l.shortprint(outputl, sizeof(outputl)),
                  op,
                  r.shortprint(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }
   }
}
