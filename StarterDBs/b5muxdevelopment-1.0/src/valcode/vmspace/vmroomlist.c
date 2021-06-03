/* $Id: vmroomlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */


#include "header.h"
#include "vmroomlist.h"
static char *rcsvmroomlistc="$Id: vmroomlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";



t_RoomList *InitRoomList(void)
{
  t_RoomList *list;
  t_RoomEnt *tmp,*tmp2;
  list = (t_RoomList*) malloc(sizeof(t_RoomList)); 
  tmp = (t_RoomEnt *) malloc(sizeof(t_RoomEnt));
  tmp2 = (t_RoomEnt *) malloc(sizeof(t_RoomEnt));
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
 if (tmp == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
 if (tmp2 == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}


void KillRoomList(list)
     t_RoomList *list;
{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  ResetRoomList(list);
  while (!AtEndRoomList(list))
    {
      RemoveRoomEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddRoomEnt(t_RoomList *list,
		int room,
		int damage
		) 

{

  t_RoomEnt *tmp;
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  tmp = (t_RoomEnt *) malloc(sizeof(t_RoomEnt));
  if (tmp == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  tmp->room = room;
  tmp->damage = damage;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveRoomEnt(list)
     t_RoomList *list;
{
  t_RoomEnt *dummy;
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  if (list->current->next == list->end) 
    return;
  dummy = list->current->next;
  list->current->next = dummy->next;
  free(dummy);
  list->size--;
}

void AdvanceRoomList(list)
     t_RoomList *list;
{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetRoomList(list)
     t_RoomList *list;
{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return;
  }
  list->current = list->start;
}

t_RoomEnt *GetCurrRoomEnt(list)
     t_RoomList *list;
{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return NULL;
  }
  if (list->current->next != list->end)
    return list->current->next;

  return(NULL);
}

int AtEndRoomList(list)
     t_RoomList *list;

{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return 1;
  }
  return list->current->next == list->end;
}

int GetRoomListSize(list)
     t_RoomList *list;
{
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return -1;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

int IsRoomInRoomList(list,VMship)
	t_RoomList *list;
	int VMship;
{
  t_RoomEnt *tmp;
  if (list == NULL) {
    /* somehow we didn't get the memory we wanted. */
    return 0;
  }
  ResetRoomList(list);
  while(!AtEndRoomList(list)) {
    tmp=GetCurrRoomEnt(list);
    if(tmp->room ==VMship) {
      return 1;
    }
    AdvanceRoomList(list);
 }
  return 0;
}


	






