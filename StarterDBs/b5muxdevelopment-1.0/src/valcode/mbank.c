#include "mbank.h"
#include <stdio.h>

/* Init banks initializes all banks to be not in use */ 

void InitBanks(int player)  { 
  int i;
  for (i=0;i <NUMBANK;i++) {
    Banks[i].id = -4;
    Banks[i].pid = -4;
  }
  NumBanks = 0;
  VMnotify(player, "Banks initialized.");
  
}

void SaveBanksToFile(int player) {
  FILE *myfile;
  int i; 

  /* code */
  myfile = fopen("vmbank.db","w");
  fprintf(myfile,"%d\n",BDBVERS);
  for (i = 0; i <= NumBanks; i++) {
    fprintf(myfile, "%d %d\n",Banks[i].id,Banks[i].pid); /* id and parent id */ 
    fprintf(myfile, "%d %d\n",Banks[i].cur,Banks[i].owner); /* currency and Owner of Bank */ 
    fprintf(myfile, "%10.3f \n",Banks[i].ir);/* Interest Rate */
    fprintf(myfile, "%10.3f \n",Banks[i].money); /* Capital Funds Bank has to Loan */ 
    fprintf(myfile, "%10.3f \n",Banks[i].accts); /* # fund in accts */ 
    fprintf(myfile, "%10.3f \n",Banks[i].loans); /* # funds in loans outstanding */
    
    fprintf(myfile, "%d %d\n",Banks[i].nloans,Banks[i].naccts); /* Number of Loans and Number of accts */ 
    fprintf(myfile, "%d\n",Banks[i].flags);
    fprintf(myfile, "%d\n",Banks[i].bobj); 
    
  }
  fprintf(myfile,"-1 -1\n"); /* sentinal value */
  fflush(myfile);
  fclose(myfile);
  VMnotify(player, "Bank Database Saved.");
}


void ReadBanksFromFile(int player) {
  FILE *myfile;
  int i; 
  int vers;
  int depth = 0;
  int done = 0;
  int id, pid,naccts, nloans, cur, owner, flags, bobj;
  double ir,money,loans, accts;
  /* code */
  myfile = fopen("vmbank.db","r");
  fscanf(myfile,"%d",&vers);
  
  switch (vers) {
  case 1:
    NumBanks = 0;
    /* Bank DB version 1 */
    while (done == 0) {
      depth++;
      if (depth > 10000) {
	break;
      }
      fscanf(myfile,"%d %d",&id,&pid);
      if ((id == -1 && pid == -1) || id == -4){
	VMnotify(1,"Id has reach sentinal value");
	done = 1;
	continue;
      }
      VMnotify(1,"ID: %d  PID: %d",id,pid);
      Banks[NumBanks].id = id;
      Banks[NumBanks].pid = id;
      fscanf(myfile, "%d %d",&cur,&owner);

      VMnotify(1,"CUR: %d  OWNER: %d",cur,owner);
      Banks[NumBanks].cur = cur;
      Banks[NumBanks].owner = owner;
      
      fscanf(myfile, "%f", &ir);
      fscanf(myfile, "%f", &money);
      fscanf(myfile, "%f", &accts);
      fscanf(myfile, "%f", &loans);
      VMnotify(1,"IR: %10.3f  Money: %10.3f Accts: %10.3f Loans: %10.3f",ir,money,accts,loans);
      if (ir < 0 || ir > 100.0) {
	ir = 5.0;
	VMnotify(1,"Making IR 5...");
      }
      
      Banks[NumBanks].ir = ir;
      Banks[NumBanks].money = money;
      Banks[NumBanks].accts = accts;
      Banks[NumBanks].loans = loans;
      
      fscanf(myfile, "%d %d",&nloans, &naccts);
      fscanf(myfile, "%d", &flags);
      fscanf(myfile, "%d", &bobj); 

      VMnotify(1,"nloans: %d  macctsj: %d flags %d BOBJ: %d",nloans,naccts,flags,bobj);
      Banks[NumBanks].nloans = nloans;
      Banks[NumBanks].naccts = naccts;
      Banks[NumBanks].flags = flags;
      Banks[NumBanks].bobj = bobj;
      NumBanks++;
    }
  }    
  VMnotify(player,"Read in %d Banks",NumBanks);
}

void
CreateBank( 
	   int player, 
	   int master,
	   int owner,
	   int obj
	   ) {
  
  if (NumBanks == NUMBANK) {
    VMnotify(player, "No more banks left!");
    return;
  }
  if (Banks[NumBanks].id != -4) {
    NumBanks++;
  }
  VMnotify(1,"NumBanks: %d",NumBanks);

  Banks[NumBanks].id = NumBanks +1 * 3;
  Banks[NumBanks].pid = master;
  Banks[NumBanks].owner = owner;
  Banks[NumBanks].bobj = obj;

  Banks[NumBanks].money = 0.0;
  Banks[NumBanks].accts = 0.0;
  Banks[NumBanks].loans = 0.0;
  Banks[NumBanks].ir = 0.0;

  Banks[NumBanks].naccts = 0;
  Banks[NumBanks].nloans = 0;
  VMnotify(player,"Bank ID: %d Created.", Banks[NumBanks].id);
  NumBanks++;
 
}
 
int FindBank(int id) {
  
  int i;
  
  for (i = 0; i <= NumBanks; i++) {
    if (Banks[i].id == id) 
      return i;
  }
  return -1;
}



void SetIR(int player, int bankid, double irate) { 

  int bank;

  bank = FindBank(bankid);
  if (bank == -1) { 
    VMnotify(player, "That Bank doesn't exist.");
    return;
  }
  VMnotify(player,"%10.3f",irate);
  if (irate < 0.0) {
    VMnotify(player, "That rate is too low");
    return;
  }
  if (irate > 99.0) {
    VMnotify(player, "That rate is too high");
    return;
  }
  
  Banks[bank].ir = irate;
  VMnotify(player, "Interest rate set to %10.3f",Banks[bank].ir);

}

void BankList(int player) {
  int i;
  VMnotify(player,"-----------Bank List-----------");
  for (i = 0; i<NumBanks;i++) {
    VMnotify(player,"Bank ID: %d",Banks[i].id);
    VMnotify(player,"Master Bank: %d", Banks[i].pid);
    VMnotify(player,"Currency: %d", Banks[i].cur);
    VMnotify(player,"Owner: %d", Banks[i].owner);
    VMnotify(player,"Interest Rate: %10.3f", Banks[i].ir );
    VMnotify(player,"Funds Available: %10.3f", Banks[i].money);
    VMnotify(player,"Total Funds in Accts: %10.3f", Banks[i].accts);
    VMnotify(player,"Total Funds in Loans: %10.3f", Banks[i].loans);
    VMnotify(player,"# of Loans: %d", Banks[i].nloans);
    VMnotify(player,"# of Accts: %d",   Banks[i].naccts);
    VMnotify(player,"Bank Object: %d",Banks[i].bobj);
    VMnotify(player,"Bank Flags: %d", Banks[i].flags);
    VMnotify(player,"*******************************");
  }
}

 

      
static char *rcsmbankc="$Id: mbank.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
