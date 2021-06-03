
#include "header.h"



int IsIcReg();
int QuietIsIcReg();
int IsDead();
int QuietIsDead();
int IsSU();
int QuietIsSU();

int IsDead(int player) {
  if((Flags3(player) & IC)) {
    if((Flags3(player) & REGISTERED)) {
      if (Flags3(player) & DEAD)  {
	VMnotify(player,"You are dead.");
	return 0;
      }
      else 
	return 1;
    }
    else
      return 0;
  }
  else
    return 0;
}

int IsIcReg(player)
dbref player;
{
int num;
char *ptr;

if(!(Flags3(player) & REGISTERED)) {
	notify(player,"Only registered players may do that");
	return 0;
	}

if(Flags3(where_is(player)) & REGISTERED) return(1);
if(!(Flags3(player) & IC)) {
	notify(player,"Only IC players may do that");
	return 0;
	}

ptr=vget_a(player,SKILL_A);
num=atoi(ptr);
if(num > ValDBMax) {
	notify(player,"You have an invalid ValDB. See a wizard.");
	return 0;
	}

	return 1;
}


int QuietIsIcReg(player)
dbref player;
{
int num;
char *ptr;
if(!(Flags3(player) & REGISTERED)) return 0;
if(Flags3(where_is(player)) & REGISTERED) return(1);
if(!(Flags3(player) & IC)) return 0;
ptr=vget_a(player,SKILL_A);
num=atoi(ptr);
if(num > ValDBMax) return 0;

	return 1;
}


int xcode_parse();

int xcode_parse(buffer,args,maxargs)
char *buffer; 
char *args[]; 
int maxargs; 
{
  int count=0;
  char *parsed=buffer;
  int num_args=0;
  
  while((count < maxargs) && parsed) {
    if(!count) {
      /* first time through */
      parsed=strtok(buffer, " \t");
    } else {
      parsed=strtok(NULL, " \t");
    }
    args[count]=parsed;    /* Set the args pointer */
    if(parsed) num_args++; /* Actual count of arguments */
    count++;               /* Loop to make sure we don't overrun our */
                           /* buffer */
  }
  return (num_args);
}




static char *rcsxcodec="$Id: xcode.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
