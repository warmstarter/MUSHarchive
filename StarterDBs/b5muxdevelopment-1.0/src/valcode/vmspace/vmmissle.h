#ifndef _VMMISSLE_H_
#define _VMMISSLE_H_
struct s_missle {
  char  name[50];
  int index;
  int plan;
  int size;
  int maxrange;
  int speed;
  int amod;
  int intel;
  int baseecm;
  int baseeccm;
  int loadtime;
  int damage;
  int flags;
/* new adds by valdar  Are of effect adds*/
  int slope;
  int basedmg;
}; 
#define NUMMISS 20
typedef struct s_missle t_missle;
#endif

