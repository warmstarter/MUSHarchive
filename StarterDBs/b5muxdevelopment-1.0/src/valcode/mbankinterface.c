/* Mike's bank code */
#include "mbank.h"
void do_MCreateBank(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[5];
  int i,master, owner, obj;
  i=xcode_parse(buffer,fargs,3);
  if(i!=3) {
    VMnotify(player,"usage: create <master> <owner> <obj>");
    return;
  }
  master = atoi(fargs[0]);
  owner = atoi(fargs[1]);
  obj = atoi(fargs[2]);
  
  CreateBank(player, master,owner,obj);

} 


void do_SetIR(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[5];
  int i, bank;
  double ir;
  i=xcode_parse(buffer,fargs,2);
  if(i!=2) {
    VMnotify(player,"usage: setir <bank> <ir>");
    return;
  }
  bank = atoi(fargs[0]);
  sscanf(fargs[1], "%f",&ir);
  VMnotify(player,"S:%s * D:%10.3f",fargs[1],ir); 
  SetIR(player, bank, ir);

} 


void do_BankList(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  BankList(player);
} 

void do_SaveBanks(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  SaveBanksToFile(player);
} 

void do_LoadBanks(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  ReadBanksFromFile(player);
} 

void do_InitBanks(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  InitBanks(player);
} 

void do_BankSkel(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  VMnotify(player,"That command is not implemented yet.");
} 

void do_InitAccts(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  InitAccts(player);
}


void do_SaveAccts(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  SaveAccts(player,"That command is not implemented yet.");
} 



void do_LoadAccts(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  LoadAcct(player);
} 

void do_CreateAcct(player,data,buffer)
     int player;
     void *data;
     char *buffer;
{
  char *fargs[5];
  int i,play,Bank, bnum;
  i=xcode_parse(buffer,fargs,1);
  if(i!=1) {
    VMnotify(player,"usage: createacct <player>"); 
    return;
  }
  play = atoi(fargs[0]);
  Bank = GetBankFromPlayer(player);
  bnum = FindBank(Bank);
  if (bnum < 0 || bnum > NUMBANK) {
	VMnotify(player,"Improper setup for Bank in CreateAcct");
	return;
  }
  
} 


static char *rcsmbankinterfacec="$Id: mbankinterface.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
