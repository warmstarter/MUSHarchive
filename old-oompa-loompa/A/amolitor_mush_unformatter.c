
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/*
#define index(a,b) strchr((a),(b))
*/

#define MAXLINE 4000

main(ac,av)
int ac;
char *av[];
{
	int	mudfd;
	FILE	*infile;
	char	linehead[32];
	char	line[512];
	char	*p,*q;
	int	count;
	int	notified;
	int	line_no;
	int	line_head_no;

	if(ac > 2){
		exit(usage(av[0]));
	}
	if(ac == 2){
		infile = fopen(av[ac - 1],"r");
		if(infile == (FILE *)0){
			fprintf(stderr,"Could not open %s.\n",av[3]);
			exit(usage(av[0]));
		}
	} else {
		infile = stdin;
	}

	mudfd = 1; /* stdout */

	/* Rock and roll */

	count = 0;
	notified = 0;
	line_no = 0;
	while(fgets(line,512,infile)){

		line_no++;

		/* Is this the end of a record? */

		if(line[0] == '-'){
			if(notified) { /* calculate overflow. */
				fprintf(stderr,"Overflowed by %d bytes.",
					count - MAXLINE);
			}
			count = 0;
			notified = 0;
			write(mudfd,"\n",1);
			fputc('.',stderr);
			continue;
		}
		/* Toss comments */

		if(line[0] == '#')
			continue;

		/* Strip leading whitespace */

		for(p = line;isspace(*p) && *p; p++)
			;

		/* Stomp the trailing newline */

		q = (char *)index(p,'\n');
		if(q)
			*q = '\0';

		/* Scribble it at the mud */

		if(count == 0){
			strncpy(linehead,p,32);
			linehead[31] = '\0';
			line_head_no = line_no;
		}
		write(mudfd,p,strlen(p));
		count += strlen(p);

		if(count > MAXLINE && !notified){
			notified = 1;
			fprintf(stderr,
				"Line exceeded %d at line number %d:%s\n",
				MAXLINE,line_head_no,linehead);
		}
	}

	/* One more for luck. */

	write(mudfd,"\n",1);
	fprintf(stderr,"done.\n");
	exit(0);
}

usage(p)
char *p;
{
	fprintf(stderr,"usage: %s [file]\n",p);
	return(1);
}

