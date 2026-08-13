#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "gcstr.hpp"
#include "uge_ramprintf.hpp"
#include "uge_z.hpp"

using namespace uge;

static N nonnegative_exponent(const Z &z) {
   if (z.sgn() < 0) {
      throw std::string("negative exponent/root is not an N\n");
   }
   return z.magnitude();
}

int main(int argc, char **argv) {
   (void)argc;
   (void)argv;

   int ibase = 10, obase = 10;
   char *p;

   while ((p = mgets())) {
      char *bufl, *op, *bufr = NULL;
      int res = 1;

      bufl = p;
      op = strchr(p, ' ');
      if (op) {
         *op++ = 0;
         res = 2;
         bufr = strchr(op, ' ');
         if (bufr) {
            *bufr++ = 0;
            res = 3;
         }
      }

      if (res == 1) {
         Z l(bufl, ibase);

         printf("== input ==\n");
         printf("print: %s\n", GCSTR l.print(ibase));
         printf("== output ==\n");
         printf("print: %s\n", GCSTR l.print(obase));
         printf("debug: %s\n", GCSTR l.dprint());
         printf("== done==\n\n");
      }
      else if (res == 2) {
         try {
            if (!strcmp(bufl, "sqrt")) {
               Z l(op, ibase);
               printf("sqrt : %s\n\n", GCSTR l.sqrt().print(obase));
            }
            else if (!strcmp(bufl, "abs")) {
               Z l(op, ibase);
               printf("abs  : %s\n\n", GCSTR l.abs().print(obase));
            }
            else if (!strcmp(bufl, "sgn")) {
               Z l(op, ibase);
               printf("sgn  : %d\n\n", l.sgn());
            }
            else if (!strcmp(bufl, "~")) {
               Z l(op, ibase);
               printf("prnt : %s\n\n", GCSTR (~l).print(obase));
            }
            else if (!strcmp(bufl, "ibase")) {
               ibase = atoi(op);
               printf("ibase = %d, obase = %d\n", ibase, obase);
            }
            else if (!strcmp(bufl, "obase")) {
               obase = atoi(op);
               printf("ibase = %d, obase = %d\n", ibase, obase);
            }
            else if (!strcmp(bufl, "base")) {
               ibase = obase = atoi(op);
               printf("ibase = %d, obase = %d\n", ibase, obase);
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
      }
      else if (res == 3) {
         Z l(bufl, ibase);
         Z r(bufr, ibase);

         printf("== input ==\n");
         printf("print: %s %s %s\n", GCSTR l.print(ibase), op,
            GCSTR r.print(ibase));
         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               printf("prnt : %s\n", GCSTR (l + r).print(obase));
            }
            else if (!strcmp(op, "-")) {
               printf("prnt : %s\n", GCSTR (l - r).print(obase));
            }
            else if (!strcmp(op, "*")) {
               printf("prnt : %s\n", GCSTR (l * r).print(obase));
            }
            else if (!strcmp(op, "/")) {
               printf("prnt : %s\n", GCSTR (l / r).print(obase));
            }
            else if (!strcmp(op, "%")) {
               printf("prnt : %s\n", GCSTR (l % r).print(obase));
            }
            else if (!strcmp(op, "&")) {
               printf("prnt : %s\n", GCSTR (l & r).print(obase));
            }
            else if (!strcmp(op, "|")) {
               printf("prnt : %s\n", GCSTR (l | r).print(obase));
            }
            else if (!strcmp(op, "^")) {
               printf("prnt : %s\n", GCSTR (l ^ r).print(obase));
            }
            else if (!strcmp(op, "<<")) {
               printf("prnt : %s\n", GCSTR (l << (int64_t)r).print(obase));
            }
            else if (!strcmp(op, ">>")) {
               printf("prnt : %s\n", GCSTR (l >> (int64_t)r).print(obase));
            }
            else if (!strcmp(op, "**")) {
               printf("prnt : %s\n",
                  GCSTR l.pow(nonnegative_exponent(r)).print(obase));
            }
            else if (!strcmp(op, "root")) {
               printf("prnt : %s\n",
                  GCSTR l.root(nonnegative_exponent(r)).print(obase));
            }
            else if (!strcmp(op, "==")) {
               printf("%s\n", (l == r) ? "true" : "false");
            }
            else if (!strcmp(op, "!=")) {
               printf("%s\n", (l != r) ? "true" : "false");
            }
            else if (!strcmp(op, "<")) {
               printf("%s\n", (l < r) ? "true" : "false");
            }
            else if (!strcmp(op, ">")) {
               printf("%s\n", (l > r) ? "true" : "false");
            }
            else if (!strcmp(op, "<=")) {
               printf("%s\n", (l <= r) ? "true" : "false");
            }
            else if (!strcmp(op, ">=")) {
               printf("%s\n", (l >= r) ? "true" : "false");
            }
            else {
               printf("unknown op '%s'\n", op);
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }
   }
}

// vim: expandtab:noai:ts=3:sw=3
