#include <stdio.h>
#include <printf.h>

static const char *powers[] = {
   #include "powers.h"
};

int Y_handler (void) { //(FILE *stream, const struct printf_info *info, const void *const *args) {
   unsigned char reverse_result[LENGTH128] =  { 0 };
   char result[LENGTH128];

   //__int128 thing = 8675309008675309;
   //__int128 thing =  -1;
   //__int128 thing =  0;
   __int128 thing = 2;

   __int128 x = 1;
   for (int i = 0; i < 128; i++) {
      if (thing & x) {
         for (int spot = 0; powers[i][spot] != 0; spot++) {
            int sum = ((int) reverse_result[spot]) + (int)(powers[i][spot] - '0');
            reverse_result[spot] = sum % 10;
            reverse_result[spot + 1] += sum / 10;
         }
      }
      x <<= 1;
   }

   // let's straighten things out, shall we?
   int spot = 0;
   int i;
   for (i = LENGTH128 - 1; i >= 0; i--) {
      if (reverse_result[i] != 0) {
         break;
      }
   }
   if (i == -1) {
      result[0] = '0';
      result[1] = 0;
   }
   else {
      while (i >= 0) {
         result[spot++] = reverse_result[i--] + '0';
      }
      result[spot] = 0;
   }
   printf("|%s|\n", result);
}

//int Y_parse_printf_format (const printf_info *templat, size_t n, int *argtypes) {
//}

void main(void) {
   Y_handler();
}
