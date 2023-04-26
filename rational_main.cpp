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

      char *fl;
      char *fr;

      if (3 == sscanf(buf, "%s %s %s\n", bufl, op, bufr)) {
         Rational l(bufl);
         Rational r(bufr);

         printf("== input ==\n");

         printf("debu_print: %s %s %s\n",
            fl = /*assign*/ l.debu_print(), op, fr = /*assign*/ r.debu_print());
         free((void *)fl);
         free((void *)fr);

         printf("frac_print: %s %s %s\n",
            fl = /*assign*/ l.frac_print(), op, fr = /*assign*/ r.frac_print());
         free((void *)fl);
         free((void *)fr);

         printf("deci_print: %s %s %s\n",
            fl = /*assign*/ l.deci_print(), op, fr = /*assign*/ r.deci_print());
         free((void *)fl);
         free((void *)fr);

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Rational x = l + r;
               double result = (double) l + (double) r;
               printf("debu : %s\n", fl = /*assign*/ x.debu_print());
               free((void *)fl);
               printf("frac : %s\n", fl = /*assign*/ x.frac_print());
               free((void *)fl);
               printf("deci : %s\n", fl = /*assign*/ x.deci_print());
               free((void *)fl);
               printf("(bad) cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("(bad) dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "-")) {
               Rational x = l - r;
               double result = (double) l - (double) r;
               printf("debu : %s\n", fl = /*assign*/ x.debu_print());
               free((void *)fl);
               printf("frac : %s\n", fl = /*assign*/ x.frac_print());
               free((void *)fl);
               printf("deci : %s\n", fl = /*assign*/ x.deci_print());
               free((void *)fl);
               printf("(bad) cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("(bad) dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "*")) {
               Rational x = l * r;
               double result = (double) l * (double) r;
               printf("debu : %s\n", fl = /*assign*/ x.debu_print());
               free((void *)fl);
               printf("frac : %s\n", fl = /*assign*/ x.frac_print());
               free((void *)fl);
               printf("deci : %s\n", fl = /*assign*/ x.deci_print());
               free((void *)fl);
               printf("(bad) cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("(bad) dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "/")) {
               Rational x = l / r;
               double result = (double) l / (double) r;
               printf("debu : %s\n", fl = /*assign*/ x.debu_print());
               free((void *)fl);
               printf("frac : %s\n", fl = /*assign*/ x.frac_print());
               free((void *)fl);
               printf("deci : %s\n", fl = /*assign*/ x.deci_print());
               free((void *)fl);
               printf("(bad) cast : %0.16f %s %0.16f = %0.16f\n", (double) l, op, (double) r, (double) x);
               printf("(bad) dble : %0.16f\n", (double) result);
            }
            if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
            if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
            if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
            if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
            if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
            if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("%s %s %s == %s\n",
                  fl = /*assign*/ l.frac_print(),
                  op,
                  fr = /*assign*/ r.frac_print(),
                  result ? "true" : "false");
               free((void *)fl);
               free((void *)fr);
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }
   }
}
