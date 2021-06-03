/* $Id: vmspacelist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */
#ifndef _VMSPACELISTH_
#define _VMSPACELISTH_
typedef struct s_SpaceList t_SpaceList;
typedef struct s_SpaceEnt t_SpaceEnt;




struct s_SpaceList {
  int size;
  t_SpaceEnt *start;
  t_SpaceEnt *end;
  t_SpaceEnt *current;
};

struct s_SpaceEnt {
  int VMShip;
  t_SpaceEnt *next;
};


#endif
