/* Auto Defense/Docking System List set */ 


#include "header.h"
#include "vmadslist.h"
static char *rcsadslistc="$Id: vmadslist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";




t_ADSList *InitADSList(void)
{

  t_ADSList *list;
  t_ADSEnt *tmp,*tmp2;
  list = (t_ADSList*) malloc(sizeof(t_ADSList)); 
  tmp = (t_ADSEnt *) malloc(sizeof(t_ADSEnt));
  tmp2 = (t_ADSEnt *) malloc(sizeof(t_ADSEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list in InitADSList");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in InitADSList");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2 in InitADSList");
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}

void KillADSList(
		 t_ADSList *list
		 ) {
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetADSList(list);
  while (!AtEndADSList(list))
    {
      RemoveADSEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddADSEnt(
	       t_ADSList *list,
	       int ship,
	       int flag,
	       int owner
	       ) {
  t_ADSEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_ADSEnt *) malloc(sizeof(t_ADSEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp from AddADSEnt");
    return;
  }
  tmp->ship = ship;
  tmp->flag = flag;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveADSEnt(
		  t_ADSList *list
		  ) {
  t_ADSEnt *dummy;
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

void AdvanceADSList(t_ADSList *list
		    ) {
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetADSList(t_ADSList *list
		  ){
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_ADSEnt *GetCurrADSEnt(t_ADSList *list
			){
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next != list->end)
    return list->current->next;

  return(NULL);
}

int AtEndADSList(t_ADSList *list
		 ){
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->current->next == list->end;
}

int GetADSListSize(t_ADSList *list
		   ){
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/
void DumpADSList(t_ADSList *list,
		int player
		) {
  t_ADSEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetADSList(list);
  while(!AtEndADSList(list)) {
    tmp = GetCurrADSEnt(list);
    if (tmp == NULL) {
      VMnotify(player, "BAD LIST!!!!!!");
      return;
    }
    VMnotify(player,"--------------");
    VMnotify(player,"ship: %d",tmp->ship);
    VMnotify(player,"owner: %d",tmp->owner);
    VMnotify(player,"flags: %d",tmp->flag);
    AdvanceADSList(list);
  }
}


/* dump function for list */
int DumpADSList2Disk(t_ADSList *list,
		     FILE *myfile
		     ) { 
  /* requires: file must be valid and opened in some kind of write mode
     ensures: list is dumped to file in manner of 
     # of entries in list
     for all entries in list
     (ent->ship ent->flags ent->owner)
     -666
     returns: negative on error, positive indicates number of entries dumped 
     */
  int count = 0;
  t_ADSEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return NOLIST;
  }
  if (GetADSListSize(list) == 0) {
    return EMPLIST;
  }
  /* write count */
  fprintf(myfile,"%d\n",GetADSListSize(list));
  ResetADSList(list);
  while(!AtEndADSList(list)) {
    tmp = GetCurrADSEnt(list);
    if (tmp == NULL) {
      /* We have a big error, dump of list failed, and database is now corrupt
       */
      return BIGERR;
    }
    fprintf(myfile,"(%d %d %d)\n",tmp->ship,tmp->flag,tmp->owner);
   AdvanceADSList(list);
   count++;
  } 
  fprintf(myfile,"%d\n",-666);
  if (GetADSListSize(list) != count) {
    VMnotify(35,"Boss, count is different from Size, returning an error");
    return BIGERR;
  }
  return count;
}
    
  

