
#include "header.h"
#include "combat.h"
#include "vmspace/vmspace.h"
#include <sys/time.h>

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
if(Flags3(where_is(player)) & REGISTERED) {
	notify(player,"You cant do that here");
	return;
	}

it=match_thing(player,message);
if(it==NOTHING) return;

if(!QuietIsIcReg(it)) {
	notify(player,tprintf("You can only affect registered IC players"));
	return;
	}

if((isPlayer(it) && (Flags2(it) & CONNECTED)) || (Flags2(it) & VEHICLE) || 
(Flags3(it) & MOB)) {

	wld=vget_a(player,ITEM_A);
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
char *ptra,*ptrr[10],aff[100],tmm[100],bff[100],bff2[100];
int typ,tim1,tim2,tim,skill;
dbref rac;
long tm;
double helth,bld,intu,fat;
char *ptr[10];
int amsg,omsg,chanc,dmg,dmg1,dmgmin,dmgmin1,dmgtmp,dmgtmp1;

ptra=vget_a(wd,A_VA);
typ=atoi(ptra);

/*type 2 is a comlink, 3 is ammo*/
 if(typ==1) {
	sprintf(aff,"#%d",it);
	ptrr[0]=aff;
	/*did_it(player,wd,A_VA+2,NULL,A_VA+3,NULL,A_VA+4,ptrr,1);*/
	bld=read_flist(it,1,ATRIBS_A);
	intu=read_flist(it,2,ATRIBS_A);
	helth=read_flist(it,6,ATRIBS_A);
	/*intu=read_flist(it,6,ATRIBS_A); Valdar's bug *NOTE* */
	fat=read_flist(it,7,ATRIBS_A);
	/*VMnotify(35,"helth: %2.4f fat: %2.4f",helth,fat);
	VMnotify(3,"helth: %2.4f fat: %2.4f",helth,fat);*/
	if(Wizard(player)) {
		helth=5*bld;
		fat=5*intu;
                notify(player,"You wizard heal the player.");
                notify(it,"You have been wizard healed.");
	/*VMnotify(35,"helth: %2.4f fat: %2.4f",helth,fat);
	VMnotify(3,"helth: %2.4f fat: %2.4f",helth,fat);*/
	}
	else {
		tim1=atoi(vget_a(it,MSKILL_A));
		tim2=(int)time(&tm);
		tim=tim2-tim1;
		if( tim < 1800) {
			notify(player,"There is nothing else you can do for this person now.");
			return;
		}
		sprintf(tmm,"%d",tim2);
		atr_add_raw(it,MSKILL_A,tmm);

	sprintf(bff,"%s",vget_a(it,RACE_A));
	rac=match_thing(it,bff);
	if(rac==NOTHING) return;
	sprintf(bff2,"%s",vget_a(rac,SKILL_A));
	skill=atoi(bff2);
	if(skill==0) skill=1;

	chanc=skill_check(player,skill,-2);

/*	VMnotify(35,"helth: %2.4f fat: %2.4f",helth,fat);
	VMnotify(3,"helth: %2.4f fat: %2.4f",helth,fat);*/
	if(chanc <0 ) {
		notify(player,"You are not able to help this person.");
		return;
	}

	else if(chanc < 3) {
		dmg=atoi(vget_a(wd,A_VA+1));
		dmgmin=atoi(vget_a(wd,A_VA+2));
		dmg1=atoi(vget_a(wd,RANK_A));
		dmgmin1=atoi(vget_a(wd,XC_A));
		amsg=A_VA+9;
		omsg=A_VA+10;
	}
	else if(chanc < 7) {
		dmg=atoi(vget_a(wd,A_VA+3));
		dmgmin=atoi(vget_a(wd,A_VA+4));
		dmg1=atoi(vget_a(wd,YC_A));
		dmgmin1=atoi(vget_a(wd,ZC_A));
		amsg=A_VA+11;
		omsg=A_VA+12;
	}
	else {
		dmg=atoi(vget_a(wd,A_VA+5));
		dmgmin=atoi(vget_a(wd,A_VA+6));
		dmg1=atoi(vget_a(wd,CLASS_A));
		dmgmin1=atoi(vget_a(wd,ADVAN_A));
		amsg=A_VA+13;
		omsg=A_VA+14;
	}


	dmgtmp=dmg-dmgmin; 
	if(dmgtmp<=0) dmgtmp=1;
	dmgtmp=random()%dmgtmp; 
	dmg=dmgtmp+dmgmin; 

	dmgtmp1=dmg1-dmgmin1; 
	if(dmgtmp1<=0) dmgtmp1=1;
	dmgtmp1=random()%dmgtmp1; 
	dmg1=dmgtmp1+dmgmin1; 

	helth+=dmg;
	fat+=dmg1;
	/*VMnotify(35,"helth: %2.4f fat: %2.4f",helth,fat);
	VMnotify(3,"helth: %2.4f fat: %2.4f",helth,fat);*/
	ptr[0]=it;
/*	did_it(player,wd,amsg,NULL,omsg,NULL,A_VA+20,ptr,1);*/
	notify_all_from_inside(where_is(player),1,tprintf("%s gives medical aid to %s",Name(player),Name(it)));
	}
	if(helth > 5*bld) helth=5*bld;
	if(fat > 5*intu) fat=5*intu;
	/*VMnotify(35,"helth: %2.4f fat: %2.4f",helth,fat);
	VMnotify(3,"helth: %2.4f fat: %2.4f",helth,fat);*/
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
if(Flags3(where_is(player)) & REGISTERED) {
	notify(player,"You cant do that here");
	return;
	}

it=match_thing(player,message);
if(it==NOTHING) return;

if(!QuietIsIcReg(it)) {
	notify(player,tprintf("You can only attack registered IC players"));
	return;
	}
if (it == player) {
        VMnotify(player,"Suicide is not an option.");
	return;
}

if((isPlayer(it) && (Flags2(it) & CONNECTED)) || (Flags2(it) & VEHICLE)  ||
(Flags3(it) & MOB)) {
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
	if(Flags3(it) & MOB) {
	if(IsInCombat(&HealList,it)==1)
    		RemoveCombat(&HealList,it);
  	if(IsInCombat(&CombatQList,it)==1) 
			return;
  	AddCombat(&CombatQList,it,player);
	wld=vget_a(it,WIELD);
	wd=match_thing(0,wld);
	if(wd==NOTHING || wd==0 || !(Flags2(wd) & WEAPON))
		wd=WEAPONTHINGY;
	sprintf(aff,"#%d",player);
	ptrr[0]=aff;
	did_it(it,wd,A_VA+24,NULL,A_VA+25,NULL,WEAR,ptrr,1);
		
		}
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
if(Flags3(where_is(player)) & REGISTERED) {
	notify(player,"You cant do that here");
	return;
	}

if(IsInCombat(&CombatQList,player)==1) {
	notify_all_from_inside(where_is(player),1,tprintf("%s has stopped attacking",Name(player)));
	RemoveCombat(&CombatQList,player);}
if(IsInCombat(&HealList,player)==0) {
	AddCombat(&HealList,player,player);
	notify(player,"You begin to rest");}
else notify(player,"You are already resting");
}


void do_wear(player,cause,key,message)
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
		notify(player,"You must be carrying the armor to wear it.");
		return;
	}

if(it==NOTHING || (!(Flags2(it) & ARMOR))) {
	notify(player,"You cant wear that");
	return;
  }

wld=vget_a(player,WEAR);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0)
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);

