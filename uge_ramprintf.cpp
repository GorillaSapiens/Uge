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
      int n = vsnprintf(NULL, 0, fmt, ap);
      va_end(ap);

      if (n < 0) {
         return NULL;
      }

      size_t size = (size_t)n;
      size_t oldsize = origin ? strlen(origin) : 0;

      char *ret = (char *)realloc(origin, oldsize + size + 1);
      if (!ret) {
         return NULL;
      }

      origin = ret;

      va_start(ap, fmt);
      vsnprintf(origin + oldsize, size + 1, fmt, ap);
      va_end(ap);

      return origin;
   }

   // realloc and prepend to origin
   char *rpprintf(char *&origin, const char *fmt, ...) {
      va_list ap;

      va_start(ap, fmt);
      int n = vsnprintf(NULL, 0, fmt, ap);
      va_end(ap);

      if (n < 0) {
         return NULL;
      }

      size_t size = (size_t)n;
      size_t oldsize = origin ? strlen(origin) : 0;

      char *ret = (char *)realloc(origin, size + oldsize + 1);
      if (!ret) {
         return NULL;
      }

      origin = ret;

      if (oldsize) {
         memmove(origin + size, origin, oldsize + 1);
      }
      else {
         origin[size] = '\0';
      }

      char first = origin[size];

      va_start(ap, fmt);
      vsnprintf(origin, size + 1, fmt, ap);
      va_end(ap);

      origin[size] = first;

      return origin;
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

// vim: expandtab:noai:ts=3:sw=3
