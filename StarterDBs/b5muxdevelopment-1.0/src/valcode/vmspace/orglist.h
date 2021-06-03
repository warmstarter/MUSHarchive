/* $Id: orglist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* orglist header file
 * Original code by ???
 * Change to org by Mike Wenk
 */
#ifndef _ORGLISTH_
#define _ORGLISTH_
typedef struct s_OrgList t_OrgList;
typedef struct s_OrgEnt t_OrgEnt;




struct s_OrgList {
  int size;
  t_OrgEnt *start;
  t_OrgEnt *end;
  t_OrgEnt *current;
};

struct s_OrgEnt {
  int id;
  int master;
  int dbrf;
  t_OrgEnt *next;
};


#endif



