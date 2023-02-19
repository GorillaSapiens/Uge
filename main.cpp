#include <string>
#include <iostream>

#include "rational.h"

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

      if (3 == sscanf(buf, "%s %s %s\n", bufl, op, bufr)) {
         Rational l(bufl);
         Rational r(bufr);

         l.print(); printf(" %s ", op); r.print(); printf("\n");
         l.prettyprint(); printf(" %s ", op); r.prettyprint(); printf("\n");

         try {
            if (!strcmp(op, "+")) {
               Rational x = l + r; x.print(); printf("\n");
               x.prettyprint(); printf("\n");
               printf("%0.16f\n", (double) x);
            }
            if (!strcmp(op, "-")) {
               Rational x = l - r; x.print(); printf("\n");
               x.prettyprint(); printf("\n");
               printf("%0.16f\n", (double) x);
            }
            if (!strcmp(op, "*")) {
               Rational x = l * r; x.print(); printf("\n");
               x.prettyprint(); printf("\n");
               printf("%0.16f\n", (double) x);
            }
            if (!strcmp(op, "/")) {
               Rational x = l / r; x.print(); printf("\n");
               x.prettyprint(); printf("\n");
               printf("%0.16f\n", (double) x);
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
