#ifndef __MEMCHECK_H
#define __MEMCHECK_H

typedef struct mem_check MEM;

struct mem_check {
  int ref_count;
  char *ref_name;
  MEM *next;
};

extern void add_check _((const char *ref));
extern void del_check _((const char *ref));
extern void rebuild_check _((void));
extern void list_mem_check _((dbref player));
extern void log_mem_check _((void));

#endif				/* __MEMCHECK_H */
