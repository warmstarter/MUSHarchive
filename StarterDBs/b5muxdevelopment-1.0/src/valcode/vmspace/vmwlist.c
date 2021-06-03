/* $Id: vmwlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
static char *vmwlistcrcs="$Id: vmwlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";

t_WList *InitWList(void)
{
  
  t_WList *list;
  t_WEnt *tmp,*tmp2;
  list = (t_WList *) malloc(sizeof(t_WList)); 
  tmp = (t_WEnt *) malloc(sizeof(t_WEnt));
  tmp2 = (t_WEnt *) malloc(sizeof(t_WEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list in InitWList");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in InitWList");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2 in InitWList");
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}

void KillWList(list)
     t_WList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetWList(list);
  while (!AtEndWList(list))
    {
      RemoveWEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddWEnt(list,time,type,ship,v1,v2,d1,d2)
     t_WList *list;
     int time,type,v1,v2,ship;
     double d1,d2;
{
  t_WEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  /*  VMnotify(35,"Adding W Ent: timer: %d event: %d ship: %d v1:%d v2:%d d1: %2.2f d2:%2.2f",time,type,ship,v1,v2,d1,d2);*/
  tmp = (t_WEnt *) malloc(sizeof(t_WEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in AddWEnt");
    return;
  } 
  tmp->timer = time;
  tmp->event = type;
  tmp->ship = ship;
  tmp->v1 = v1;
  tmp->v2 = v2;
  tmp->d1 = d1;
  tmp->d2 = d2;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveWEnt(list)
     t_WList *list;
{
  t_WEnt *dummy;
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

void AdvanceWList(list)
     t_WList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetWList(list)
     t_WList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_WEnt *GetCurrWEnt(list)
     t_WList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return NULL;
  }
  if (list->current->next != list->end)
    return list->current->next;
  return NULL;
}

int AtEndWList(list)
     t_WList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return 1;
  }
  return(list->current->next == list->end);
}

int GetWListSize(list)
     t_WList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  return list->size;
}














