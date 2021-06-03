
#include "header.h"
#include "combat.h"
#include "autocombat.h"


COMBATinit=1;
COMBATcount=0;
RESTcount=0;


void UpdateValsStuff()
{
if(COMBATinit)
{
 COMBATinit=0;
 InitCombat();
}

COMBATcount++;
RESTcount++;


if(RESTcount > 30) {
	RESTcount=0;
	RestAll();
	}
if(COMBATcount > 3) {
	COMBATcount=0;
	AttackAll();
	}


}


void InitCombat() {
CombatQList.size=0;
CombatQList.last=(Combat*) malloc(sizeof(Combat));
CombatQList.start=CombatQList.last;
CombatQList.last->player=-1;
CombatQList.last->next=CombatQList.start;

HealList.size=0;
HealList.last=(Combat*) malloc(sizeof(Combat));
HealList.start=HealList.last;
HealList.last->player=-1;
HealList.last->next=HealList.start;
}


int AddCombat(CQ,player,victim) 
CombatHeader *CQ;
dbref player;
dbref victim;{
Combat *tmp;
 if(!IsInCombat(CQ,player)) {
  tmp=(Combat*)malloc(sizeof(Combat));
  tmp->player=player;
  tmp->victim=victim;
  tmp->next=CQ->start;
  CQ->last->next=tmp;
  CQ->last=tmp;
  CQ->size++;
 }
else{return 0;}
return 1;
}


int IsInCombat(CQ,player) 
CombatHeader *CQ;
dbref player;{
int x=0;
Combat*tmp;
tmp=CQ->start->next;
while(tmp->player!=-1 && x==0) {
 if(tmp->player==player) x=1;
tmp=tmp->next;
 }
return x;  
}

int RemoveCombat(CQ,player)
CombatHeader *CQ;
dbref player;{
Combat*tmp,*tmplast;
int x=0;
tmp=CQ->start->next;
tmplast=CQ->start;
if(IsInCombat(CQ,player)) {
 while(tmp->player!=-1) {
  if(tmp->player==player) {
   x=1;	
   if(CQ->last==tmp) CQ->last=tmplast;
   tmplast->next=tmp->next;
   free(tmp);
   CQ->size--;
   break;
   } 
   tmplast=tmp;
   tmp=tmp->next;
  }
 }
else {return 0;}
return 1;
}

void contents() {
Combat*tmp;
int x=0;
tmp=CombatQList.start->next;
 while(tmp->player!=-1) {
  printf("Location: %d=%d\n",x,tmp->player);
  tmp=tmp->next;
  x++;
 }
}

void AttackAll() {
Combat  *tmp,*tmp2;
int x=0;

tmp=CombatQList.start->next;
 while(tmp->player!=-1) {
  tmp2=tmp->next;
  DoAttack(tmp);
  tmp=tmp2;
 }
}

void RestAll() {
Combat  *tmp,*tmp2;
int x=0;
tmp=HealList.start->next;
 while(tmp->player!=-1) {
  tmp2=tmp->next;
  if (QuietIsIcReg(tmp->player)) {
  DoRest(tmp);
  }
  tmp=tmp2;
 }
}

void DoRest(playe)
Combat *playe;
{
float helth,bld,intu,fat;
int play;

	bld=read_flist(playe->player,1,ATRIBS_A);
	intu=read_flist(playe->player,2,ATRIBS_A);
	fat=read_flist(playe->player,7,ATRIBS_A);
	helth=read_flist(playe->player,6,ATRIBS_A);
	helth+=.5;
	fat+=.5;
	play=playe->player;
	if(helth >= 5*bld)  
	{
		helth=5*bld;
	}
	
	if(fat >= 5*intu)
	{
		fat=5*intu;	
	}
	
	if(fat >= 5*intu && helth>= 5*bld)
	{
		RemoveCombat(&HealList,play);
		notify(play,"You are fully healed.");
	}
	else
	{
		notify(play,"You feel a little healthier.");
	}
	change_flist(play,6,helth,ATRIBS_A);
	change_flist(play,7,fat,ATRIBS_A);
}

