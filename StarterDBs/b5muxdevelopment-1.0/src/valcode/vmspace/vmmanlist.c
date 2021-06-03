/* $Id: vmmanlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */
#include "header.h"


#include "vmmanlist.h"
static char *rcsvmmanlistc="$Id: vmmanlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";


t_ManList *InitManList(void)
{
  t_ManList *list;
  t_ManEnt *tmp,*tmp2;
  list = (struct s_manlist *) malloc(sizeof(t_ManList)); 
  tmp = (struct s_manent *) malloc(sizeof(t_ManEnt));
  tmp2 = (struct s_manent *) malloc(sizeof(t_ManEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2");
    return;
  }

  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}

void KillManList(list)
     t_ManList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetManList(list);
  while (!AtEndManList(list))
    {
      RemoveManEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddManEnt(list,VMplayer,VMroom)
     t_ManList *list;
     int VMplayer,VMroom;
{
  t_ManEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_ManEnt *) malloc(sizeof(t_ManEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in AddManEnt");
    return;
  }
  tmp->VMRoom= VMroom;
  tmp->VMPlayer= VMplayer;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveManEnt(list)
     t_ManList *list;
{
  t_ManEnt *dummy;
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

void AdvanceManList(list)
     t_ManList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetManList(list)
     t_ManList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_ManEnt *GetCurrManEnt(list)
     t_ManList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return NULL;
  }
  if (list->current->next != list->end)
    return list->current->next;
  return NULL;
}

int AtEndManList(list)
     t_ManList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return 1;
  }
  return list->current->next == list->end;
}

int GetManListSize(list)
     t_ManList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

int IsRoomInManList(list,VMroom)
	t_ManList *list;
	int VMroom;
{
  t_ManEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  ResetManList(list);
  while(!AtEndManList(list)) {
    tmp=GetCurrManEnt(list);
    if(tmp->VMRoom==VMroom) {
      return 1;
    }
    AdvanceManList(list);
  }
  return 0;
}


int IsPlayerInManList(list,VMplayer)
	t_ManList *list;
	int VMplayer;
{
  t_ManEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  ResetManList(list);
  while(!AtEndManList(list)) {
    tmp=GetCurrManEnt(list);
    if(tmp->VMPlayer==VMplayer) {
      return 1;
    }
    AdvanceManList(list);
  }
  return 0;
}



	








