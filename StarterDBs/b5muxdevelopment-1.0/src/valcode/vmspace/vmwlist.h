/* $Id: vmwlist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */


#ifndef _VMWLISTH_
#define _VMWMANLISTH_
typedef struct s_wlist t_WList;
typedef struct s_went t_WEnt;




struct s_wlist {
  int size;
  t_WEnt *start;
  t_WEnt *end;
  t_WEnt *current;
};

struct s_went {
  int timer; /* when this event goes (decremented every tick) */ 
  int event; /* what sort of event i am */
  int v1; /* int variable depending on event */
  int v2; /* int variable depending on event */
  double d1; /* double variable depending on event */
  double d2; /* double variable depending on event */
  int ship;
  t_WEnt *next;
};

#endif





