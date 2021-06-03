#include "header.h"
#include <stdio.h>
#include "vmacctlist.h"
#include "attrs.h"
t_AcctList *Accts;

static char *rcsidacct="$Id: acct.c,v 1.1 2001/01/15 03:23:19 wenk Exp $";

void InitAccts(int player) {

  Accts = NULL;
  Accts = (t_AcctList *) InitAcctList();
  if (Accts == NULL) {
    VMnotify(player,"Failed to init Accts.");
    return;
  }
  VMnotify(player,"Accounts DB initialized.");
}

void SaveAccts(int player) {

  FILE *myfile;
  t_AcctEnt *cur;
  if (Accts == NULL) {   
    VMnotify(player,"Accounts are not initialized.");
    return;      
  }
  myfile = fopen("vmacct.db","w");
  if (myfile == NULL) {
    VMnotify(player,"Cant open vmacct.db for writing.");
    return;
  }
  if (GetAcctListSize(Accts) == 0) {
    VMnotify(player,"Accounts are initialized, but no accts in db.");
    fprintf(myfile,"%d",0);
    return;
  }
  else {
    VMnotify(1,"%d entries being wrote to file",GetAcctListSize(Accts));
    fprintf(myfile,"%d",GetAcctListSize(Accts));
  }
  
  
  ResetAcctList(Accts);
  while (!AtEndAcctList(Accts)) {
    cur =(t_AcctEnt *) GetCurrAcctEnt(Accts);
    fprintf(myfile,"%d\n",cur->acct.num);
    fprintf(myfile,"%d %d\n",cur->acct.bank,cur->acct.player);
    fprintf(myfile,"%d %d\n",cur->acct.pin,cur->acct.cur);
    fprintf(myfile,"%15.5f\n",cur->acct.ir);
    fprintf(myfile,"%15.5f\n",cur->acct.amt);
    fprintf(myfile,"%d\n",cur->acct.flags);
    fprintf(myfile,"%d\n",cur->acct.expire);
    fflush(myfile);
    AdvanceAcctList(Accts);
  }
  fclose(myfile);
  VMnotify(player,"Account db saved.");
}

void LoadAcct(int player) {
  
  FILE *myfile;
  int numread = -1;
  int cnt =0;
  int num;
  int bank;
  int play;
  int pin;
  int i;
  int cur;
  double ir;
  double amt;
  int flags;
  int expire;
  
  if (Accts == NULL) {   
    VMnotify(player,"Accounts are not initialized.");
    return;      
  }
  myfile = fopen("vmacct.db","r");
  if (myfile == NULL) {
    VMnotify(player,"Cant open vmacct.db for reading.");
    return;
  }
  fscanf(myfile,"%d",&numread);
  VMnotify(1,"Will read in %d accts",numread);
  if (numread == -1) {
    VMnotify(1,"DB corrupt!");
    fclose(myfile);
    VMnotify(player,"DB load FAILED!");
    return;
  }
  cnt = 0;
  for (i = 0; i <= numread;i++) {
    fscanf(myfile,"%d",&num);
    fscanf(myfile,"%d %d",&bank,&play);
    fscanf(myfile,"%d %d",&pin,&cur);
    fscanf(myfile,"%f",&ir);
    fscanf(myfile,"%f",&amt);
    fscanf(myfile,"%d",&flags);
    fscanf(myfile,"%d",&expire);
    VMnotify(1,"Num: %d Bank: %d Player: %d Pin: %d Cur %d IR: %15.5f Amt: %20.3f flags: %d expire: %d",num,bank,play,pin,cur,ir,amt,flags,expire);
    AddAcctEnt(Accts,num,bank,play,pin,cur,ir,amt,flags,expire);
    cnt++;
  }
	VMnotify(player, "Actually read in %d entries.",cnt);
}

void CreateAcct(int player, int play, int bank, double ir, int pin ) {

	int bankpos;
	
  	int num;
  	int cur;
  	double amt;
  	int flags;
  	int expire;

	bankpos = FindBank(bank);
	
     	num = GetNewestAcctNum(bank);
	cur = Banks[bankpos].cur;
	flags = 0;
	expire = atoi(vget_a(Banks[bankpos].bobj,A_VA));
	amt = 0.0;
	flags = 0;
	AddAcctEnt(Accts,num,bank,play,pin,cur,ir,amt,flags,expire);
	VMnotify(player,"Account created.");
}

void DisplayAccount(int player, int acct) {
	t_AcctEnt *tmp;

	if (IsAcctInAcctList(Accts,acct)) {
		tmp = (t_AcctEnt *) GetCurrAcctEnt(Accts);
		DisplayAcctInternal(player,tmp);
	}
	else {
		VMnotify(player,"That Account Does not exist");
		return;
	}
}

void DisplayAcctInternal(int player, t_AcctEnt *acct) {
	int bnk;
	bnk = FindBank(acct->acct.bank);
	if (bnk == -1) {
		VMnotify("Bank for acct: %d unknown. exiting.",acct->acct.num);
		return;
	}
	VMnotify(player,"Account Number: %d",acct->acct.num);
  	VMnotify(player,"Bank: %s (id:%d)", Name(Banks[bnk].bobj),acct->acct.bank); 
	VMnotify(player,"Acct Owner: %s", Name(acct->acct.player));
	VMnotify(player,"Interest Rate: %6.3f",acct->acct.ir);
	VMnotify(player,"Amount in Account: %15.2f",acct->acct.amt);
}	

void DisplayAllAccts(int player, int acct) {
	t_AcctEnt *tmp;
	ResetAcctList(Accts);
	while (!AtEndAcctList(Accts)) {
		tmp = (t_AcctEnt *) GetCurrAcctEnt(Accts);
		AdvanceAcctList(Accts);
	}
}


int GetBankFromPlayer(int player) {
	int room;

	room = where_is(player);
	VMnotify(1,"player: %d room: %d Bank: %d",player,room,atoi(vget_a(room,A_VA)));
	return atoi(vget_a(room,A_VA));

}
int GetNewestAcctNum(int bank) {
	int anum,bnum;
	bnum = FindBank(bank);
	anum = bnum * 10000 + Banks[bnum].naccts + 1;
	VMnotify(1,"anum: %d",anum);
	return anum;
}
