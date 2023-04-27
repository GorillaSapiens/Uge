#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>
#include <assert.h>

#include "gcstr.hpp"
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

      int res = sscanf(buf, "%s %s %s\n", bufl, op, bufr);

      if (-1 == res) {
         printf("exiting\n");
         return 0;
      }
      else if (1 == res) {
         Rational l(bufl);

         printf("== input ==\n");

         printf("debu_print: %s\n", GCSTR l.debu_print());
         printf("frac_print: %s\n", GCSTR l.frac_print());
         printf("deci_print: %s\n", GCSTR l.deci_print());

         printf("== done==\n\n");
      }
      else if (3 == res) {
         Rational l(bufl);
         Rational r(bufr);

         printf("== input ==\n");

         printf("debu_print: %s %s %s\n", GCSTR l.debu_print(), op, GCSTR r.debu_print());
         printf("frac_print: %s %s %s\n", GCSTR l.frac_print(), op, GCSTR r.frac_print());
         printf("deci_print: %s %s %s\n", GCSTR l.deci_print(), op, GCSTR r.deci_print());

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Rational x = l + r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            if (!strcmp(op, "-")) {
               Rational x = l - r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            if (!strcmp(op, "*")) {
               Rational x = l * r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            if (!strcmp(op, "/")) {
               Rational x = l / r;
               printf("debu : %s\n", GCSTR x.debu_print());
               printf("frac : %s\n", GCSTR x.frac_print());
               printf("deci : %s\n", GCSTR x.deci_print());
            }
            if (!strcmp(op, "==")) {
               bool result = (l == r);
               printf("%s\n", result ? "true" : "false");
            }
            if (!strcmp(op, "!=")) {
               bool result = (l != r);
               printf("%s\n", result ? "true" : "false");
            }
            if (!strcmp(op, "<")) {
               bool result = (l < r);
               printf("%s\n", result ? "true" : "false");
            }
            if (!strcmp(op, ">")) {
               bool result = (l > r);
               printf("%s\n", result ? "true" : "false");
            }
            if (!strcmp(op, "<=")) {
               bool result = (l <= r);
               printf("%s\n", result ? "true" : "false");
            }
            if (!strcmp(op, ">=")) {
               bool result = (l >= r);
               printf("%s\n", result ? "true" : "false");
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }
   }
}