s_Flags2(it,Flags2(it) & ~NO_COMMAND);

sprintf(wieldd,"#%d",it);
atr_add_raw(player,WEAR,wieldd);

did_it(player,it,A_VA+21,NULL,A_VA+22,NULL,A_VA+23,(char **)NULL,0);
/*notify(player,tprintf("You wield %s",message));*/
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


void do_unwear(player,cause,extra)
dbref player,cause;
int extra;
{
char wieldd[100],*wld;
dbref wd;

wld=vget_a(player,WEAR);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0) {
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);
	sprintf(wieldd," ");
	atr_add_raw(player,WEAR,wieldd);
did_it(player,wd,A_VA+17,NULL,A_VA+18,NULL,A_VA+19,(char **)NULL,0);

/*	notify(player,"You unwield %s",Name(wd));*/
	}
else notify(player,"You arent wearing any armor");

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

void do_oocstuff(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
it=match_thing(player,message);

if (it == NOTHING) {
	return;
}
	if (!QuietIsIcReg(it)) {
		if (IsShipInSpaceList(VMAllPlayers,it)) {
			RemoveSpaceEnt(VMAllPlayers);
		}
	}
}

void do_eat(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
int tp;
double nv,fc,wc;
double bld,food,water,newfood;
int msg,omsg,ch;
double full,max,puke;
char buf[10];
ch = 0;
it=match_thing(player,message);
if (!IsIcReg(player)) {
	return;
}

	if (it == NOTHING )
		return;

	if (it == player) {
		VMnotify(player,"Kinky.. But you cant eat yourself.");
		return;
	} 
	if (isPlayer(it)) {
		VMnotify(player,"Kinky.. But you cant eat other players, I think ill tell them next time.");
		return;
	} 
	if (!(Flags3(it) & FOOD)) {
		VMnotify(player,"You cant eat that.");
		return;
	}
	tp = atoi(vget_a(it,A_VA));
	nv = atof(vget_a(it,A_VA+1));
	fc = atof(vget_a(it,A_VA+2));
	wc = atof(vget_a(it,A_VA+3));
	ch = atoi(vget_a(it,A_VA+10));
	if ( tp != 2 ) {
		VMnotify(player,"You can only EAT food.");
		return;
	}
	bld=read_flist(player,1,ATRIBS_A);
	food=read_flist(player,11,ATRIBS_A);
	water=read_flist(player,12,ATRIBS_A);
	msg = A_VA+8;
	omsg = A_VA+9;
	did_it(player,it,msg,NULL,omsg,NULL,0,tprintf("#%d",it),1);
	full = (bld * 10)/3;
	max = (bld * 12);
	puke = (bld *15);

	water -= wc; /* yes it costs water to eat */
	newfood = food + nv - fc;
	if (newfood > puke) {
		PukePlayer(player,newfood,food,it); /* player hurls */
		return;
	}
	if (newfood > max) {

		VMnotify(player,"You are really full.  You start to feel sick.");
	}
	if (newfood > full) {

		VMnotify(player,"You feel full.");
	}
	
	change_flist(player,11,newfood,ATRIBS_A);
	change_flist(player,12,water,ATRIBS_A);
	
	ch--;
	if (ch <=0) {
		move_via_generic(it,666,NOTHING,0);
		sprintf(buf,"#%d",it);
		do_destroy(1,1,DEST_OVERRIDE,buf);
	}
	else {
		sprintf(buf,"%d",ch);
		atr_add_raw(it,A_VA+10,buf);
	}


	/* TODO: add code to dest food object */
	/*
		VA = Type (1 water, 2 food, 3 alcholoic)
		VB = NutV - nutritional value
		VC = Energy cost to eat
		VD = Water cost
		VE = poison chance 
		VF = spoil time
	 	VG = expiration
		VH = Eat msg
		VI = OEat Msg
		VJ = spoiled eat msg
		VK = charges
	
	*/
 	


}

