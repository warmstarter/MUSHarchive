/* $Id: vmsecurity.c,v 1.1 2001/01/15 03:23:20 wenk Exp $ */

#include "vmsecurity.h"
#include "header.h"
static char *rcsvmsecurityc="$Id: vmsecurity.c,v 1.1 2001/01/15 03:23:20 wenk Exp $";
int VMVerCurrent(int player,int room,int level,int area) 
{

if(Wizard(player)) return 1;
if(area==1)
if(atoi(vget_a(room,RANK_A)) >= level) return 1;
else if(area==2)
if(atoi(vget_a(room,CARGO)) >= level) return 1;
else if(area==3)
if(atoi(vget_a(room,MCARGO)) >= level) return 1;
else if(area==4)
if(atoi(vget_a(room,SIZE)) >= level) return 1;

VMnotify(player,"Access Denied");
return 0;
}


int VMCanAccessSecurity(int player)
{

if(atoi(vget_a(where_is(player),TITLE_A))==player) 
	{
	return 1;
	}
	VMnotify(player,"You arent manning the Security Console");
 return 0;
}

void VMSecAdd(int player,int entry,char *usrname,char *password,int level,int lev2,int lev3,int lev4)
{
int i;

if(!VMCanAccessSecurity(player)) return;

if(entry < 1 || entry > MAXSECENTRY) {
	VMnotify(player,"Invalid Entry Number");
	return;
	}

if(!VMVerCurrent(player,where_is(player),9,1)) return;

i=change_slist(where_is(player),entry,usrname,SKILL_A);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 1");
	return;
	}
i=change_slist(where_is(player),entry,password,ADVAN_A);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 2");
	return;
	}
i=change_ilist(where_is(player),entry,level,ATRIBS_A);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 3");
	return;
	}
i=change_ilist(where_is(player),entry,lev2,COMMOD_A);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 4");
	return;
	}
i=change_ilist(where_is(player),entry,lev3,WIELD);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 5");
	return;
	}
i=change_ilist(where_is(player),entry,lev4,WEAR);
if(i==-1) {
	VMnotify(player,"Entry Failed. Unknown Error 6");
	return;
	}

VMnotify(player,"Entry Added");
}


void VMSecView(int player)
{
int i,level,lev2,lev3,lev4;
char *usrname;
if(!VMCanAccessSecurity(player)) return;

if(!VMVerCurrent(player,where_is(player),3,1)) return;

VMnotify(player,"Security File");
VMnotify(player,"--------------------------------------------------------------");
for(i=1;i<=MAXSECENTRY;i++) {
 if((level=read_ilist(where_is(player),i,ATRIBS_A))!=0) {
 lev2=read_ilist(where_is(player),i,COMMOD_A);
 lev3=read_ilist(where_is(player),i,WIELD);
 lev4=read_ilist(where_is(player),i,WEAR);
    usrname=(char *) read_slist(where_is(player),i,SKILL_A);
   VMnotify(player,"%d) User: %10s  - Levels: %2d / %2d / %2d / %2d",i,usrname,level,lev2,lev3,lev4);
   }
 }
VMnotify(player,"--------------------------------------------------------------\n\n");

}

int VMSecGetVerify(int secroom,char *usr, char *pswrd,int *lev1,int *lev2,int *lev3,int *lev4)
{
int i;
  for(i=1;i<=MAXSECENTRY;i++) {
	if(strcmp((char *) read_slist(secroom,i,SKILL_A),usr)==0) {
		if(verify_slist(secroom,i,ADVAN_A,pswrd)) {
		*lev1=read_ilist(secroom,i,ATRIBS_A);
		*lev2=read_ilist(secroom,i,COMMOD_A);
		*lev3=read_ilist(secroom,i,WIELD);
		*lev4=read_ilist(secroom,i,WEAR);
		return 1;	
			}
		return 0;
	
	}
  }
return 0;
}


