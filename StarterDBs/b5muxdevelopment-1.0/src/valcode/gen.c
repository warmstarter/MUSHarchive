/*
 * lists.c --  Various funcitons used to read/write an attrib as a list
 */
/*
 */


#include "header.h"
#include "skills.h"
#include <string.h>


int new_playergen(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *tt;
int k,index=-1;
char buff[50],buff2[50];
index=-1;
for(k=1;k<=ValDBMax;k++) {
	if(ValDB[k].used==1 && ValDB[k].object==player) {
		index=k;
		break;
	}
}
if(index==-1) index=new_ValDB(player);
sprintf(buff2,"%d",index);
atr_add_raw(player,SKILL_A,buff2);
atr_add_raw(player,COMMOD_A,buff2);

sprintf(buff,"0 0 0 0 0 0 0 0 0 0 0 0");
atr_add_raw(player,ATRIBS_A,buff);
atr_add_raw(player,MONEY_A,buff);
sprintf(buff,"0");
atr_add_raw(player,WIELD,buff);
atr_add_raw(player,SIZE,buff);
atr_add_raw(player,CARGO,buff);
atr_add_raw(player,MCARGO,buff);
atr_add_raw(player,FACTION_A,buff);
atr_add_raw(player,TITLE_A,buff);
atr_add_raw(player,CLASS_A,buff);
atr_add_raw(player,XC_A,buff);
atr_add_raw(player,YC_A,buff);
atr_add_raw(player,ZC_A,buff);
atr_add_raw(player,RACE_A,buff);
atr_add_raw(player,ITEM_A,buff);
atr_add_raw(player,COMPUTER_A,buff);
atr_add_raw(player,WEAR,buff);
atr_add_raw(player,ADVAN_A,buff);
atr_add_raw(player,STRUCPTS_A,buff);

s_Flags3(player,Flags3(player) | PREREG);
clear_ValDB(index);
write_ValDB();
if(where_is(player)==6361)
VMnotify(player, "You are setup.  Choose your race an continue.");
else
VMnotify(player, "You are setup.  Set priorities, and then continue.");
}


int ValDB_Mark(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[3];
int i,rdb;
/*
if(!Wizard(player)) {
	notify(player,"Sorry. You are not allowed to do that");
	return;
	}
*/
i=xcode_parse(buffer,fargs,1);
if(i!=1) {
	notify(player,"Usage: MARK <VALDB #>");
	return;
	}
rdb=atoi(fargs[0]);
if(rdb > ValDBMax || rdb < 0) {
	notify(player,"Invalid DB #\n");
	return;
	}
if(ValDB[rdb].used==1) {
	ValDB[rdb].used=0;
	}
else ValDB[rdb].used=1;
	notify(player,"Used flag toggled");
}


