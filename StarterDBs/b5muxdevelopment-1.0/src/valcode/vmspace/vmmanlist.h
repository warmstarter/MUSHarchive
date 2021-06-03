/* $Id: vmmanlist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */


#ifndef _VMMANLISTH_
#define _VMMANLISTH_
typedef struct s_manlist t_ManList;
typedef struct s_manent t_ManEnt;




struct s_manlist {
  int size;
  t_ManEnt *start;
  t_ManEnt *end;
  t_ManEnt *current;
};

struct s_manent {
  int VMPlayer;
  int VMRoom;
  t_ManEnt *next;
};

#endif





