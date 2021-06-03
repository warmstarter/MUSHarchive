void do_affect(player, cause, key, message)
dbref player, cause;
int key;
char *message;
{
dbref it;
char *wld;
dbref wd;
char *ptr[10];
char aff[100];


if(!IsIcReg(player)) return;

it=match_thing(player,message);
if(it==NOTHING) return;

if(!QuietIsIcReg(it)) {
	notify(player,tprintf("You can only affect registered IC players"));
	return;
	}

if((isPlayer(it) && (Flags2(it) & CONNECTED)) || (Flags2(it) & VEHICLE)) {

	wld=vget_a(player,ITEM);
	wd=match_thing(0,wld);
	if(wd==NOTHING || wd==0 || !(Flags3(wd) & ITEM))
		{
			notify(player,"You arent using a proper ITEM");
			return;
		}

	do_internal_affect(player,it,wd);
	}
else {notify(player,"You cant affect that");}
}

void do_internal_affect(player,it,wd)
dbref player,it,wd;
{
char *ptr,*ptrr[10],aff[100];
int typ;
double helth,bld,intu,fat;

ptr=vget_a(wd,A_VA);
typ=atoi(ptr);

/*type 2 is a comlink*/
if(typ==1) {
	sprintf(aff,"#%d",it);
	ptrr[0]=aff;
	did_it(player,wd,A_VA+2,NULL,A_VA+3,NULL,A_VA+4,ptrr,1);
	bld=read_flist(it,1,ATRIBS_A);
	intu=read_flist(it,2,ATRIBS_A);
	helth=5*bld;
	fat=5*intu;
	change_flist(it,6,helth,ATRIBS_A);
	change_flist(it,7,fat,ATRIBS_A);
	}	
else {
	notify(player,"Im not sure what your item does");
	}

}

void do_attack(player, cause, key, message)
dbref player, cause;
int key;
char *message;
{
dbref it;
char *wld,*ptrr[10],aff[100];
dbref wd;


if(!IsIcReg(player)) return;

it=match_thing(player,message);
if(it==NOTHING) return;

if(!QuietIsIcReg(it)) {
	notify(player,tprintf("You can only attack registered IC players"));
	return;
	}

if((isPlayer(it) && (Flags2(it) & CONNECTED)) || (Flags2(it) & VEHICLE)) {
	if(IsInCombat(&HealList,player)==1)
    		RemoveCombat(&HealList,player);
  	if(IsInCombat(&CombatQList,player)==1) 
    		RemoveCombat(&CombatQList,player);
  	AddCombat(&CombatQList,player,it);


	wld=vget_a(player,WIELD);
	wd=match_thing(0,wld);
	if(wd==NOTHING || wd==0 || !(Flags2(wd) & WEAPON))
		wd=WEAPONTHINGY;
	sprintf(aff,"#%d",it);
	ptrr[0]=aff;
	did_it(player,wd,A_VA+24,NULL,A_VA+25,NULL,WEAR,ptrr,1);
	}
else {notify(player,"You cant attack that");}
}


void do_stop(player, cause, extra)
dbref player, cause;
int extra;
{
if(IsInCombat(&CombatQList,player)==1) {
notify_all_from_inside(where_is(player),1,tprintf("%s has stopped attacking",Name(player)));
RemoveCombat(&CombatQList,player);
	}
else
	notify(player,"You arent attacking anyone"); 
	
}

void do_rest(player, cause, extra)
dbref player, cause;
int extra;
{
if(!IsIcReg(player)) return;

if(IsInCombat(&CombatQList,player)==1) {
	notify_all_from_inside(where_is(player),1,tprintf("%s has stopped attacking",Name(player)));
	RemoveCombat(&CombatQList,player);}
if(IsInCombat(&HealList,player)==0) {
	AddCombat(&HealList,player,player);
	notify(player,"You begin to rest");}
else notify(player,"You are already resting");
}




void do_wield(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
char wieldd[100];
char *wld;

if(!IsIcReg(player)) return;

it=match_thing(player,message);

if(where_is(it) != player) {
		notify(player,"You must be carrying the weapon to wield it.");
		return;
	}

if(it==NOTHING || (!(Flags2(it) & WEAPON))) {
	notify(player,"You cant wield that");
	return;
  }

wld=vget_a(player,WIELD);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0)
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);

s_Flags2(it,Flags2(it) & ~NO_COMMAND);

sprintf(wieldd,"#%d",it);
atr_add_raw(player,WIELD,wieldd);

did_it(player,it,A_VA+21,NULL,A_VA+22,NULL,A_VA+23,(char **)NULL,0);
/*notify(player,tprintf("You wield %s",message));*/
}


void do_unwield(player,cause,extra)
dbref player,cause;
int extra;
{
char wieldd[100],*wld;
dbref wd;

wld=vget_a(player,WIELD);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0) {
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);
	sprintf(wieldd," ");
	atr_add_raw(player,WIELD,wieldd);
did_it(player,wd,A_VA+17,NULL,A_VA+18,NULL,A_VA+19,(char **)NULL,0);

/*	notify(player,"You unwield %s",Name(wd));*/
	}
else notify(player,"You arent wielding anything");

}


void do_grab(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
char wieldd[100];
char *wld;

if(!IsIcReg(player)) return;

it=match_thing(player,message);

if(where_is(it) != player) {
		notify(player,"You must be carrying the item to grab it.");
		return;
	}

if(it==NOTHING || !(Flags3(it) & ITEM)) {
	notify(player,"You cant grab that");
	return;
  }

wld=vget_a(player,ITEM);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0)
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);

s_Flags2(it,Flags2(it) & ~NO_COMMAND);

sprintf(wieldd,"#%d",it);
atr_add_raw(player,ITEM,wieldd);

did_it(player,it,A_VA+21,NULL,A_VA+22,NULL,A_VA+23,(char **)NULL,0);
}


void do_release(player,cause,extra)
dbref player,cause;
int extra;
{
char wieldd[100],*wld;
dbref wd;

wld=vget_a(player,ITEM);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0) {
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);
	sprintf(wieldd," ");
	atr_add_raw(player,ITEM,wieldd);
did_it(player,wd,A_VA+17,NULL,A_VA+18,NULL,A_VA+19,(char **)NULL,0);

/*	notify(player,"You unwield %s",Name(wd));*/
	}
else notify(player,"You arent using any items");

}

