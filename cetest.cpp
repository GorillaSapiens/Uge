#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "gcstr.hpp"
#include "uge_ce.hpp"
#include "uge_ramprintf.hpp"

#define PRECISION 256

using namespace uge;

static Ce parse_ce(const char *p) {
   if (!strcmp(p, "i")) return Ce(Q((int64_t)0), Q((int64_t)1));
   if (!strcmp(p, "-i")) return Ce(Q((int64_t)0), Q((int64_t)-1));
   const char *comma = strchr(p, ',');
   if (!comma) return Ce(Q(p));
   std::string rs(p, comma - p);
   std::string is(comma + 1);
   return Ce(Q(rs.c_str()), Q(is.c_str()));
}

static void show(const Ce &v) {
   printf("value  : %s\n", GCSTR v.print());
   printf("error  : %s\n", GCSTR v.error().print());
   printf("ierror : %s\n", GCSTR v.ierror().print());
   printf("exact  : %s\n", v.exact() ? "yes" : "no");
}

int main(void) {
   char *p;
   while ((p = mgets())) {
      size_t len = strlen(p);
      while (len && (p[len - 1] == '\n' || p[len - 1] == '\r')) p[--len] = 0;
      if (!len) { free(p); continue; }
      try {
         char *space = strchr(p, ' ');
         if (!space) {
            show(parse_ce(p));
         }
         else {
            *space++ = 0;
            Ce x = parse_ce(space);
            if (!strcmp(p, "sqrt")) show(x.sqrt(PRECISION));
            else if (!strcmp(p, "ln")) show(x.ln(PRECISION));
            else if (!strcmp(p, "e")) show(x.e(PRECISION));
            else if (!strcmp(p, "sin")) show(x.sin(PRECISION));
            else if (!strcmp(p, "cos")) show(x.cos(PRECISION));
            else if (!strcmp(p, "tan")) show(x.tan(PRECISION));
            else if (!strcmp(p, "atan")) show(x.atan(PRECISION));
            else if (!strcmp(p, "sinpi")) show(x.sinpi(PRECISION));
            else if (!strcmp(p, "cospi")) show(x.cospi(PRECISION));
            else if (!strcmp(p, "sintau")) show(x.sintau(PRECISION));
            else if (!strcmp(p, "costau")) show(x.costau(PRECISION));
            else if (!strcmp(p, "abs")) show(x.abs(PRECISION));
            else if (!strcmp(p, "arg")) show(x.arg(PRECISION));
            else if (!strcmp(p, "norm")) show(x.norm());
            else if (!strcmp(p, "conj")) show(x.conj());
            else printf("unknown operation\n");
         }
      }
      catch (const std::string &e) {
         printf("error: %s\n", e.c_str());
      }
      free(p);
   }
   return 0;
}
