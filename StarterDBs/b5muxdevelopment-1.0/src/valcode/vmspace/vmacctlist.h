/* $Id: vmacctlist.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

/* acctlist header file
 * Original code by ???
 * Change to acct by Mike Wenk
 */
#ifndef _VMACCTLISTH_
#define _VMACCTLISTH_
typedef struct s_AcctList t_AcctList;
typedef struct s_AcctEnt t_AcctEnt;




struct s_AcctList {
  int size;
  t_AcctEnt *start;
  t_AcctEnt *end;
  t_AcctEnt *current;
};

struct s_AcctEnt {
  t_Acct acct;
  t_AcctEnt *next;
};


#endif
