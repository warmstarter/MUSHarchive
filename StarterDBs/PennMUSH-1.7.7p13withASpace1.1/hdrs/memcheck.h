#ifndef __MEMCHECK_H
#define __MEMCHECK_H

extern void add_check(const char *ref);
extern void del_check(const char *ref);
extern void list_mem_check(dbref player);
extern void log_mem_check(void);

#endif				/* __MEMCHECK_H */
