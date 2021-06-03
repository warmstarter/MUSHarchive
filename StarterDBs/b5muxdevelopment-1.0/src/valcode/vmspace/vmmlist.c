/* $Id: vmmlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"
#include "vmmlist.h"
static char *rcsvmlistc="$Id: vmmlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
t_MList *InitMList(void)
{
  t_MList *list;
  t_MEnt *tmp,*tmp2;
  /*list = (struct s_mlist *) malloc(sizeof(t_MList)); 
  tmp = (struct s_ment *) malloc(sizeof(t_MEnt));
  tmp2 = (struct s_ment *) malloc(sizeof(t_MEnt));
*/
  list = (t_MList *) malloc(sizeof(t_MList)); 
  tmp = (t_MEnt *) malloc(sizeof(t_MEnt));
  tmp2 = (t_MEnt *) malloc(sizeof(t_MEnt));
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


void KillMList(list)
     t_MList *list;
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


void AddMEnt(t_MList *list,
	     int time,
	     int launcher,
	     int gunner,
	     int target, 
	     t_xyz orgin,
	     t_xyz destination,
	     int missle,
	     int acrrng,
	     int intel,
	     int ecm,
	     int eccm
	     ) {
  t_MEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_MEnt *) malloc(sizeof(t_MEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in AddMEnt");
    return;
  }
  tmp->timer = time;
  tmp->launcher =launcher;
  tmp->target = target;
  tmp->gunner = gunner;
  tmp->orgin = orgin;
  tmp->destination = destination;
  tmp->missle = missle;
  tmp->acrrng = acrrng;
  tmp->intel = intel;
  tmp->ecm = ecm;
  tmp->eccm = eccm;

  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveMEnt(list)
     t_MList *list;
{
  t_MEnt *dummy;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)  {
    return;
}
  dummy = list->current->next;
  list->current->next = dummy->next;
  free(dummy);
  list->size--;
}

void AdvanceMList(list)
     t_MList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetMList(list)
     t_MList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_MEnt *GetCurrMEnt(list)
     t_MList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return NULL;
  }
  if (list->current->next != list->end)
    return list->current->next;
  return NULL;
}

int AtEndMList(list)
     t_MList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return 1;
  }
  return(list->current->next == list->end);
}

int GetMListSize(list)
     t_MList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  return list->size;
}














