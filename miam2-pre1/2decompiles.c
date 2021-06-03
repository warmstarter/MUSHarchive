/* decompile.c
   a tool for MU replication
   09/12/2001 Azundris
 */

#include <stdio.h>
#include <string.h>
#include "mu.h"


int main(int argc,char **argv,char **env) {
  FILE *fh=stdin;
  char *l;
  int   ret=-1;

  /* quasi-positional parameters. bad, bad. */
  if(argc>1) {
    if(!strcmp(argv[1],"--help")||!strcmp(argv[1],"-h")) {
      printf("%s [<file>] -- converts MU* object lists to @decompile lists\n\t--Azundris 2001/09/13\n",
             argv[0]);
      return 0; }

    fh=fopen(argv[1],"r"); }

  if(fh) {
    int   dbref;
    while(l=getline(fh)) {
      char *p=l-1L;
      do {
        p=find_dbref(++p);
      } while(p&&((p>&l[1])&&(p[-1]!='(')));

      if(p) {
     /* printf("line: %s\n",p); */
        dbref=atoi(++p);
        printf("@pemit me=#%d\n"
               "@pemit me=home(#%d)\n"
               "@pemit me=loc(#%d)\n"
               "@pemit me=owner(#%d)\n"
               "@decompile #%d\n"
               "@pemit me=@alias #%d=[get(#%d/ALIAS)]\n"
               "@pemit me=" STR_NEXT "\n\n",dbref, dbref,dbref,dbref, dbref,
                                            dbref,dbref); }
      free(l); }
    ret=0;

    if(fh!=stdin)                       /* defensive */
      fclose(fh); }

  return ret; }

/* ends */
