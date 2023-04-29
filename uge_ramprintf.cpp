#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "uge_ramprintf.hpp"

namespace uge {

   // returns malloc'd pointer that must be free'd
   char *mprintf(const char *fmt, ...) {
      va_list ap;
      va_start(ap, fmt);
      int size = 1 + vsnprintf(NULL, 0, fmt, ap);
      va_end(ap);

      char *ret = (char *) malloc(sizeof(char) * size);

      va_start(ap, fmt);
      vsnprintf(ret, size, fmt, ap);
      va_end(ap);

      return ret;
   }

   // realloc and append to origin
   char *raprintf(char *&origin, const char *fmt, ...) {
      va_list ap;
      va_start(ap, fmt);
      int size = 1 + vsnprintf(NULL, 0, fmt, ap);
      va_end(ap);

      int oldsize = origin ? strlen(origin) : 0;
      char *ret = (char *) realloc((void *) origin, sizeof(char) * (size + oldsize));
      origin = ret;

      va_start(ap, fmt);
      vsnprintf(ret + oldsize, size, fmt, ap);
      va_end(ap);

      return ret;
   }

   void strrev(char *str) {
      char tmp;
      size_t i = 0;
      size_t j = strlen(str) - 1;
      while (i < j) {
         tmp = str[i];
         str[i++] = str[j];
         str[j--] = tmp;
      }
   }

   char *mgets(void) {
      char *ret = NULL;
      int c;

      do {
         c = getc(stdin);
         if (c != EOF) {
            raprintf(ret, "%c", c);
         }
      } while (c != EOF && c != '\n');

      return ret;
   }

};
