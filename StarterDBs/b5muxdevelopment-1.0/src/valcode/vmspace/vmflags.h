/* $Id: vmflags.h,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

struct s_Flag
{
 char *name;
 int FlagNum; 
};

typedef struct s_Flag t_Flag;

extern t_Flag Flags[];

