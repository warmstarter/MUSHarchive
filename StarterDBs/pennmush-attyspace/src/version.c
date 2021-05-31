/* This file defines the @version command. It's all by itself because
 * we want to rebuild this file at every compilation, so that the
 * BUILDDATE is correct
 */
#include "config.h"
#include "copyrite.h"

#ifdef I_STRING
#include <string.h>
#else
#include <strings.h>
#endif
#include <time.h>
#include "conf.h"
#include "intrface.h"
#include "externs.h"
#include "version.h"
#ifndef WIN32
#include "buildinf.h"
#endif
#include "confmagic.h"

extern time_t start_time;	/* from bsd.c */
void do_version _((dbref player));

void
do_version(player)
    dbref player;
{
  char buff[BUFFER_LEN];

  notify(player, tprintf("You are connected to %s", MUDNAME));

  strcpy(buff, ctime(&start_time));
  buff[strlen(buff) - 1] = '\0';	/* eat the newline */
  notify(player, tprintf("Last restarted: %s", buff));

  notify(player, tprintf("%s", VERSION));
#ifdef WIN32
  notify(player, tprintf("Build date: %s", __DATE__));
#else
  notify(player, tprintf("Build date: %s", BUILDDATE));
  notify(player, tprintf("Compiler: %s", COMPILER));
  notify(player, tprintf("Compilation flags: %s", CCFLAGS));
  notify(player, tprintf("Malloc package: %d", MALLOC_PACKAGE));
#endif
}
