/* $Id: vmrelaylist.h,v 1.1 2001/01/15 03:23:16 wenk Exp $ */

/* spacelist header file
 * Original code by ???
 * Change to space by Mike Wenk
 */
#ifndef _VMRELAYLISTH_
#define _VMRELAYLISTH_
typedef struct s_RelayList t_RelayList;
typedef struct s_RelayEnt t_RelayEnt;




struct s_RelayList {
  int size;
  t_RelayEnt *start;
  t_RelayEnt *end;
  t_RelayEnt *current;
};

struct s_RelayEnt {
  t_SpaceList *Relays[30];
  int VMID;
  int VMShip;
  int VMRoom;
  t_RelayEnt *next;
};



t_RelayList *InitRelayList(void);

void SemiKillRelayList(t_RelayList *list,int ShipNum);

void KillRelayList(t_RelayList *list);


void AddRelayEnt(t_RelayList *list,int VMship,int VMRoom, int VMID);

void RemoveRelayEnt(t_RelayList *list);

t_RelayEnt *SaveRelay(t_RelayList *list);

void RestoreRelay(t_RelayList *list,t_RelayEnt *item);
void AdvanceRelayList(t_RelayList *list);

void ResetRelayList(t_RelayList *list);

t_RelayEnt *GetCurrRelayEnt(t_RelayList *list);

int AtEndRelayList(t_RelayList *list);
int GetRelayListSize(t_RelayList *list);


int IsShipInRelayList(t_RelayList *list,int VMship);
static char *rcsvmrelaylisth="$Id: vmrelaylist.h,v 1.1 2001/01/15 03:23:16 wenk Exp $";
#endif
