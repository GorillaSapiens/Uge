#ifndef _INCLUDE_RAMPRINTF_H_
#define _INCLUDE_RAMPRINTF_H_

#include <stdarg.h>

// returns malloc'd pointer that must be free'd
char *mprintf(const char *fmt, ...);

// realloc and append to origin
char *raprintf(char *&origin, const char *fmt, ...);

// reverse a string in place
void strrev(char *str);

#endif
