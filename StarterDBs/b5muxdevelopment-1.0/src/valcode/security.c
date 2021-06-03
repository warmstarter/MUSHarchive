/*
* Security 
*/

#include "header.h"


void do_press(player,cause,key,buffer)
int player,cause;
int key;
char *buffer;
{
int i,area;
char *fargs[2];
char tst[20];

i=xcode_parse(buffer,fargs,2);
if(i!=2) {
		notify(player,"Usage: PRESS <EXIT> <CODE>");
		return;
	}
init_match_check_keys(player,fargs[0],TYPE_EXIT);
match_exit();
area=match_result();
if(area==NOTHING || area==AMBIGUOUS) {
	notify(player,"Invalid exit.");
	return;
	}
sprintf(tst,"%s",vget_a(area,SKILL_A));
if(strcmp(fargs[1],tst)!=0) {
	notify(player,"Invalid code.");
	return;
	}
atr_clr(area,A_LOCK);
notify(player,"Exit unlocked.");
}





static char *rcssecurityc="$Id: security.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