int VMSecVerify(int secroom,char *usr, char *pswrd,int level,int area)
{
int i,lev;
  for(i=1;i<=MAXSECENTRY;i++) {
	if(strcmp((char *) read_slist(secroom,i,SKILL_A),usr)==0) {
		if(verify_slist(secroom,i,ADVAN_A,pswrd)) { 

lev=-1;
if(area==1)
 lev=read_ilist(secroom,i,ATRIBS_A);
else if(area==2)
 lev=read_ilist(secroom,i,COMMOD_A);
else if(area==3)
 lev=read_ilist(secroom,i,WIELD);
else if(area==4)
 lev=read_ilist(secroom,i,WEAR);
		if(lev >= level)
			return 1;
		return 0;
		}
		return 0;
	}
  }
  return 0;
}


void VMSecTest(int player, char *usr, char *pswrd, int level)
{

if(VMSecVerify(where_is(player),usr,pswrd,level,1))
{
  VMnotify(player,"Security Verified");
  return;
}
  VMnotify(player,"Security Failed");

}



void VMSecMan(int player)
{
char tmp[10];
VMnotifyAll(where_is(player),"%s mans the security console",Name(player));
VMClearCurrent(where_is(player));
sprintf(tmp,"%d",player);
vput_attribute(where_is(player),"title",tmp);
}

void VMClearCurrent(int room)
{
char tmp[10];

sprintf(tmp,"-1");
vput_attribute(room,"class",tmp);
vput_attribute(room,"title",tmp);
vput_attribute(room,"rank",tmp);
vput_attribute(room,"cargo",tmp);
vput_attribute(room,"mcargo",tmp);
vput_attribute(room,"size",tmp);
}


void VMSecLogin(int player, char *usr, char *pswrd)
{
int level,lev2,lev3,lev4;
char tmp[10];

if(!VMCanAccessSecurity(player)) return;

if(!VMSecGetVerify(where_is(player),usr,pswrd,&level,&lev2,&lev3,&lev4))
	{
  		VMnotify(player,"Security Access Denied\n");
  		return;
	}

VMnotify(player,"You are now LOGGED IN %s Active Security level is %2d / %2d / %2d / %2d",usr,level,lev2,lev3,lev4);

sprintf(tmp,"%d",level);
vput_attribute(where_is(player),"class",usr);
vput_attribute(where_is(player),"rank",tmp);
sprintf(tmp,"%d",lev2);
vput_attribute(where_is(player),"cargo",tmp);
sprintf(tmp,"%d",lev3);
vput_attribute(where_is(player),"mcargo",tmp);
sprintf(tmp,"%d",lev4);
vput_attribute(where_is(player),"size",tmp);


}


void VMSecLogout(int player)
{
char tmp[10];

if(!VMCanAccessSecurity(player)) return;

VMnotify(player,"You are now LOGGED OUT");
sprintf(tmp,"-1");
vput_attribute(where_is(player),"class",tmp);
vput_attribute(where_is(player),"rank",tmp);
vput_attribute(where_is(player),"cargo",tmp);
vput_attribute(where_is(player),"mcargo",tmp);
vput_attribute(where_is(player),"size",tmp);

}



void VMSecRStat(int player)
{
char *usr;
int level,lev2,lev3,lev4,room;
if(!VMCanAccessSecurity(player)) return;


VMnotify(player,"Security Console Status");

room=where_is(player);
level=atoi(vget_a(room,RANK_A));

if(level==-1)
	{
	VMnotify(player,"You are not logged in");
	return;
	} 

lev2=atoi(vget_a(room,CARGO));
lev3=atoi(vget_a(room,MCARGO));
lev4=atoi(vget_a(room,SIZE));

usr=vget_a(room,CLASS_A);

VMnotify(player,"You are logged in as %s with Security Level %2d / %2d / %2d / %2d",usr,level,lev2,lev3,lev4);
}




