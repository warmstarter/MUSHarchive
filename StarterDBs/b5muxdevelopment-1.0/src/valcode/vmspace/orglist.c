/* $Id: orglist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

static char *orglistcrcs="$Id: orglist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
#include "header.h"
#include "orglist.h"




t_OrgList *InitOrgList(void)
{

  t_OrgList *list;
  t_OrgEnt *tmp,*tmp2;
  list = (t_OrgList*) malloc(sizeof(t_OrgList)); 
  tmp = (t_OrgEnt *) malloc(sizeof(t_OrgEnt));
  tmp2 = (t_OrgEnt *) malloc(sizeof(t_OrgEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list in InitOrgList");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in InitOrgList");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2 in InitOrgList");
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}

void AddOrgEnt(list,id,master,dbrf)
     t_OrgList *list;
     int id;
     int master;
     int dbrf;
{
  t_OrgEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_OrgEnt *) malloc(sizeof(t_OrgEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp from AddOrgEnt");
    return;
  }
  tmp->id = id;
  tmp->master = master;
  tmp->dbrf = dbrf;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveOrgEnt(list)
     t_OrgList *list;
{
  t_OrgEnt *dummy;
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

void AdvanceOrgList(list)
     t_OrgList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetOrgList(list)
     t_OrgList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_OrgEnt *GetCurrOrgEnt(list)
     t_OrgList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next != list->end)
    return list->current->next;

  return(NULL);
}

int AtEndOrgList(list)
     t_OrgList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->current->next == list->end;
}

int GetOrgListSize(list)
     t_OrgList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

void DumpOrgList2Player(list,player)
     t_OrgList *list;
     int player;
{
  t_OrgEnt *tmp;
  ResetOrgList(list);
  while (!AtEndOrgList(list)) {
    tmp = GetCurrOrgEnt(list);
    VMnotify(player,"Org ID: %d Name: %s MasterOrgID: %d",tmp->id,Name(tmp->dbrf),tmp->master);
    AdvanceOrgList(list);
  }
}
	






