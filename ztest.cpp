#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <printf.h>
#include <string.h>
#include <assert.h>

#include "uge_gcstr.hpp"
#include "uge_z.hpp"

using namespace uge;

// not sure why this is needed, maybe deprecateed
extern "C" {
   char *gets(char *);
}

int main(int argc, char **argv) {
   char buf[1024];
   char x;
   while (gets(buf)) {
      char bufl[512];
      char op[512];
      char bufr[512];

      int res = sscanf(buf, "%s %s %s\n", bufl, op, bufr);

      if (-1 == res) {
         printf("exiting\n");
         return 0;
      }
      else if (1 == res) {
         Z l(bufl);

         printf("== input ==\n");
         printf("print: %s\n", GCSTR l.print());
         printf("== done==\n\n");
      }
      else if (2 == res) {
         if (!strcmp(bufl, "sqrt")) {
            Z l(op);

         printf("== input ==\n");
         printf("print: sqrt %s\n", GCSTR l.print());
         printf("== result==\n");
         printf("sqrt : %s\n", GCSTR l.sqrt().print());
         printf("\n");
         }
      }
      else if (3 == res) {
         Z l(bufl);
         Z r(bufr);

         printf("== input ==\n");
         printf("print: %s %s %s\n", GCSTR l.print(), op, GCSTR r.print());

         printf("== result ==\n");

         try {
            if (!strcmp(op, "+")) {
               Z x = l + r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, "-")) {
               Z x = l - r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, "*")) {
               Z x = l * r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, "/")) {
               Z x = l / r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, "%")) {
               Z x = l % r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, "<<")) {
               Z x = l << (uint64_t) r;
               printf("prnt : %s\n", GCSTR x.print());
            }
            if (!strcmp(op, ">>")) {
               Z x = l >> (uint64_t) r;
               printf("prnt : %s\n", GCSTR x.print());
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
               printf("== %s\n", result ? "true" : "false");
            }
         }
         catch (std::string e) {
            std::cerr << e;
         }
         printf("\n");
      }
   }
}
