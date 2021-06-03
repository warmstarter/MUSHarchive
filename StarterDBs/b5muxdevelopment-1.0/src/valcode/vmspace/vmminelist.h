/* $Id: vmminelist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */

#ifndef _VMMineLISTH_
#define _VMMineLISTH_
typedef struct s_minelist t_MineList;
typedef struct s_Mineent t_MineEnt;




struct s_minelist {
  int size;
  t_MineEnt *start;
  t_MineEnt *end;
  t_MineEnt *current;
};

struct s_Mineent {
  int timer;
  int launcher;
  int gunner;
  int missle;
  int crng;
  int mrng;
  int flags;
  t_xyz loc;
  t_MineEnt *next;
};

#endif