void DoAttack(CombatIt) 
Combat *CombatIt;
{
int wield;
char *wld,wieldd[100];
char *ptr[10];
char vicc[100];
char vicc2[100];

int amsg,omsg,skil=1,mod=-2,schanc;
int dmg1,dmgmin1,dmgtmp1,chanc=0,dmgtmp,dmgmin=0,dmg=0;
char *tmp;
int ammo;

if(((Flags3(CombatIt->victim) & MOB) || (Flags2(CombatIt->victim) & CONNECTED) || (Flags2(CombatIt->victim) & VEHICLE)) && ((Flags2(CombatIt->player) & CONNECTED) || (Flags3(CombatIt->player) & MOB) || (Flags2(CombatIt->player) & VEHICLE)) && where_is(CombatIt->player)==where_is(CombatIt->victim)) 
	{

	wld=vget_a(CombatIt->player,WIELD);
	wield=match_thing(0,wld);
	if(wield==NOTHING || wield==0 || !(Flags2(wield) & WEAPON)) 
		wield=WEAPONTHINGY;

	if(Flags3(wield) & ITEM)
		wield=WEAPONTHINGY;

	tmp=vfetch_attribute(wield,"ammo");
	ammo=atoi(tmp);
	if(ammo!=-5) {
		if(ammo<=0) {
		notify_all_from_inside(where_is(CombatIt->player),1,tprintf("%s's weapon runs out of ammunition.",Name(CombatIt->player)));
		RemoveCombat(&CombatQList,CombatIt->player); 
		return;
/*
		s_Flags2(wield,Flags2(wield) | NO_COMMAND);
		sprintf(wieldd," ");
		atr_add_raw(CombatIt->player,WIELD,wieldd);
		did_it(CombatIt->player,wield,A_VA+17,NULL,A_VA+18,NULL,A_VA+19,(char **)NULL,0);
		wield=WEAPONTHINGY;
*/
		}
		ammo--;
		sprintf(wieldd,"%d",ammo);
		vput_attribute(wield,"ammo",wieldd);
	}		

	skil=atoi(vget_a(wield,A_VA));

	mod=-1*atoi(vget_a(wield,A_VA+7));

			/*1st check of attack hitting defender*/
	chanc=skill_check(CombatIt->player,skil,mod);	
	schanc=chanc;	/*save the chanc value for use later*/
	if(chanc < 0) {
		notify_all_from_inside(where_is(CombatIt->player),1,tprintf("%s misses %s",Name(CombatIt->player),Name(CombatIt->victim)));
		return;
	}

	chanc=skill_check(CombatIt->victim,16,chanc);
	if(chanc >= 0) {
		notify_all_from_inside(where_is(CombatIt->player),1,tprintf("%s dodges %s's attack.",Name(CombatIt->victim),Name(CombatIt->player)));
		return;
 	}

/* from this point on the attacker has hit his target*/

	if(chanc > -3) {
		dmg=atoi(vget_a(wield,A_VA+1));
		dmgmin=atoi(vget_a(wield,A_VA+2));
		dmg1=atoi(vget_a(wield,RANK_A));
		dmgmin1=atoi(vget_a(wield,XC_A));
		amsg=A_VA+9;
		omsg=A_VA+10;
	}
	else if(chanc > -7) {
		dmg=atoi(vget_a(wield,A_VA+3));
		dmgmin=atoi(vget_a(wield,A_VA+4));
		dmg1=atoi(vget_a(wield,YC_A));
		dmgmin1=atoi(vget_a(wield,ZC_A));
		amsg=A_VA+11;
		omsg=A_VA+12;
	}
	else {
		dmg=atoi(vget_a(wield,A_VA+5));
		dmgmin=atoi(vget_a(wield,A_VA+6));
		dmg1=atoi(vget_a(wield,CLASS_A));
		dmgmin1=atoi(vget_a(wield,ADVAN_A));
		amsg=A_VA+13;
		omsg=A_VA+14;
	}

	VMnotify(35,"dmg: %d dmgmin: %d",dmg,dmgmin);
	VMnotify(35,"dmg1: %d dmgmin1: %d",dmg1,dmgmin1);
	dmgtmp=dmg-dmgmin; 
	if(dmgtmp<=0) dmgtmp=1;
	dmgtmp=random()%dmgtmp; 
	dmg=dmgtmp+dmgmin; 

	dmgtmp1=dmg1-dmgmin1; 
	if(dmgtmp1<=0) dmgtmp1=1;
	dmgtmp1=random()%dmgtmp1; 
	dmg1=dmgtmp1+dmgmin1; 

	sprintf(vicc,"#%d",CombatIt->victim);
	ptr[0]=vicc;


/*
not working yet

sprintf(vicc2,"%s",Name(CombatIt->victim));
ptr[1]=vicc2;
*/
	VMnotify(35,"Dam: %d", dmg);
	did_it(CombatIt->player,wield,amsg,NULL,omsg,NULL,A_VA+20,ptr,1);
	Hurt(CombatIt->victim,(float)dmg);
	HurtMent(CombatIt->victim,(float)dmg1);

	}

else {RemoveCombat(&CombatQList,CombatIt->player); }

}

