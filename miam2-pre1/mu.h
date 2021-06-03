/* mu.h
   helpers for tools for MU replication
   09/12/2001 Azundris
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifndef FALSE
#  define FALSE (0)
#  ifndef TRUE
#    define TRUE (!(FALSE))
#  endif
#endif


#define MAGIC "-- AZUNDRIS --"

#define STR_DIG    "@dig/teleport "
#define STR_CREATE "@create "
#define STR_PLAYER "@pcreate "
#define STR_OPEN   "@open "
#define STR_NEXT   "---"



#define TYPE_NONE   0
#define TYPE_EXIT   1
#define TYPE_OBJECT 2
#define TYPE_ROOM   3
#define TYPE_PLAYER 4
#define TYPE_EXIT_F 5



#define STATE_FIRST 1
#define STATE_DBREF 1
#define STATE_HOME  2
#define STATE_LOC   3
#define STATE_OWNER 4
#define STATE_OBJ   5



/* read a line from a file, a return a pointer to it.
   length does not matter as long as there is enough RAM.
   -> fh   file handle
   <- ret  char *line or NULL for no more readable entries (due to EOF or OOM)
 */
#define MAX_GL (64*1024L)

char *getline(FILE *fh) {
  char   *s,*p;
  size_t  l;

  if(s=malloc(MAX_GL)) {
    p=s;
    s[0]='\0';

    while(TRUE) {
      if(!fgets(p,MAX_GL,fh)) {   /* EOF? */
        if(p==s) {                /* ...yes, if none at all, return NULL */
          free(s);
          return NULL; }          /* ...else return what we got so far */
        else
          return s; }
      else {                      /* got something... */
        char *p2;
        l=strlen(p);
        p+=l;
        p2=p;
        while((*(--p)=='\n')||(*p=='\r')) {
          *p='\0'; }
        if(p<p2)                  /* ...got EOL, done */
          return s;
        else {
          if(!(s=realloc(s,l+MAX_GL))) { /* ...get more mem */
            free(s);
            return NULL; }}}}}    /* ...and bail out if we can't */

  return NULL; }



#define streq(a,b)   (!strncmp(a,b,strlen(b)))



char *next_word(char *l) {
  char *p;
  if((p=l)&&(p=strchr(p,' ')))
    p++;
  return p; }



char *find_dbref(char *l) {
  char *p=l;

  if(p) {
    while(p=strchr(p,'#')) {
      if(((p==l)||((p[-1]!='#')&&(p[-1]!='%')))&&isdigit((int)p[1]))
        return p;
      else
        p++; }}
  return NULL; }

/* ends */
