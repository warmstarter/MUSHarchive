/* $Id: vmrelaylist.c,v 1.1 2001/01/15 03:23:16 wenk Exp $ */


#include "header.h"


#include "./vmspace/vmspace.h"
#include "./vmspace/vmspacelist.h"
#include "vmrelaylist.h"



t_RelayList *InitRelayList(void)
{
  t_RelayList *list;
  t_RelayEnt *tmp,*tmp2;
  list = (t_RelayList*) malloc(sizeof(t_RelayList)); 
  tmp = (t_RelayEnt *) malloc(sizeof(t_RelayEnt));
  tmp2 = (t_RelayEnt *) malloc(sizeof(t_RelayEnt));
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;

  return list;
}
void SemiKillRelayList(list,ShipNum)
     t_RelayList *list;
     int ShipNum;
{
t_RelayEnt *tmp;
int i;
  ResetRelayList(list);
  while (!AtEndRelayList(list))
    {
      tmp=GetCurrRelayEnt(list);
	if(tmp==NULL) break;

      ContactMatrix[ShipNum][tmp->VMShip]=0;


      RemoveRelayEnt(list);

    }
}


void KillRelayList(list)
     t_RelayList *list;
{
int i;
t_RelayEnt *item;
  ResetRelayList(list);
  while (!AtEndRelayList(list))
    {
	item=GetCurrRelayEnt(list);
      RemoveRelayEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddRelayEnt(list,VMship,VMRoom,VMID)
     t_RelayList *list;
     int VMship;
     int VMRoom;
     int VMID;
{
int i;
  t_RelayEnt *tmp;
  tmp = (t_RelayEnt *) malloc(sizeof(t_RelayEnt));
  tmp->VMShip= VMship;
  tmp->VMRoom= VMRoom;
  tmp->VMID= VMID;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;


}

void RemoveRelayEnt(list)
     t_RelayList *list;
{
  int i;
  t_RelayEnt *dummy;
  if (list->current->next == list->end) 
    return;
  dummy = list->current->next;
  list->current->next = dummy->next;

  free(dummy);
  list->size--;
}

t_RelayEnt *SaveRelay(list)
     t_RelayList *list;
{
	return(list->current);
}

void RestoreRelay(list,item)
     t_RelayList *list;
     t_RelayEnt *item;
{
	list->current=item;
}
void AdvanceRelayList(list)
     t_RelayList *list;
{
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetRelayList(list)
     t_RelayList *list;
{
  list->current = list->start;
}

t_RelayEnt *GetCurrRelayEnt(list)
     t_RelayList *list;
{
  if (list->current->next != list->end)
    return list->current->next;
  return NULL;
}

int AtEndRelayList(list)
     t_RelayList *list;

{
  return list->current->next == list->end;
}

int GetRelayListSize(list)
     t_RelayList *list;
{
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

int IsShipInRelayList(list,VMship)
	t_RelayList *list;
	int VMship;
{
  t_RelayEnt *tmp;
  ResetRelayList(list);
  while(!AtEndRelayList(list)) {
    tmp=GetCurrRelayEnt(list);
    if(tmp->VMShip==VMship) {
      return 1;
    }
    AdvanceRelayList(list);
 }
  return 0;
}


	






static char *rcsvmrelaylistc="$Id: vmrelaylist.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
