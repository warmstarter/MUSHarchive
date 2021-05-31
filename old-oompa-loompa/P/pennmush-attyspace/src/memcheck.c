#include "config.h"
#include "conf.h"
#include "copyrite.h"

#ifdef MEM_CHECK

#include "externs.h"
#include "dbdefs.h"
#include "memcheck.h"
#include "mymalloc.h"
#include "confmagic.h"

void add_check _((const char *ref));
void del_check _((const char *ref));
void rebuild_check _((void));
void list_mem_check _((dbref player));
void log_mem_check _((void));

MEM *my_check = NULL;

void
add_check(ref)
    const char *ref;
{
  MEM *loop;

  for (loop = my_check; loop; loop = loop->next) {
    if (!strcasecmp(ref, loop->ref_name)) {
      loop->ref_count++;
      return;
    }
  }
  loop = (MEM *) malloc(sizeof(MEM));
  loop->ref_name = (char *) malloc(strlen(ref) + 1);
  strcpy(loop->ref_name, ref);
  loop->ref_count = 1;
  loop->next = my_check;
  my_check = loop;
  add_check("mem_check");
  return;
}

void
del_check(ref)
    const char *ref;
{
  MEM *loop;

  for (loop = my_check; loop; loop = loop->next) {
    if (!strcasecmp(loop->ref_name, ref)) {
      loop->ref_count--;
      if (!loop->ref_count) {
	rebuild_check();
	del_check("mem_check");
      }
      return;
    }
  }
  fprintf(stderr,
      "ERROR: Tried deleteing a check that was never added! :%s\n", ref);
}

void
rebuild_check()
{
  MEM *point, *next, *newone = NULL;

  point = my_check;
  while (point) {
    next = point->next;
    if (point->ref_count) {
      point->next = newone;
      newone = point;
    } else {
      free((Malloc_t) point->ref_name);
      free((Malloc_t) point);
    }
    point = next;
  }
  my_check = newone;
}

void
list_mem_check(player)
    dbref player;
{

  MEM *loop;

  for (loop = my_check; loop; loop = loop->next) {
    notify(player, tprintf("%s : %d", loop->ref_name, loop->ref_count));
  }
}

void
log_mem_check()
{

  MEM *loop;

  for (loop = my_check; loop; loop = loop->next) {
    do_log(LT_CHECK, 0, 0, "%s : %d", loop->ref_name, loop->ref_count);
  }
}

#endif				/* MEM_CHECK */