void Hurt(player,damage)
dbref player;
float damage;
{
double helth;
int armor,bonus;
float thresh;
char trsh[30];
char *ar;

ar=vget_a(player,WEAR);
armor=match_thing(0,ar);

/*notify(player,tprintf("Your armor is %d",armor));*/

if(armor > 0) {
thresh=atof(vget_a(armor,A_VA+1));
bonus=atoi(vget_a(armor,A_VA+2));
/*notify(player,tprintf("Your armor has %d/%d",thresh,bonus));*/
if(thresh > 0) {
     if(damage > thresh) {
	    thresh-=.1*damage;
		sprintf(trsh,"%f\0",thresh);
		atr_add_raw(armor,A_VA+1,trsh);

	  }
   damage-=bonus;
  }
}

if(damage < 0) damage=0;

helth=read_flist(player,6,ATRIBS_A);
helth=helth - damage;
change_flist(player,6,helth,ATRIBS_A);
if(helth < 0) {
		notify_all_from_inside(where_is(player),1,tprintf("%s has died!",Name(player)));
	move_via_generic(player,666,NOTHING,0);
	}
}

void HurtMent(player,damage)
dbref player;
float damage;
{
double helth;

helth=read_flist(player,7,ATRIBS_A);
helth=helth - damage;
change_flist(player,7,helth,ATRIBS_A);
}


void DoPlayerAttack(player) 
dbref player;{
int x=0;
Combat*tmp;
tmp=CombatQList.start->next;
while(tmp->player!=-1 && x==0) {
 if(tmp->player==player) DoAttack(tmp);
tmp=tmp->next;
 }
}

void BadHurt(player,damage)
dbref player;
float damage;
{
double helth;
int armor,bonus;
float thresh;
char trsh[30];
char *ar;



helth=read_flist(player,6,ATRIBS_A);
helth=helth - damage;
change_flist(player,6,helth,ATRIBS_A);
if(helth < 0) {
		notify_all_from_inside(where_is(player),1,tprintf("%s has died!",Name(player)));
	move_via_generic(player,666,NOTHING,0);
	}
}


void sust_update() {

	t_SpaceEnt *tmp;
	int object;

	ResetSpaceList(VMAllPlayers);
	while (!AtEndSpaceList(VMAllPlayers)) {
		tmp = GetCurrSpaceEnt(VMAllPlayers);
		object = tmp->VMShip;
		food=read_flist(object,11,ATRIBS_A);
		water=read_flist(object,12,ATRIBS_A);
	}
}	
static char *rcscombatc="$Id: com2.c,v 1.1 2001/01/15 03:23:15 wenk Exp $";



