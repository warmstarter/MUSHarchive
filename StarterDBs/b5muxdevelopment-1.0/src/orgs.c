/*
 * orgs.c -- commands for giving help 
 */
/*
 * $Id: orgs.c,v 1.1 2001/01/15 03:14:10 wenk Exp $ 
 */

#include "copyright.h"
#include "autoconf.h"

#include <fcntl.h>

#include "mudconf.h"
#include "config.h"
#include "db.h"
#include "interface.h"
#include "externs.h"
#include "htab.h"
#include "alloc.h"
#include "flags.h"
#include "orglist.h"

t_OrgList *Orgs == NULL;


void OrgInit(void) {

  if (Orgs != NULL) { /* already init'd */
    return;
  }

}

