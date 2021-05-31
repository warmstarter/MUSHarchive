#ifndef __GLOBALS_H
#define __GLOBALS_H

#include <ctype.h>

#ifdef HAS_SAFE_TOUPPER
#define DOWNCASE(x)	tolower(x)
#define UPCASE(x)	toupper(x)
#else
#define DOWNCASE(x) ((isascii(x) && isupper(x)) ? tolower(x) : (x))
#define UPCASE(x)   ((isascii(x) && islower(x)) ? toupper(x) : (x))
#endif

#endif				/* __GLOBALS_H */