int choose_pri(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[3],buff[50],*tt;
int p1,p2,p3;
int v1,v2,v3;
int k,i,index=-1;
i=xcode_parse(buffer,fargs,3);
if(i!=3) {
	notify(player,"Usage: PRIORITIES # # #\n");
	return;
}

for(k=0;k<=ValDBMax;k++) {
	if(ValDB[k].used==1 && ValDB[k].object==player) {
		index=k;
		break;
	}
}
if(index==-1) 
	{
		notify(player,"You must type NEW first.");
		return;
	}
clear_ValDB(index);
sprintf(buff,"0 0 0 0 0 0 0 0 0 0 0 0");
atr_add_raw(player,ATRIBS_A,buff);

sprintf(buff,"0");
atr_add_raw(player,XC_A,buff);
atr_add_raw(player,YC_A,buff);
atr_add_raw(player,ZC_A,buff);
p1=atoi(fargs[0]);
p2=atoi(fargs[1]);
p3=atoi(fargs[2]);
if(p1==1) {
	sprintf(buff,"22");
	atr_add_raw(player,XC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Attributes assigned first priority");
	}
if(p2==1) {
	sprintf(buff,"19");
	atr_add_raw(player,XC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Attributes assigned second priority");
	}
if(p3==1) {
	sprintf(buff,"16");
	atr_add_raw(player,XC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Attributes assigned third priority");
	}
if(p1==2) {
	sprintf(buff,"40");
	atr_add_raw(player,YC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Skills assigned first priority");
	}
if(p2==2) {
	sprintf(buff,"35");
	atr_add_raw(player,YC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Skills assigned second priority");
	}
if(p3==2) {
	sprintf(buff,"30");
	atr_add_raw(player,YC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Skills assigned third priority");
	}
if(p1==3) {
	sprintf(buff,"50000");
	atr_add_raw(player,ZC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Money assigned first priority");
	}
if(p2==3) {
	sprintf(buff,"15000");
	atr_add_raw(player,ZC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Money assigned second priority");
	}
if(p3==3) {
	sprintf(buff,"1000");
	atr_add_raw(player,ZC_A,buff);
if(where_is(player)!=6361)
	notify(player,"Money assigned third priority");
	}
tt=vget_a(player,XC_A);
v1=atoi(tt);
tt=vget_a(player,YC_A);
v2=atoi(tt);
tt=vget_a(player,ZC_A);
v3=atoi(tt);
if(where_is(player)!=6361)
notify(player,tprintf("You have\n%d Attribute Points\n%d Skills Points\n%d Money Points\nIf you dont like these points. You can use priority again.",v1,v2,v3));
}



int sbuy(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
int i,which,val;
float vall,lrn;
char *tt,buff[50];
i=xcode_parse(buffer,fargs,1);
if(!(Flags3(player) & PREREG)) {
	notify(player,"You must wipe your character first\n");
	return;
	}
if(i!=1) {
	notify(player,"Usage: SBUY SKILL#\n");
	return;
	}
which=atoi(fargs[0]);
if(which < 1 || which >= SKILLSs) {
	notify(player,"Invalid Skill #.");
	return;
	}

tt=vget_a(player,YC_A);
val=atoi(tt);
if(which==33) {
	notify(player,"That is a restricted skill.");
	return;
	}
if(val <= 0) {
	notify(player,"No skill points left.");
	return;
	}

lrn=read_flist(player,4,ATRIBS_A);
vall=read_flist(player,which,SKILL_A);
if(vall >= lrn) {
	notify(player,"No skill can exceed your Learn attribute.");
	return;
	}
val--;
sprintf(buff,"%d",val);
atr_add_raw(player,YC_A,buff);
vall++;
change_flist(player,which,vall,SKILL_A);
notify(player,tprintf("Skill %s(#%d) purchased. You have %d points left.",Skills[which].name,which,val));


}

int abuy(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
char *tt,buff[100];
int val;
int i,which;
float vall;
i=xcode_parse(buffer,fargs,1);

if(!(Flags3(player) & PREREG)) {
	notify(player,"You must wipe your character first\n");
	return;
	}
if(i!=1) {
	notify(player,"Usage: ABUY SKILL#\n");
	return;
	}
which=atoi(fargs[0]);
if(which < 1 || which > 5) {
	notify(player,"Invalid Attribute #\n 1 - Build\n 2 - Intuition\n 3 - Reflexes\n 4 - Learning\n 5 - Charisma");
	return;
	}

tt=vget_a(player,XC_A);
val=atoi(tt);
if(val <= 0) {
	notify(player,"No attribute points left.");
	return;
	}
val--;
sprintf(buff,"%d",val);
atr_add_raw(player,XC_A,buff);
vall=read_flist(player,which,ATRIBS_A);
vall++;
change_flist(player,which,vall,ATRIBS_A);
notify(player,tprintf("Attribute purchased. You have %d points left.",val));

tt=vget_a(player,ATRIBS_A);
sprintf(buff,"%s",tt);
atr_add_raw(player,SIZE,buff);


}


int do_done(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *tt;
int at,sk;
float bld,intu;
tt=vget_a(player,XC_A);
at=atoi(tt);
tt=vget_a(player,YC_A);
sk=atoi(tt);
if(at!=0 || sk!=0) {
		notify(player,"You cannot be done with points leftover.");
		return;
	}
s_Flags3(player,Flags3(player) | IC);
s_Flags3(player,Flags3(player) | REGISTERED);
write_ValDB();
bld=read_flist(player,1,ATRIBS_A);
change_flist(player,6,5*bld,ATRIBS_A);
intu=read_flist(player,2,ATRIBS_A);
change_flist(player,7,5*intu,ATRIBS_A);
notify(player,"You may continue now.");
}


int ssell(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
int i,which,val;
float vall,lrn;
char *tt,buff[50];
i=xcode_parse(buffer,fargs,1);
if(!(Flags3(player) & PREREG)) {
	notify(player,"You must wipe your character first\n");
	return;
	}
if(i!=1) {
	notify(player,"Usage: SSELL SKILL#\n");
	return;
	}
which=atoi(fargs[0]);
if(which < 1 || which >= SKILLS) {
	notify(player,"Invalid Skill #.");
	return;
	}

val=read_flist(player,which,SKILL_A);
if(val <= 0) {
	notify(player,"You dont have any more of that skill.");
	return;
	}

tt=vget_a(player,YC_A);
val=atoi(tt);
val++;

vall=read_flist(player,which,SKILL_A);
vall--;

sprintf(buff,"%d",val);
atr_add_raw(player,YC_A,buff);
change_flist(player,which,vall,SKILL_A);
notify(player,tprintf("Skill sold. You have %d points left.",val));


}

int asell(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
char *tt,buff[100];
int vdb,val;
int k,i,which;
float vall;
i=xcode_parse(buffer,fargs,1);

if(!(Flags3(player) & PREREG)) {
	notify(player,"You must wipe your character first\n");
	return;
	}
if(i!=1) {
	notify(player,"Usage: ASELL ATTRIBUTE#\n");
	return;
	}
which=atoi(fargs[0]);
if(which < 1 || which > 5) {
	notify(player,"Invalid Attribute #\n 1 - Build\n 2 - Intuition\n 3 - Reflexes\n 4 - Learning\n 5 - Charisma");
	return;
	}

vall=read_flist(player,which,ATRIBS_A);
if(vall <= 0) {
	notify(player,"You dont have any more of that attribute.");
	return;
	}
vall=read_flist(player,which,ATRIBS_A);
tt=vget_a(player,SKILL_A);
vdb=atoi(tt);
if(which==4) {
for(k=0;k<=SKILLS;k++) {
  if(ValDB[vdb].skills[k]>=vall) {
	notify(player,"Your learn cant be lower than a skill. Sell off excess skills first.");
	return;
	}
    }
}
tt=vget_a(player,XC_A);
val=atoi(tt);
val++;
sprintf(buff,"%d",val);
atr_add_raw(player,XC_A,buff);
vall--;
change_flist(player,which,vall,ATRIBS_A);
notify(player,tprintf("Attribute sold. You have %d points left.",val));
tt=vget_a(player,ATRIBS_A);
sprintf(buff,"%s",tt);
atr_add_raw(player,SIZE,buff);


}



static char *rcsgenc="$Id: gen.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
