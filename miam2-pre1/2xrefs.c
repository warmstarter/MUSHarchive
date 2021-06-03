/* 2xrefs.c
   a tool for MU replication
   09/12/2001 Azundris
 */

#include <stdio.h>
#include <string.h>
#include "mu.h"


int main(int argc,char **argv,char **env) {
  FILE *fh=stdin;
  char *l,*l2=NULL;
  int   ret=-1;

  /* quasi-positional parameters. bad, bad. */
  if(argc>1) {
    if(!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h")) {
      printf("%s [<file>] -- converts MU* object lists to xref lists\n\t--Azundris 2001/09/13\n",
             argv[0]);
      return 0; }

    fh=fopen(argv[1],"r"); }

  if(fh) {
    int   from,to;
    char *p;
    while(l=getline(fh)) {
      if((p=strstr(l,MAGIC))&&strlen(l2)) {
        from=atoi(&l[1]);
        p=&l2[strlen(l2)-1];
        while(p>=l2&&!isdigit(*p))
          *(p--)='\0';
        while(p>l2&&((*p=='-')||isdigit(*p)))
          p--;
        to=atoi(&p[1]);
        if(to<1)
          fprintf(stderr,"1st: %s\n2nd: %s\n\n",l2,l);
        else
          printf("%d->%d\n",from,to); }

      if(l2)
        free(l2);
      l2=l; }
    ret=0;

    if(l)
      free(l);

    if(fh!=stdin)                       /* defensive */
      fclose(fh); }

  return ret; }

/* ends */
