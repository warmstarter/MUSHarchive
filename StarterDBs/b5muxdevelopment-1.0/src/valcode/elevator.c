#include "vmspace/vmspacelist.h"
#include "header.h"
#include "elevator.h"
#include <string.h>

static int einit=0;

void Elevator_Update()
{
t_SpaceEnt *tmp;
int max,i,floor,room,dest,ext;


if(einit==0) { 
	einit=1;
	for(i=0;i<MAXELEVS;i++) {
		Elevators[i].EQueue=InitSpaceList();
		Elevators[i].update=0;
	
		}
}
for(i=0;i<MAXELEVS;i++) {
	if(Elevators[i].update!=0) {
		ResetSpaceList(Elevators[i].EQueue);
		if(AtEndSpaceList(Elevators[i].EQueue))
		{
			Elevators[i].update=0;
		}
		else {
		tmp=GetCurrSpaceEnt(Elevators[i].EQueue);
		if(tmp==NULL)  {
				Elevators[i].update=0;
				break;	
			}
		else {
			dest=tmp->VMShip;
			room=Elevators[i].room;
			RemoveSpaceEnt(Elevators[i].EQueue);
			if(GetSpaceListSize(Elevators[i].EQueue)==0) {
				Elevators[i].update=0;
				}

ext=atoi(vget_a(room,A_VA+1));
s_Location(ext,dest);
notify_all_from_inside(room,1,tprintf("The elevator arrives at %s",Name(dest)));
notify_all_from_inside(dest,1,tprintf("The elevator doors open"));

			}
		}
			
	}
}


}

void call_elevator(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
int max,i,floor,room,dest,ext,elev;

dest=where_is(player);
room=atoi(vget_a(dest,A_VA));
elev=atoi(vget_a(room,A_VA+3));
Elevators[elev].room=room;
AddSpaceEnt(Elevators[elev].EQueue,dest);
Elevators[elev].update=1;

notify_all_from_inside(dest,1,tprintf("%s pushes the elevator button.",Name(player)));


}


void push_elevator(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
char *fargs[1];
int max,i,floor,room,dest,ext,elev;
i=xcode_parse(buffer,fargs,1);
if(i!=1) {
	notify(player,"Usage: PUSH <FLOOR>");
	return;
	}
floor=atoi(fargs[0]);
room=where_is(player);
max=atoi(vget_a(room,A_VA));
ext=atoi(vget_a(room,A_VA+1));
if(floor <1 || floor > max) {
	notify(player,"Invalid Floor");
	return;
	}
elev=atoi(vget_a(room,A_VA+3));
Elevators[elev].room=room;
Elevators[elev].update=1;
dest=read_ilist(room,floor,A_VA+2);
AddSpaceEnt(Elevators[elev].EQueue,dest);
notify_all_from_inside(room,1,tprintf("%s pushes the button marked %d.",Name(player),floor));
}

static char *rcselevatorc="$Id: elevator.c,v 1.1 2001/01/15 03:23:16 wenk Exp $";
