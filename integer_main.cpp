#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>
#include <assert.h>

#include "integer.hpp"

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
         Integer l(bufl);
         Integer r(bufr);

         printf("== input ==\n");

         printf("print: %s %s %s\n",
            l.print(outputl, sizeof(outputl)), op, r.print(outputr, sizeof(outputr)));

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Integer x = l + r;
               uint64_t result = (uint64_t) l + (uint64_t) r;
               printf("prnt : %s\n", x.print(outputl, sizeof(outputl)));
               printf("cast : %ld %s %ld = %ld\n", (uint64_t) l, op, (uint64_t) r, (uint64_t) x);
               printf("intr : %ld\n", (uint64_t) result);
            }
            if (!strcmp(op, "-")) {
               Integer x = l - r;
               uint64_t result = (uint64_t) l - (uint64_t) r;
               printf("prnt : %s\n", x.print(outputl, sizeof(outputl)));
               printf("cast : %ld %s %ld = %ld\n", (uint64_t) l, op, (uint64_t) r, (uint64_t) x);
               printf("intr : %ld\n", (uint64_t) result);
            }
            if (!strcmp(op, "*")) {
               Integer x = l * r;
               uint64_t result = (uint64_t) l * (uint64_t) r;
               printf("prnt : %s\n", x.print(outputl, sizeof(outputl)));
               printf("cast : %ld %s %ld = %ld\n", (uint64_t) l, op, (uint64_t) r, (uint64_t) x);
               printf("intr : %ld\n", (uint64_t) result);
            }
            if (!strcmp(op, "/")) {
               Integer x = l / r;
               uint64_t result = (uint64_t) l / (uint64_t) r;
               printf("prnt : %s\n", x.print(outputl, sizeof(outputl)));
               printf("cast : %ld %s %ld = %ld\n", (uint64_t) l, op, (uint64_t) r, (uint64_t) x);
               printf("intr : %ld\n", (uint64_t) result);
            }
            if (!strcmp(op, "%")) {
               Integer x = l % r;
               uint64_t result = (uint64_t) l % (uint64_t) r;
               printf("prnt : %s\n", x.print(outputl, sizeof(outputl)));
               printf("cast : %ld %s %ld = %ld\n", (uint64_t) l, op, (uint64_t) r, (uint64_t) x);
               printf("intr : %ld\n", (uint64_t) result);
            }
            if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
                  result ? "true" : "false");
            }
            if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("%s %s %s == %s\n",
                  l.print(outputl, sizeof(outputl)),
                  op,
                  r.print(outputr, sizeof(outputr)),
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
