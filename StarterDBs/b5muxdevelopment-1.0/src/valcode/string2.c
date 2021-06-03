
#include "header.h"


 #define DOWNCASE(x) to_lower(x)
 
 
 char to_lower(x)
      int x;
 {
   if(x<'A' || x>'Z') return x;
   return 'a'+(x-'A');
 }
 
 
 
 int string_compare2(s1,s2)
  char *s1;
  char *s2;
 {
   if ((s1 == NULL) || (s2 == NULL))
     {
       return(!(s1 == s2));
     }
 
   while(*s1 && *s2 && DOWNCASE(*s1) == DOWNCASE(*s2)) s1++, s2++;
   
   return(DOWNCASE(*s1) - DOWNCASE(*s2));
 }
 
 
 
 
static char *rcsstring2c="$Id: string2.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
