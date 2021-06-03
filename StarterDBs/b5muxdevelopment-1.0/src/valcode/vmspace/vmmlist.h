/* $Id: vmmlist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */


#ifndef _VMMLISTH_
#define _VMMLISTH_
typedef struct s_mlist t_MList;
typedef struct s_ment t_MEnt;




struct s_mlist {
  int size;
  t_MEnt *start;
  t_MEnt *end;
  t_MEnt *current;
};

struct s_ment {
  int timer; /* when this event goes (decremented every tick) */ 
  int launcher; /* ship that launched */
  int gunner; /* dbref of player who fired, needed for skill check */
  int target; /* target ship */
  t_xyz orgin;
  t_xyz destination;
  int missle; /* where in the missle array that this guy is */
  int acrrng;
  int intel;
  int ecm;
  int eccm;
  t_MEnt *next;
};

#endif





