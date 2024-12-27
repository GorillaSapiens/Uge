#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <assert.h>

#include "uge_ramprintf.hpp"
#include "gcstr.hpp"
#include "uge_q.hpp"

#define PRECISION 4096

using namespace uge;

int main(int argc, char **argv) {
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
         Q l(bufl);

         printf("== input ==\n");

         printf("debu_print: %s\n", GCSTR l.debu_print());
         printf("frac_print: %s\n", GCSTR l.frac_print());
         printf("deci_print: %s\n", GCSTR l.deci_print());

         Q r = l.sqrt(PRECISION);

         printf("sqrt debu_print: %s\n", GCSTR r.debu_print());
         printf("sqrt frac_print: %s\n", GCSTR r.frac_print());
         printf("sqrt deci_print: %s\n", GCSTR r.deci_print());

         r = ~l;

         printf("~ debu_print: %s\n", GCSTR r.debu_print());
         printf("~ frac_print: %s\n", GCSTR r.frac_print());
         printf("~ deci_print: %s\n", GCSTR r.deci_print());

         printf("== done==\n\n");
      }
      else if (3 == res) {
         Q l(bufl);
         Q r(bufr);

         printf("== input ==\n");
         printf("(%s) (%s) (%s)\n", bufl, op, bufr);

         printf("debu_print: %s %s %s\n", GCSTR l.debu_print(), op, GCSTR r.debu_print());
         printf("frac_print: %s %s %s\n", GCSTR l.frac_print(), op, GCSTR r.frac_print());
         printf("deci_print: %s %s %s\n", GCSTR l.deci_print(), op, GCSTR r.deci_print());

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Q x = l + r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "-")) {
               Q x = l - r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "*")) {
               Q x = l * r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "/")) {
               Q x = l / r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "%")) {
               Q x = l % r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "&")) {
               Q x = l & r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "|")) {
               Q x = l | r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "^")) {
               Q x = l ^ r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            else if (!strcmp(op, "**")) {
               Q x = l.pow(r, PRECISION);
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
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
               printf("%s\n", result ? "true" : "false");
            }
            else {
               printf("unknown op '%s'\n", op);
            }
            printf("== done==\n\n");
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }

      free(p);
   }
}
// vim: expandtab:noai:ts=3:sw=3
