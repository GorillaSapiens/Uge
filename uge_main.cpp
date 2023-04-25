#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>
#include <assert.h>

#include "uge.hpp"

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
         Uge l(bufl);
         Uge r(bufr);

         printf("== input ==\n");

         printf("debu_print: %s %s %s\n",
            l.debu_print(outputl, sizeof(outputl)), op, r.debu_print(outputr, sizeof(outputr)));

         printf("frac_print: %s %s %s\n",
            l.frac_print(outputl, sizeof(outputl)), op, r.frac_print(outputr, sizeof(outputr)));

         printf("deci_print: %s %s %s\n",
            l.deci_print(outputl, sizeof(outputl)), op, r.deci_print(outputr, sizeof(outputr)));

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Uge x = l + r;
               double result = (double) l + (double) r;
               printf("debu : %s\n", x.debu_print(outputl, sizeof(outputl)));
               printf("frac : %s\n", x.frac_print(outputl, sizeof(outputl)));
               printf("deci : %s\n", x.deci_print(outputl, sizeof(outputl)));
               printf("cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "-")) {
               Uge x = l - r;
               double result = (double) l - (double) r;
               printf("debu : %s\n", x.debu_print(outputl, sizeof(outputl)));
               printf("frac : %s\n", x.frac_print(outputl, sizeof(outputl)));
               printf("deci : %s\n", x.deci_print(outputl, sizeof(outputl)));
               printf("cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "*")) {
               Uge x = l * r;
               double result = (double) l * (double) r;
               printf("debu : %s\n", x.debu_print(outputl, sizeof(outputl)));
               printf("frac : %s\n", x.frac_print(outputl, sizeof(outputl)));
               printf("deci : %s\n", x.deci_print(outputl, sizeof(outputl)));
               printf("cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "/")) {
               Uge x = l / r;
               double result = (double) l / (double) r;
               printf("debu : %s\n", x.debu_print(outputl, sizeof(outputl)));
               printf("frac : %s\n", x.frac_print(outputl, sizeof(outputl)));
               printf("deci : %s\n", x.deci_print(outputl, sizeof(outputl)));
               printf("cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("%s %s %s == %s\n",
                  l.frac_print(outputl, sizeof(outputl)),
                  op,
                  r.frac_print(outputr, sizeof(outputr)),
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
