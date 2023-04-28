#ifndef _INCLUDE_UGE_GCSTR_H_
#define _INCLUDE_UGE_GCSTR_H_

#include <stdlib.h>

namespace uge {
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

#define GCSTR (const char *)(_gcstr)

#endif // _INCLUDE_UGE_GCSTR_H_
