#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "uge_ramprintf.hpp"
#include "gcstr.hpp"
#include "uge_q.hpp"

#define PRECISION 4096

using namespace uge;

static void print_q(const char *label, const Q &q, uint64_t radix) {
   printf("%sdebu : %s\n", label, GCSTR q.debu_print());
   printf("%sfrac : %s\n", label, GCSTR q.frac_print(radix));
   printf("%sprnt : %s\n", label, GCSTR q.print(radix));
}

int main(void) {
   int ibase = 10, obase = 10;
   char *p;

   while ((p = /*assign*/ mgets())) {
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

      try {
         if (1 == res) {
            if (!strcmp(bufl, "pi") || !strcmp(bufl, "tau")) {
               Q r = !strcmp(bufl, "pi") ? Q::pi(PRECISION) : Q::tau(PRECISION);
               printf("== %s ==\n", bufl);
               print_q("", r, obase);
               printf("\n");
               free(p);
               continue;
            }

            Q l(bufl, ibase);

            printf("== input ==\n");
            print_q("", l, ibase);

            printf("== output ==\n");
            printf("frac : %s\n", GCSTR l.frac_print(obase));
            printf("prnt : %s\n", GCSTR l.print(obase));

            Q r = l.sqrt(PRECISION);
            printf("== sqrt ==\n");
            print_q("sqrt ", r, obase);

            r = ~l;
            printf("== ~ ==\n");
            print_q("~ ", r, obase);

            printf("== done==\n\n");
         }
         else if (2 == res) {
            if (!strcmp(bufl, "sqrt") || !strcmp(bufl, "sin") ||
                !strcmp(bufl, "cos") || !strcmp(bufl, "tan") ||
                !strcmp(bufl, "atan") || !strcmp(bufl, "sinpi") ||
                !strcmp(bufl, "cospi") || !strcmp(bufl, "tanpi") ||
                !strcmp(bufl, "atanpi") || !strcmp(bufl, "sintau") ||
                !strcmp(bufl, "costau") || !strcmp(bufl, "tantau") ||
                !strcmp(bufl, "atantau") || !strcmp(bufl, "sindeg") ||
                !strcmp(bufl, "cosdeg") || !strcmp(bufl, "tandeg") ||
                !strcmp(bufl, "atandeg") || !strcmp(bufl, "ln") ||
                !strcmp(bufl, "e")) {
               Q l(op, ibase);
               Q r;
               if (!strcmp(bufl, "sqrt")) r = l.sqrt(PRECISION);
               else if (!strcmp(bufl, "sin")) r = l.sin(PRECISION);
               else if (!strcmp(bufl, "cos")) r = l.cos(PRECISION);
               else if (!strcmp(bufl, "tan")) r = l.tan(PRECISION);
               else if (!strcmp(bufl, "atan")) r = l.atan(PRECISION);
               else if (!strcmp(bufl, "sinpi")) r = l.sinpi(PRECISION);
               else if (!strcmp(bufl, "cospi")) r = l.cospi(PRECISION);
               else if (!strcmp(bufl, "tanpi")) r = l.tanpi(PRECISION);
               else if (!strcmp(bufl, "atanpi")) r = l.atanpi(PRECISION);
               else if (!strcmp(bufl, "sintau")) r = l.sintau(PRECISION);
               else if (!strcmp(bufl, "costau")) r = l.costau(PRECISION);
               else if (!strcmp(bufl, "tantau")) r = l.tantau(PRECISION);
               else if (!strcmp(bufl, "atantau")) r = l.atantau(PRECISION);
               else if (!strcmp(bufl, "sindeg")) r = l.sindeg(PRECISION);
               else if (!strcmp(bufl, "cosdeg")) r = l.cosdeg(PRECISION);
               else if (!strcmp(bufl, "tandeg")) r = l.tandeg(PRECISION);
               else if (!strcmp(bufl, "atandeg")) r = l.atandeg(PRECISION);
               else if (!strcmp(bufl, "ln")) r = l.ln(PRECISION);
               else r = l.e(PRECISION);

               printf("== input ==\n");
               printf("prnt : %s %s\n", bufl, GCSTR l.print(ibase));
               printf("== result ==\n");
               printf("prnt : %s\n", GCSTR r.print(obase));
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
            if (!strcmp(bufl, "atan2") || !strcmp(bufl, "atan2pi") ||
                !strcmp(bufl, "atan2tau") || !strcmp(bufl, "atan2deg")) {
               Q y(op, ibase);
               Q x(bufr, ibase);
               Q r = !strcmp(bufl, "atan2") ? y.atan2(x, PRECISION) :
                     !strcmp(bufl, "atan2pi") ? y.atan2pi(x, PRECISION) :
                     !strcmp(bufl, "atan2tau") ? y.atan2tau(x, PRECISION) :
                                                 y.atan2deg(x, PRECISION);
               printf("== input ==\n");
               printf("prnt : %s %s %s\n", bufl, GCSTR y.print(ibase), GCSTR x.print(ibase));
               printf("== result ==\n");
               print_q("", r, obase);
               printf("== done==\n\n");
               free(p);
               continue;
            }

            Q l(bufl, ibase);
            Q r(bufr, ibase);

            printf("== input ==\n");
            printf("debu : %s %s %s\n", GCSTR l.debu_print(), op, GCSTR r.debu_print());
            printf("frac : %s %s %s\n", GCSTR l.frac_print(ibase), op, GCSTR r.frac_print(ibase));
            printf("prnt : %s %s %s\n", GCSTR l.print(ibase), op, GCSTR r.print(ibase));

            printf("== result ==\n");

            if (!strcmp(op, "+")) {
               Q x = l + r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "-")) {
               Q x = l - r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "*")) {
               Q x = l * r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "/")) {
               Q x = l / r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "%")) {
               Q x = l % r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "&")) {
               Q x = l & r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "|")) {
               Q x = l | r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "^")) {
               Q x = l ^ r;
               print_q("", x, obase);
            }
            else if (!strcmp(op, "**")) {
               Q x = l.pow(r, PRECISION);
               print_q("", x, obase);
            }
            else if (!strcmp(op, "==")) {
               printf("%s\n", l == r ? "true" : "false");
            }
            else if (!strcmp(op, "!=")) {
               printf("%s\n", l != r ? "true" : "false");
            }
            else if (!strcmp(op, "<")) {
               printf("%s\n", l < r ? "true" : "false");
            }
            else if (!strcmp(op, ">")) {
               printf("%s\n", l > r ? "true" : "false");
            }
            else if (!strcmp(op, "<=")) {
               printf("%s\n", l <= r ? "true" : "false");
            }
            else if (!strcmp(op, ">=")) {
               printf("%s\n", l >= r ? "true" : "false");
            }
            else {
               printf("unknown op '%s'\n", op);
            }
            printf("== done==\n\n");
         }
      }
      catch (std::string e) {
         std::cerr << e;
      }

      free(p);
   }
}

// vim: expandtab:noai:ts=3:sw=3
