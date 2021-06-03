/* $Id: vmspaceflags.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
#include "vmspaceflags.h"

static char *vmspaceflagsid="$Id: vmspaceflags.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int setflag(spacevar,flag)
     int spacevar;
     int flag;
     
{
  if (! (spacevar & flag) )
    spacevar += flag;
  return spacevar;
}

#define  checkflag(spacevar,flag)  (spacevar & flag)

int clearflag(spacevar,flag)
     int spacevar;
     int flag;
     
{
  if (spacevar & flag )
     spacevar -= flag;
  return spacevar;
}





