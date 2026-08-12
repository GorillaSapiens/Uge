#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <assert.h>

#include "gcstr.hpp"
#include "uge_ramprintf.hpp"
#include "uge_n.hpp"

using namespace uge;

int main(int argc, char **argv) {
   int ibase = 10, obase = 10;
   char *p;

   while (p = /*assign*/ mgets()) {
      char *bufl, *op, *bufr;
      int res = 0;

      bufl = p;
      res = 1;
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

      if (-1 == res) {
         printf("exiting\n");
         return 0;
      }
      else if (1 == res) {
         N l(bufl, ibase);

         printf("== input ==\n");
         printf("print: %s\n", GCSTR l.print(ibase));
         printf("== output ==\n");
         printf("print: %s\n", GCSTR l.print(obase));
         printf("== done==\n\n");
      }
      else if (2 == res) {
         if (!strcmp(bufl, "sqrt")) {
            N l(op, ibase);

            printf("== input ==\n");
            printf("print: sqrt %s\n", GCSTR l.print(ibase));
            printf("== result==\n");
            printf("sqrt : %s\n", GCSTR l.sqrt().print(obase));
            printf("\n");
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
      else if (3 == res) {
         N l(bufl, ibase);
         N r(bufr, ibase);

         printf("== input ==\n");
         printf("print: %s %s %s\n", GCSTR l.print(ibase), op, GCSTR r.print(ibase));

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               N x = l + r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "-")) {
               N x = l - r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "*")) {
               N x = l * r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "/")) {
               N x = l / r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "%")) {
               N x = l % r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "&")) {
               N x = l & r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "|")) {
               N x = l | r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "^")) {
               N x = l ^ r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "<<")) {
               N x = l << (uint64_t) r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, ">>")) {
               N x = l >> (uint64_t) r;
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "**")) {
               N x = l.pow(r);
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "root")) {
               N x = l.root(r);
               printf("prnt : %s\n", GCSTR x.print(obase));
            }
            else if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s\n", result ? "true" : "false");
            }
            else if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s\n", result ? "true" : "false");
            }
            else if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s\n", result ? "true" : "false");
            }
            else if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s\n", result ? "true" : "false");
            }
            else if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s\n", result ? "true" : "false");
            }
            else if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("== %s\n", result ? "true" : "false");
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
