#include <stdio.h>
#include <string.h>

static const char *powers[] = {
   #include "powers.h"
};

int Y_handler (char *buffer, __int128 value) {
   unsigned char reverse_result[LENGTH128] =  { 0 };

   __int128 x = 1;
   for (int i = 0; i < 128; i++) {
      if (value & x) {
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
      buffer[0] = '0';
      buffer[1] = 0;
   }
   else {
      while (i >= 0) {
         buffer[spot++] = reverse_result[i--] + '0';
      }
      buffer[spot] = 0;
   }
   return strlen(buffer);
}
