#ifndef _INCLUDE_UGE_RAMPRINTF_H_
#define _INCLUDE_UGE_RAMPRINTF_H_

#include <stdarg.h>

namespace uge {

   // returns malloc'd pointer that must be free'd
   char *mprintf(const char *fmt, ...);

   // realloc and append to origin
   char *raprintf(char *&origin, const char *fmt, ...);

   // reverse a string in place
   void strrev(char *str);

   // returns malloc'd pointer that must be free'd
   char *mgets(void);

};

#endif // _INCLUDE_UGE_RAMPRINTF_H_
