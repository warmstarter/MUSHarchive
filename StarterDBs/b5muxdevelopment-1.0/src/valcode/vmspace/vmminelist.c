#include "header.h"
static char *rcsminelistc="$Id: vmminelist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $"; 
t_MineList *InitMineList(void)
{
  t_MineList *list;
  t_MineEnt *tmp,*tmp2;
  list = (struct s_minelist *) malloc(sizeof(t_MineList)); 
  tmp = (struct s_Mineent *) malloc(sizeof(t_MineEnt));
  tmp2 = (struct s_Mineent *) malloc(sizeof(t_MineEnt));
  if (list == NULL) {
    /* list is not valid */
    return NULL;
  }
  if (tmp == NULL) {
    /* list is not valid */
    return;
  }
  if (tmp2 == NULL) {
    /* list is not valid */
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}


void KillMineList(list)
     t_MineList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetMList(list);
  while (!AtEndMList(list))
    {
      RemoveMEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddMineEnt(t_MineList *list,
	     int time,
	     int launcher,
	     int gunner,
	     t_xyz loc,
	     int missle,
	     int crng,
	     int mrng,
	     int flags
	     ) {
  t_MineEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_MineEnt *) malloc(sizeof(t_MineEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in AddMEnt");
    return;
  }
  tmp->timer = time;
  tmp->launcher =launcher;
  tmp->gunner = gunner;
  tmp->loc = loc;
  tmp->missle = missle;
  tmp->crng = crng;
  tmp->flags = flags;
  tmp->mrng = mrng;

  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveMineEnt(list)
     t_MineList *list;
{
  t_MineEnt *dummy;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end) 
    return;
  dummy = list->current->next;
  list->current->next = dummy->next;
  free(dummy);
  list->size--;
}

void AdvanceMineList(list)
     t_MineList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetMineList(list)
     t_MineList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_MineEnt *GetCurrMineEnt(list)
     t_MineList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return NULL;
  }
  if (list->current->next != list->end)
    return list->current->next;
  return NULL;
}

int AtEndMineList(list)
     t_MineList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return 1;
  }
  return list->current->next == list->end;
}

int GetMineListSize(list)
     t_MineList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  return list->size;
}














