#ifndef _INCLUDE_GCSTR_H_
#define _INCLUDE_GCSTR_H_

#include <stdlib.h>

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

#define GCSTR (const char *)(_gcstr)

#endif // _INCLUDE_GCSTR_H_
