

#include "header.h"
#include "vmacctlist.h"

static char *rcsvmacctlist="$Id: vmacctlist.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";



t_AcctList *InitAcctList(void)
{

  t_AcctList *list;
  t_AcctEnt *tmp,*tmp2;
  list = (t_AcctList*) malloc(sizeof(t_AcctList)); 
  tmp = (t_AcctEnt *) malloc(sizeof(t_AcctEnt));
  tmp2 = (t_AcctEnt *) malloc(sizeof(t_AcctEnt));
  if (list == NULL) {
    VMnotify(35,"Bad malloc from list in InitAcctList");
    return;
  }
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp in InitAcctList");
    return;
  }
  if (tmp2 == NULL) {
    VMnotify(35,"Bad malloc from tmp2 in InitAcctList");
    return;
  }
  tmp->next= tmp2;
  list->start = tmp;
  list->end = tmp2;
  list->current = tmp;
  list->size = 0;
  return list;
}

void KillAcctList(list)
     t_AcctList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  ResetAcctList(list);
  while (!AtEndAcctList(list))
    {
      RemoveAcctEnt(list);
    }
  free(list->start);
  free(list->end);
}


void AddAcctEnt(list,num,bank,player,pin,cur,ir,amt,flags,expire)
     t_AcctList *list;
     int num;
     int bank;
     int player;
     int pin;
     int cur;
     double ir; 
     double amt;
     int flags;
     int expire;
{
  t_AcctEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  tmp = (t_AcctEnt *) malloc(sizeof(t_AcctEnt));
  if (tmp == NULL) {
    VMnotify(35,"Bad malloc from tmp from AddAcctEnt");
    return;
  }
  tmp->acct.num = num;
  tmp->acct.bank = bank;
  tmp->acct.player = player;
  tmp->acct.cur = cur;
  tmp->acct.pin = pin;
  tmp->acct.ir = ir; 
  tmp->acct.amt = amt;
  tmp->acct.flags = flags;
  tmp->acct.expire = expire;;
  tmp->next = list->current->next;
  list->current->next = tmp;
  list->size++;
}

void RemoveAcctEnt(list)
     t_AcctList *list;
{
  t_AcctEnt *dummy;
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

void AdvanceAcctList(list)
     t_AcctList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next == list->end)
    return;
  list->current = list->current->next;
}

void ResetAcctList(list)
     t_AcctList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  list->current = list->start;
}

t_AcctEnt *GetCurrAcctEnt(list)
     t_AcctList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  if (list->current->next != list->end)
    return list->current->next;

  return(NULL);
}

int AtEndAcctList(list)
     t_AcctList *list;

{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->current->next == list->end;
}

int GetAcctListSize(list)
     t_AcctList *list;
{
  if (list == NULL) {
    /* list is not valid */
    return;
  }
  return list->size;
}


/*Guarantees List will point to room if it exists after use*/

int IsAcctInAcctList(list,acct)
	t_AcctList *list;
	int acct;
{
  t_AcctEnt *tmp;
  if (list == NULL) {
    /* list is not valid */
    return 0;
  }
  ResetAcctList(list);
  while(!AtEndAcctList(list)) {
    tmp=GetCurrAcctEnt(list);
    if(tmp->acct.num == acct ) {
      return 1;
    }
    AdvanceAcctList(list);
 }
  return 0;
}


	






