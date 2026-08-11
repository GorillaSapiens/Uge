#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "uge_ramprintf.hpp"
#include "gcstr.hpp"
#include "uge_c.hpp"

#define PRECISION 4096

using namespace uge;

// ctest deliberately does not define user-facing complex-number syntax.
// A complex operand is written here as real,imag; a plain operand is real.
// Examples:  1/2,2/3   0,1   7/8
static C parse_c(const char *p, uint64_t radix) {
   if (!strcmp(p, "i")) {
      return C(Q((int64_t)0), Q((int64_t)1));
   }
   if (!strcmp(p, "-i")) {
      return C(Q((int64_t)0), Q((int64_t)-1));
   }

   const char *comma = strchr(p, ',');
   if (!comma) {
      return C(Q(p, radix));
   }

   std::string rs(p, comma - p);
   std::string is(comma + 1);
   if (rs.empty() || is.empty()) {
      throw(std::string("complex test operand must be real,imag\n"));
   }

   return C(Q(rs.c_str(), radix), Q(is.c_str(), radix));
}

static void print_c(const char *label, const C &c, uint64_t radix) {
   printf("%sdebu : %s\n", label, GCSTR c.debu_print());
   printf("%sfrac : %s\n", label, GCSTR c.frac_print(radix));
   printf("%sprnt : %s\n", label, GCSTR c.print(radix));
}

static void print_q(const char *label, const Q &q, uint64_t radix) {
   printf("%sdebu : %s\n", label, GCSTR q.debu_print());
   printf("%sfrac : %s\n", label, GCSTR q.frac_print(radix));
   printf("%sprnt : %s\n", label, GCSTR q.print(radix));
}

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

      try {
         if (1 == res) {
            if (!strcmp(bufl, "pi") || !strcmp(bufl, "tau")) {
               C r = !strcmp(bufl, "pi") ? C::pi(PRECISION) : C::tau(PRECISION);
               printf("== %s ==\n", bufl);
               print_c("", r, obase);
               printf("\n");
               free(p);
               continue;
            }

            C l = parse_c(bufl, ibase);

            printf("== input ==\n");
            print_c("", l, ibase);

            printf("== output ==\n");
            printf("frac : %s\n", GCSTR l.frac_print(obase));
            printf("prnt : %s\n", GCSTR l.print(obase));

            printf("== components ==\n");
            printf("real : %s\n", GCSTR l.real().print(obase));
            printf("imag : %s\n", GCSTR l.imag().print(obase));

            printf("== done==\n\n");
         }
         else if (2 == res) {
            if (!strcmp(bufl, "sqrt") || !strcmp(bufl, "conj") ||
                !strcmp(bufl, "floor") || !strcmp(bufl, "sin") ||
                !strcmp(bufl, "cos") || !strcmp(bufl, "tan") ||
                !strcmp(bufl, "atan") || !strcmp(bufl, "sinpi") ||
                !strcmp(bufl, "cospi") || !strcmp(bufl, "tanpi") ||
                !strcmp(bufl, "atanpi") || !strcmp(bufl, "sintau") ||
                !strcmp(bufl, "costau") || !strcmp(bufl, "tantau") ||
                !strcmp(bufl, "atantau") || !strcmp(bufl, "sindeg") ||
                !strcmp(bufl, "cosdeg") || !strcmp(bufl, "tandeg") ||
                !strcmp(bufl, "atandeg") || !strcmp(bufl, "ln") ||
                !strcmp(bufl, "e")) {
               C l = parse_c(op, ibase);
               C r;
               if (!strcmp(bufl, "sqrt")) r = l.sqrt(PRECISION);
               else if (!strcmp(bufl, "conj")) r = l.conj();
               else if (!strcmp(bufl, "floor")) r = l.floor();
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
               print_c("", r, obase);
               printf("== done==\n\n");
            }
            else if (!strcmp(bufl, "norm") || !strcmp(bufl, "abs") ||
                     !strcmp(bufl, "arg") || !strcmp(bufl, "real") ||
                     !strcmp(bufl, "imag")) {
               C l = parse_c(op, ibase);
               Q r;
               if (!strcmp(bufl, "norm")) r = l.norm();
               else if (!strcmp(bufl, "abs")) r = l.abs(PRECISION);
               else if (!strcmp(bufl, "arg")) r = l.arg(PRECISION);
               else if (!strcmp(bufl, "real")) r = l.real();
               else r = l.imag();

               printf("== input ==\n");
               printf("prnt : %s %s\n", bufl, GCSTR l.print(ibase));
               printf("== result ==\n");
               print_q("", r, obase);
               printf("== done==\n\n");
            }
            else if (!strcmp(bufl, "sgn")) {
               C l = parse_c(op, ibase);
               printf("== input ==\n");
               printf("prnt : sgn %s\n", GCSTR l.print(ibase));
               printf("== result ==\n");
               printf("%d\n", l.sgn());
               printf("== done==\n\n");
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
               C y = parse_c(op, ibase);
               C x = parse_c(bufr, ibase);
               C r = !strcmp(bufl, "atan2") ? y.atan2(x, PRECISION) :
                     !strcmp(bufl, "atan2pi") ? y.atan2pi(x, PRECISION) :
                     !strcmp(bufl, "atan2tau") ? y.atan2tau(x, PRECISION) :
                                                  y.atan2deg(x, PRECISION);
               printf("== input ==\n");
               printf("prnt : %s %s %s\n", bufl, GCSTR y.print(ibase), GCSTR x.print(ibase));
               printf("== result ==\n");
               print_c("", r, obase);
               printf("== done==\n\n");
               free(p);
               continue;
            }

            C l = parse_c(bufl, ibase);
            C r = parse_c(bufr, ibase);

            printf("== input ==\n");
            printf("debu : %s %s %s\n", GCSTR l.debu_print(), op, GCSTR r.debu_print());
            printf("frac : %s %s %s\n", GCSTR l.frac_print(ibase), op, GCSTR r.frac_print(ibase));
            printf("prnt : %s %s %s\n", GCSTR l.print(ibase), op, GCSTR r.print(ibase));

            printf("== result ==\n");

            if (!strcmp(op, "+")) {
               C x = l + r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "-")) {
               C x = l - r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "*")) {
               C x = l * r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "/")) {
               C x = l / r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "%")) {
               C x = l % r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "&")) {
               C x = l & r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "|")) {
               C x = l | r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "^")) {
               C x = l ^ r;
               print_c("", x, obase);
            }
            else if (!strcmp(op, "<<")) {
               if (!r.is_real()) {
                  throw(std::string("shift count must be real\n"));
               }
               C x = l << (int64_t)r.real();
               print_c("", x, obase);
            }
            else if (!strcmp(op, ">>")) {
               if (!r.is_real()) {
                  throw(std::string("shift count must be real\n"));
               }
               C x = l >> (int64_t)r.real();
               print_c("", x, obase);
            }
            else if (!strcmp(op, "**")) {
               C x = l.pow(r, PRECISION);
               print_c("", x, obase);
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
