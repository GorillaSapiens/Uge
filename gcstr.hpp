#ifndef _INCLUDE_UGE_GCSTR_H_
#define _INCLUDE_UGE_GCSTR_H_

#include <stdlib.h>

namespace uge {

   /// A helper class to assist with garbage collection of pointers.
   class _gcstr {
      private:
         const char *p;
      public:
         _gcstr(const char *_p) {
            p = _p;
         }

         ~_gcstr() {
            if (p) {
               free((void *)p);
            }
         }

         operator const char*() {
            return p;
         }
   };
};

/// example usage:
///
/// printf("%s\n", GCSTR strdup("hello world"));
///
/// would make sure the malloc'd string "hello world" will
/// get free'd after the printf() call.

#define GCSTR (const char *)(_gcstr)

#endif // _INCLUDE_UGE_GCSTR_H_
