/* ansi.h */

/* ANSI control codes for various neat-o terminal effects

 * Some older versions of Ultrix don't appear to be able to
 * handle these escape sequences. If lowercase 'a's are being
 * stripped from @doings, and/or the output of the ANSI flag
 * is screwed up, you have the Ultrix problem.
 *
 * To fix the ANSI problem, try replacing the '\x1B' with '\033'.
 * To fix the problem with 'a's, replace all occurrences of '\a'
 * in the code with '\07'.
 *
 */

#ifndef __ANSI_H
#define __ANSI_H

#ifndef OLD_ANSI

#define BEEP_CHAR     '\a'
#define ESC_CHAR      '\x1B'

#define ANSI_NORMAL   "\x1B[0m"

#define ANSI_HILITE   "\x1B[1m"
#define ANSI_INVERSE  "\x1B[7m"
#define ANSI_BLINK    "\x1B[5m"
#define ANSI_UNDERSCORE "\x1B[4m"

#define ANSI_INV_BLINK         "\x1B[7;5m"
#define ANSI_INV_HILITE        "\x1B[1;7m"
#define ANSI_BLINK_HILITE      "\x1B[1;5m"
#define ANSI_INV_BLINK_HILITE  "\x1B[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK	"\x1B[30m"
#define ANSI_RED	"\x1B[31m"
#define ANSI_GREEN	"\x1B[32m"
#define ANSI_YELLOW	"\x1B[33m"
#define ANSI_BLUE	"\x1B[34m"
#define ANSI_MAGENTA	"\x1B[35m"
#define ANSI_CYAN	"\x1B[36m"
#define ANSI_WHITE	"\x1B[37m"

/* Background colors */

#define ANSI_BBLACK	"\x1B[40m"
#define ANSI_BRED	"\x1B[41m"
#define ANSI_BGREEN	"\x1B[42m"
#define ANSI_BYELLOW	"\x1B[43m"
#define ANSI_BBLUE	"\x1B[44m"
#define ANSI_BMAGENTA	"\x1B[45m"
#define ANSI_BCYAN	"\x1B[46m"
#define ANSI_BWHITE	"\x1B[47m"

/* #ifdef TREKMUSH */
#define ANSI_LOLITE		"\x1B[2m"
#define ANSI_SCREEN_CLEAR	"\x1B[2J"
#define ANSI_ITALIC		"\x1B[3m"
#define ANSI_POS_1		"\x1B["
#define ANSI_POS_2		";"
#define ANSI_POS_3		"H"
/* #endif TREKMUSH */

#else

#define BEEP_CHAR     '\07'
#define ESC_CHAR      '\033'

#define ANSI_NORMAL   "\033[0m"

#define ANSI_HILITE   "\033[1m"
#define ANSI_INVERSE  "\033[7m"
#define ANSI_BLINK    "\033[5m"
#define ANSI_UNDERSCORE "\033[4m"

#define ANSI_INV_BLINK         "\033[7;5m"
#define ANSI_INV_HILITE        "\033[1;7m"
#define ANSI_BLINK_HILITE      "\033[1;5m"
#define ANSI_INV_BLINK_HILITE  "\033[1;5;7m"

/* Foreground colors */

#define ANSI_BLACK	"\033[30m"
#define ANSI_RED	"\033[31m"
#define ANSI_GREEN	"\033[32m"
#define ANSI_YELLOW	"\033[33m"
#define ANSI_BLUE	"\033[34m"
#define ANSI_MAGENTA	"\033[35m"
#define ANSI_CYAN	"\033[36m"
#define ANSI_WHITE	"\033[37m"

/* Background colors */

#define ANSI_BBLACK	"\033[40m"
#define ANSI_BRED	"\033[41m"
#define ANSI_BGREEN	"\033[42m"
#define ANSI_BYELLOW	"\033[43m"
#define ANSI_BBLUE	"\033[44m"
#define ANSI_BMAGENTA	"\033[45m"
#define ANSI_BCYAN	"\033[46m"
#define ANSI_BWHITE	"\033[47m"

/* #ifdef TREKMUSH */
#define ANSI_LOLITE		"\033[2m"
#define ANSI_SCREEN_CLEAR	"\033[2J"
#define ANSI_ITALIC		"\033[3m"
#define ANSI_POS_1		"\033["
#define ANSI_POS_2		";"
#define ANSI_POS_3		"H"
/* #endif TREKMUSH */

#endif

#endif				/* __ANSI_H */
