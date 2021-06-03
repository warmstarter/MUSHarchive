/* $Id: vmroomlist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* spacelist header file
 * Original code by Mike Wenk in use for 
 * Change to space by Mike Wenk
 */
#ifndef _VMROOMLISTH_
#define _VMROOMLISTH_
typedef struct s_RoomList t_RoomList;
typedef struct s_RoomEnt t_RoomEnt;




struct s_RoomList {
  int size;
  t_RoomEnt *start;
  t_RoomEnt *end;
  t_RoomEnt *current;
};

struct s_RoomEnt {
  int room;
  int damage;
  t_RoomEnt *next;
};


#endif
