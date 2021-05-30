# include <stdio.h>
# include <sys/types.h>
# include <sys/time.h>
# include <sys/stat.h>

# define WFILE "/usr/dict/words"

# ifdef TESTING
main ()
{ char *codeword ();
  
  srand (time (0) + getpid () + getuid ());
  printf ("%s\n", codeword ());
}
# endif

char *codeword ()
{ static char word[64];
  struct stat sbuf;
  long length;
  FILE *wfile = NULL;
  char *s = word;
  int ch, i;

  /* Pick random line from file */
  if (stat (WFILE, &sbuf) == 0 &&
      (length = sbuf.st_size) > 0 &&
      (wfile = fopen (WFILE, "r")))
  { fseek (wfile, randint (length), 0);
    while ((ch = getc (wfile)) != EOF && ch != '\n') ;
    if (ch == EOF) rewind (wfile);

    if (fgets (word, 64, wfile))
    { fclose (wfile);
      word[strlen (word) - 1] = '\0';
    }
    
    return (s);
  }

  /* Return totally random 8 chars */
  for (i=0; i<8; i++)
  { word[i] = randint (26) + 'a'; }
  word[i] = '\0';
  return (s);
}
