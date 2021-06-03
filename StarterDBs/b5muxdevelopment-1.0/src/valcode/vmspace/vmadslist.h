#ifndef _VMADSLISTH_
#define _VMADSLISTH_
typedef struct s_ADSList t_ADSList;
typedef struct s_ADSEnt t_ADSEnt;

struct s_ADSList {
  int size;
  t_ADSEnt *start;
  t_ADSEnt *end;
  t_ADSEnt *current;
};

struct s_ADSEnt {
  int ship;
  int flag;
  int owner;
  t_ADSEnt *next;
};
#define BIGERR -1
#define NOLIST -2
#define EMPLIST -3


#endif