void PukePlayer(int player, double food, double ofood, int it) {

	struct timeval *tim;
	long sec,osec,dif;
	int i = 0;
	int msg,omsg;
		
	tim = (struct timeval *) malloc(sizeof(struct timeval));
	if (tim == NULL) {
		VMnotify(player,"Problem with Puking, contact an admin");
		return;
	}
	gettimeofday(tim,NULL);

	osec=read_flist(player,1,STRUCPTS_A);
	sec = tim->tv_sec;

	if (osec != 0) {

	dif = sec - osec;
	msg = A_VA+11;
	omsg = A_VA+12;
	did_it(player,it,msg,NULL,omsg,NULL,0,tprintf("#%d",it),1);
	if (dif < 7200) /* you may puke every 2 hrs w/o penalty */ {
		i = read_flist(player,2,STRUCPTS_A);	
		if ( i > 1 ) {
			if (i < 10) {
				HurtMent(player,i * 0.25);
				Hurt2(player,i * 0.05);
				VMnotify(player,"You do not feel well.");
			}
			else {	
				HurtMent(player,i * 0.35);
				Hurt2(player,i * 0.08);
				VMnotify(player,"You really feel bad.");
			}
		}
		i++;
		change_flist(player,2,i,STRUCPTS_A);
	}
	}
	change_flist(player,1,sec,STRUCPTS_A);
	
	
}


