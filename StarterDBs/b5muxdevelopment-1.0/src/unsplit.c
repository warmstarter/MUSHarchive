/*
 * unsplit.c -- filter for re-combining continuation lines 
 */
/*
 * $Id: unsplit.c,v 1.1 2001/01/15 03:14:11 wenk Exp $ 
 */

#include "copyright.h"

#include <stdio.h>
#include <ctype.h>

void main(argc, argv)
int argc;
char **argv;
{
	int c, numcr;

	while ((c = getchar()) != EOF) {
		if (c == '\\') {
			numcr = 0;
			do {
				c = getchar();
				if (c == '\n')
					numcr++;
			} while ((c != EOF) && isspace(c));
			if (numcr > 1)
				putchar('\n');
			ungetc(c, stdin);
		} else {
			putchar(c);
		}
	}
	fflush(stdout);
	exit(0);
}
