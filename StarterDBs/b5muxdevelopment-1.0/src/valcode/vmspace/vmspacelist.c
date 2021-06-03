/* $Id: vmspacelist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */


#include "header.h"
#include "vmspacelist.h"

static char *rcsvmspacelistc="$Id: vmspacelist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";



t_SpaceList *InitSpaceList(void)
{

  t_SpaceList *list;
  t_SpaceEnt *tmp,*tmp2;
  list = (t_SpaceList*) malloc(sizeof(t_SpaceList)); 
  tmp = (t_SpaceEnt *) malloc(sizeof(t_SpaceEnt));
  tmp2 = (t_SpaceEnt *) malloc(sizeof(t_SpaceEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list in InitSpaceList");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in InitSpaceList");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2 in InitSpaceList");
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}
void SemiKillSpaceList(list,ShipNum)
     t_SpaceList *list;
     int ShipNum;
{

  t_SpaceEnt *tmp;
  int i;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetSpaceList(list);
  while (!AtEndSpaceList(list))
    {
      tmp=GetCurrSpaceEnt(list);
	if(tmp==NULL) break;

      ContactMatrix[ShipNum][tmp->VMShip]=0;


      RemoveSpaceEnt(list);

    }
}

void SortaKillSpaceList(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetSpaceList(list);
  while (!AtEndSpaceList(list))
    {
      RemoveSpaceEnt(list);
    }
}

void KillSpaceList(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetSpaceList(list);
  while (!AtEndSpaceList(list))
    {
      RemoveSpaceEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddSpaceEnt(list,VMship)
     t_SpaceList *list;
     int VMship;
{
  t_SpaceEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_SpaceEnt *) malloc(sizeof(t_SpaceEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp from AddSpaceEnt");
    return;
  }
  tmp->VMShip= VMship;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveSpaceEnt(list)
     t_SpaceList *list;
{
  t_SpaceEnt *dummy;
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

void AdvanceSpaceList(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetSpaceList(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_SpaceEnt *GetCurrSpaceEnt(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next != list->end)
    return list->current->next;

  return(NULL);
}

int AtEndSpaceList(list)
     t_SpaceList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->current->next == list->end;
}

int GetSpaceListSize(list)
     t_SpaceList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

int IsShipInSpaceList(list,VMship)
	t_SpaceList *list;
	int VMship;
{
  t_SpaceEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetSpaceList(list);
  while(!AtEndSpaceList(list)) {
    tmp=GetCurrSpaceEnt(list);
    if(tmp->VMShip==VMship) {
      return 1;
    }
    AdvanceSpaceList(list);
 }
  return 0;
}


	