void do_drink(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
it=match_thing(player,message);
if (!IsIcReg(player)) {
        return;
}

        if (it == NOTHING )
                return;

        if (it == player) {
                VMnotify(player,"Kinky.. But you cant eat yourself.");
                return;
        }
        if (isPlayer(it)) {
                VMnotify(player,"Kinky.. But you cant eat other players, I think
 ill tell them next time.");
                return;
        }
        if (!(Flags3(it) & FOOD)) {
                VMnotify(player,"You cant eat that.");
                return;
        }
/*
        tp = atoi(vget_a(it,A_VA));
        nv = atof(vget_a(it,A_VA+1));
        fc = atof(vget_a(it,A_VA+2));
        wc = atof(vget_a(it,A_VA+3));
        ch = atoi(vget_a(it,A_VA+10));
        if ( tp == 2 ) {
                VMnotify(player,"You can only EAT food.");
                return;
        }

        bld=read_flist(player,1,ATRIBS_A);
        food=read_flist(player,11,ATRIBS_A);
        water=read_flist(player,12,ATRIBS_A);
        msg = A_VA+8;
        omsg = A_VA+9;
        did_it(player,it,msg,NULL,omsg,NULL,0,tprintf("#%d",it),1);
        full = (bld * 5);
        max = (bld * 12);
        puke = (bld *15);
	newwater = water + nv - wc;
	food = food - fc;


*/
}

void do_icstuff(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
it=match_thing(player,message);

if (it == NOTHING) {
	return;
}
	if (QuietIsIcReg(it)) {
		if (IsShipInSpaceList(VMAllPlayers,it)) {
			return;
		}
		else
			AddSpaceEnt(VMAllPlayers,it);
	}
}

void do_grab(player,cause,key,message)
dbref player,cause;
int key;
char *message;
{
dbref it,wd;
char wieldd[100];
char *wld;
char *typ;
int tp;

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

wld=vget_a(player,ITEM_A);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0)
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);

s_Flags2(it,Flags2(it) & ~NO_COMMAND);

sprintf(wieldd,"#%d",it);
atr_add_raw(player,ITEM_A,wieldd);

did_it(player,it,A_VA+21,NULL,A_VA+22,NULL,A_VA+23,(char **)NULL,0);
typ=vget_a(it,A_VA);
tp=atoi(typ);
if(tp==2) {
	sprintf(wieldd,"#%d",it);
	atr_add_raw(player,COMPUTER_A,wieldd);
	}

}


void do_release(player,cause,extra)
dbref player,cause;
int extra;
{
char wieldd[100],*wld;
dbref wd;

wld=vget_a(player,ITEM_A);
wd=match_thing(0,wld);
if(wd!=NOTHING && wd!=0) {
	s_Flags2(wd,Flags2(wd) | NO_COMMAND);
	sprintf(wieldd," ");
	atr_add_raw(player,ITEM_A,wieldd);
did_it(player,wd,A_VA+17,NULL,A_VA+18,NULL,A_VA+19,(char **)NULL,0);

/*	notify(player,"You unwield %s",Name(wd));*/
	}
else notify(player,"You arent using any items");

}

void do_reload(player,cause,extra)
dbref player,cause;
int extra;
{
char wieldd[100],*wld;
char *ptr,*tmp;
int cm1,cm2;
int typ,ammo;
dbref wd;
dbref item;
wld=vget_a(player,ITEM_A);
wd=match_thing(0,wld);
if(wd==NOTHING || wd==0 || !(Flags3(wd) & ITEM)) {
	notify(player,"You arent using any ammo.");
	return;
	}
item=wd;
ptr=vget_a(item,A_VA);
typ=atoi(ptr);
if(typ!=3) {
	notify(player,"You arent using any ammo.");
	return;
	}

	wld=vget_a(player,WIELD);
	wd=match_thing(0,wld);
if(wd==NOTHING || wd==0 || !(Flags2(wd) & WEAPON)) {
		notify(player,"Youa rent wielding a proper weapon.");
		return;
	}
tmp=vfetch_attribute(wd,"ammo");
ammo=atoi(tmp);
if(tmp==-5) {
	notify(player,"That weapon doesn't use ammo.");
	return;
	}
tmp=vget_a(item,COMPUTER_A);
cm1=atoi(tmp);
tmp=vget_a(wd,COMPUTER_A);
cm2=atoi(tmp);
if(cm1!=cm2) {
	notify(player,"That ammo doesnt work in the weapon.");
	return;
	}
tmp=vfetch_attribute(item,"ammo");
ammo=atoi(tmp);
sprintf(wieldd,"%d",ammo);
vput_attribute(wd,"ammo",wieldd);
notify_all_from_inside(where_is(player),1,tprintf("%s has reloaded.",Name(player)));
move_via_generic(item,where_is(WEAPONTHINGY),NOTHING,0);
	s_Flags2(item,Flags2(item) | NO_COMMAND);
	sprintf(wieldd," ");
	atr_add_raw(player,ITEM_A,wieldd);
}
static char *rcscombatcmdc="$Id: combatcmd.c,v 1.1 2001/01/15 03:23:15 wenk Exp $";
