/*
* Bank Code
*/

void do_access(player,data,buffer)
int player;
void *data;
void *buffer;
{
int i,bank,acc,area;
char *fargs[2];
char tst[20];

area=where_is(player);
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
		notify(player,"Useage: ACCESS <ACCOUNT> <CODE>");
		return;
	}
bank=match_thing(1,vget_a(where_is(player),SIZE));
if(bank==NOTHING) {
		notify(player,"Fatal Error!");
		return;
	}
acc=VMsmatch(vget_a(bank,SKILL_A),fargs[0]);
if(acc==0) {
		notify(player,"Account Unknown");
		return;
	}
if(verify_slist(bank,acc,ADVAN_A,fargs[1])!=1) {
		notify(player,"Incorrect Code");
		return;
	}

notify_all_from_inside(where_is(player),1,tprintf("%s accesses a bank account",Name(player)));
sprintf(tst,"%d",player);
atr_add_raw(area,RANK_A,tst);
sprintf(tst,"%d",acc);
atr_add_raw(area,CLASS_A,tst);
}


void do_deposit(player,data,buffer)
int player;
void *data;
void *buffer;
{
int i,bank,acc,area,x,mon;
char *fargs[2];
float tmp,tot,amt;

bank=match_thing(1,vget_a(where_is(player),SIZE));
if(bank==NOTHING) {
		notify(player,"Fatal Error!");
		return;
	}
area=where_is(player);
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
		notify(player,"Useage: DEPOSIT <AMOUNT in specified currency> <CURRENCY>");
		return;
	}
if(atoi(vget_a(area,RANK_A))!=player) {
		notify(player,"You are not accessing any account");
		return;
	}


acc=atoi(vget_a(area,CLASS_A));
amt=atof(fargs[0]);
if(amt <= 0) {
	notify(player,"Invalid Amount");
	return;
	}
x=-1;
for(x=1;x<=MONEYS;x++) {
	if(string_compare2(fargs[1],Moneys[x].name)==0) {mon=x;}
	}
if(x==-1) {
		notify(player,"Unknown Currency");
		return;
	}
tot=read_flist(player,mon,MONEY_A);
if(amt > tot) {
	notify(player,"You dont have that much");
	return;
	}
tot-=amt;
change_flist(player,mon,tot,MONEY_A);

tot=read_flist(bank,mon,MONEY_A);
tot+=amt;
change_flist(bank,mon,tot,MONEY_A);

tot=read_flist(bank,acc,ITEM_A);
tot+=amt*Moneys[mon].worth;
change_flist(bank,acc,tot,ITEM_A);
notify(player,"Money Deposited");

}

void do_withdraw(player,data,buffer)
int player;
void *data;
void *buffer;
{
int i,bank,acc,area,x,mon;
char *fargs[2];
float amt,tmp,tot;

bank=match_thing(1,vget_a(where_is(player),SIZE));
if(bank==NOTHING) {
		notify(player,"Fatal Error!");
		return;
	}
area=where_is(player);
i=xcode_parse(buffer,fargs,2);
if(i!=2) {
		notify(player,"Useage: WITHDRAW <AMOUNT in monetary units> <CURRENCY>");
		return;
	}
if(atoi(vget_a(area,RANK_A))!=player) {
		notify(player,"You are not accessing any account");
		return;
		}
acc=atoi(vget_a(area,CLASS_A));
amt=atof(fargs[0]);
if(amt <= 0) {
	notify(player,"Invalid Amount");
	return;
	}
x=-1;
for(x=1;x<=MONEYS;x++) {
	if(string_compare2(fargs[1],Moneys[x].name)==0) {mon=x;}
	}
if(x==-1) {
		notify(player,"Unknown Currency");
		return;
	}
tot=read_flist(bank,mon,MONEY_A)/Moneys[mon].worth;
if(amt > tot) {
	notify(player,"The Bank doesnt seem to be able to cover your withdraw in that currency.");
	return;
	}
tmp=read_flist(bank,acc,ITEM_A);
if(amt > tmp) {
	notify(player,"You dont have that much in your account.");
	return;
	}

tot-=amt/Moneys[mon].worth;
change_flist(bank,mon,tot,MONEY_A);

tot=read_flist(player,mon,MONEY_A);
tot+=amt/Moneys[mon].worth;
change_flist(player,mon,tot,MONEY_A);

tot=read_flist(bank,acc,ITEM_A);
tot-=amt;
change_flist(bank,acc,tot,ITEM_A);
notify(player,"Money Withdrawn");


}


void do_balance(player,data,buffer)
int player;
void *data;
void *buffer;
{
int i,bank,acc,area,x,mon,amt;
char *fargs[2];
float tmp,tot;
char tst[50];

bank=match_thing(1,vget_a(where_is(player),SIZE));
if(bank==NOTHING) {
		notify(player,"Fatal Error!");
		return;
	}
area=where_is(player);
if(atoi(vget_a(area,RANK_A))!=player) {
		notify(player,"You are not accessing any account");
		return;
		}
acc=atoi(vget_a(area,CLASS_A));
sprintf(tst,"%s",read_slist(bank,acc,SKILL_A));
notify(player,tprintf("------------------------------------------------\n   Account - %s\n\n   Balance = %f\n------------------------------------------------\n",tst,read_flist(bank,acc,ITEM_A)));


}

