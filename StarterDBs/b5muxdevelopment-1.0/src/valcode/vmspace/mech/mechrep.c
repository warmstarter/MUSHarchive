/*
  Mechrep.c

  File for mech repair stations
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>
#define POW_TEMPLATE 1
#include "config.h"
/*#include "externs.h"*/
#include "../header.h"
#include "mech.h"
#include "mechrep.h"
/*
  #include "weapons.h"
*/
extern struct weapon_struct MechWeapons[];

/* Selectors */
#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

extern char *strtok();
/*extern char *strncasecmp();*/

/* EXTERNS THAT SHOULDN"T BE IN HERE! */
extern void *FindObjectsData();

/*--------------------------------------------------------------------------*/
/* Code Begins                                                              */
/*--------------------------------------------------------------------------*/

void mechrep_Rdisplaysection(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mechrep_data *rep;
  struct mech_data *mech;
  char *args[1];
  int index;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *) FindObjectsData(rep->current_target);
	if(CheckData(player, mech)) {
	  if(1==mech_parseattributes(buffer, args, 1)) {
	    index=ArmorSectionFromString(mech->type, args[0]);
	    if(index!= -1) {
	      CriticalStatus(player, mech, index);
	    } else {
	      notify(player, "Invalid section!");
	    }
	  } else {
	    notify(player, "You must specify a section to list the criticals for!");
	  }
	}
      } else notify(player, "You must set a target first!");
    }
  }
}

void mechrep_Rsettech(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[2];
int argc;
struct mechrep_data *rep;
int newtech;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      argc=mech_parseattributes(buffer, args, 2);
      if(argc==1) {
	newtech=atoi(&args[0][1]); /* remove the # */
	/*if((newtech>0) && (newtech < mudstate.db_top) && Alive(newtech)) {*/
	if((newtech>0) && (newtech < mudstate.db_top) ) {
	  rep->tech=newtech;
	  notify(player, tprintf("Tech changed to #%d", newtech));
	} else {
	  notify(player, "That is not a valid player!");
	}
      } else {
	notify(player, "Too many arguments!");
      }
    }
}

void mechrep_Rsettarget(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[2];
int argc;
struct mechrep_data *rep;
int newmech;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      argc=mech_parseattributes(buffer, args, 2);
      if(argc==1) {
	newmech=atoi(&args[0][1]); /* Strip the # */
	if((newmech>0) && (newmech < mudstate.db_top) && 
		(Flags2(newmech) & CONSOLE)) {
	  rep->current_target=newmech;
	  notify(player, tprintf("Mech to repair changed to #%d", newmech));
	} else {
	  notify(player, "That is not a BattleMech or Vehicle!");
	}
      } else {
	notify(player, "Too many arguments!");
      }
    }
}


/* Alloc free function */
void newfreemechrep(key,data,selector)
dbref key; 
void **data; 
int selector; 
{
  struct mechrep_data *new;
  
  switch(selector) {
  case SPECIAL_ALLOC:
    new=(struct mechrep_data *) malloc(sizeof(struct mechrep_data));
    if(new) {
      new->mynum=key;
      new->money=10000;
      new->tech= -1;
      new->current_target= -1;
      (*data)= new;
    } else {
      log_important("Special Alloc for Mech repair failed!");
    }
    break;

  case SPECIAL_FREE:
    if(*data) {
      free(*data);
    }
    break;
  }
}


/* With cap R means restricted command */
void mechrep_Rsetspeed(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char *args[1], max[10];
float newmax;
struct mechrep_data *rep;
  
  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newmax=atof(args[0]);
	    mech->maxspeed=newmax*KPH_PER_MP;
	    sprintf(max, "%.2f", newmax);
	    notify(player, tprintf("Maxspeed changed to %s", max));
	  } else {
	    notify(player, "Invalid number of arguments to SetSpeed!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetheatsinks(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  int newmax;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newmax=atoi(args[0]);
	    mech->numsinks=newmax;
	    notify(player, tprintf("Heatsinks changed to %d", newmax));
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetjumpspeed(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], max[10];
  float newmax;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newmax=atof(args[0]);
	    mech->jumpspeed=newmax*KPH_PER_MP;
	    sprintf(max, "%.2f", newmax);
	    notify(player, tprintf("Jumpspeed changed to %s", max));
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetlrsrange(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], max[10];
  char newrange;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newrange=atoi(args[0]);
	    mech->lrs_range=newrange;
	    notify(player, tprintf("Long range sensors changed to %d hexes", 
				   newrange));
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsettacrange(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], max[10];
  char newrange;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newrange=atoi(args[0]);
	    mech->tac_range=newrange;
	    notify(player, tprintf("Tactical sensor range changed to %d hexes", 
				   newrange));
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetscanrange(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1], max[10];
  char newrange;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);

	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    newrange=atoi(args[0]);
	    mech->scan_range=newrange;
	    notify(player, tprintf("Scan range changed to %d hexes", 
				   newrange));
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

/* Mech repair/type commands */
void mechrep_Rsettons(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);  
	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    mech->tons=atoi(args[0]);
	    notify(player, tprintf("Mech tonnage set to %d",
				   mech->tons));
	  }
	} 
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsettype(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);  
	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    switch(args[0][0])
	      {
	      case 'm':
	      case 'M':
		mech->type=CLASS_MECH;
		notify(player, "Type set to MECH");
		break;
	      case 'g':
	      case 'G':
		mech->type=CLASS_VEH_GROUND;
		notify(player, "Type set to VEHICLE");
		break;
	      case 'v':
	      case 'V':
		mech->type=CLASS_VEH_VTOL;
		notify(player, "Type set to VTOL");
		break;
	      case 'n':
	      case 'N':
		mech->type=CLASS_VEH_NAVAL;
		notify(player, "Type set to NAVAL");
		break;
	      default:
		mech->type=CLASS_MECH;
		notify(player, "Types are: MECH, GROUND, VTOL and NAVAL");
		break;
	      }
	  }
	} 
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetmove(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target);  
	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    switch(args[0][0])
	      {
	      case 't':
	      case 'T':
		mech->move=MOVE_TRACK;
		notify(player, "Movement set to TRACKED");
		break;
	      case 'w':
	      case 'W':
		mech->move=MOVE_WHEEL;
		notify(player, "Movement set to WHEELED");
		break;
	      case 'h':
	      case 'H':
		switch(args[0][1])
		  {
		  case 'o':
		  case 'O':
		    mech->move=MOVE_HOVER;
		    notify(player, "Movement set to HOVER");
		    break;
		  case 'u':
		  case 'U':
		    mech->move=MOVE_HULL;
		    notify(player, "Movement set to HULL");
		    break;
		  }
		break;
	      case 'v':
	      case 'V':
		mech->move=MOVE_VTOL;
		notify(player, "Movement set to VTOL");
		break;
	      case 'f':
	      case 'F':
		mech->move=MOVE_FOIL;
		notify(player, "Movement set to FOIL");
		break;
	      default:
		notify(player, "Types are: TRACK, WHEEL, VTOL, HOVER, HULL and FOIL");
		break;
	      }
	  }
	} 
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

char *subdirs[] = {
    "3025",
    "3050",
    "3055",
    "2750",
    "MISC",
    "Clan",
    "Vehicles"
};

int numsubdirs = 7;

void mechrep_Rloadnew(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char *args[1];
char openfile[50];
struct mech_data readmech;
FILE *fp;
dbref mynum;
struct mechrep_data *rep;
int i, j;
int i1,i2,i3,i4,i5,i6;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
	
	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    sprintf(openfile, "mechs/%s", args[0]);
	    fp = fopen(openfile, "r");
	    for (i = 0; !fp && i < numsubdirs; i++)
	    {
		sprintf(openfile, "mechs/%s/%s",subdirs[i], args[0]);
		fp = fopen(openfile, "r");
	    }
	    if(fp) {
	      if (i == 0)
		  notify(player, tprintf("Loading %s", args[0]));
	      else
		  notify(player, tprintf("Loading %s from %s dir", 
					args[0],subdirs[i-1]));
	      /*mystrncpy(mech->mech_type, args[0],31);*/
	      strncpy(mech->mech_type, args[0],31);
	      mech->status=TORSO_NORMAL;
              mech->pilotskill = 5;
              mech->gunneryskill = 4;
	      mech->critstatus=0;
	      mech->pilot= -1;
              mech->aim= NUM_SECTIONS;
	      mech->pilotstatus=0;
	      mech->unconcious=0;
	      mech->pilotskillbase=0;
	      mech->turndamage=0;
	      mech->weapheat=0;
	      mech->heat=0;
	      mech->engineheat=0;
	      mech->basetohit=0;
	      mech->speed=0;
	      mech->desired_speed=0;
	      mech->facing = 0;
	      mech->desiredfacing = 0;
	      mech->goingx = mech->goingy = 0;
	      mech->jumpheading = 0;
	      mech->target = -1;
	      mech->chgtarget = -1;
	      mech->dfatarget = -1;
	      mech->targx = -1;
	      mech->targy = -1;
	      mech->clock = 0;
	      for(i=0; i<NUM_SECTIONS; i++) {
	        mech->sections[i].armor=0;
	        mech->sections[i].internal=0;
	        mech->sections[i].rear=0;
	        mech->sections[i].basetohit=0;
	        mech->sections[i].config=0;
	        FillDefaultCriticals(mech, i);
	      }

	      fscanf(fp, "%d %d %d %d %d %f %f %d\n",&i1,&i2,&i3,&i4,&i5,
		     &mech->maxspeed, &mech->jumpspeed, &i6);
	      mech->tons=i1; mech->tac_range=i2; 
	      mech->lrs_range=i3; mech->scan_range=i4;
	      mech->numsinks=i5; mech->specials=i6;
	      for(i=0;i<NUM_SECTIONS;i++) 
		{
		  fscanf(fp, "%d %d %d %d\n", &i1,&i2,&i3,&i4);
		  mech->sections[i].armor=i1; 
		  mech->sections[i].internal=i2; 
		  mech->sections[i].rear=i3;
		  mech->sections[i].config=i4;
		  for(j=0;j<NUM_CRITICALS;j++) 
		    {
		      fscanf(fp, "%d %d %d\n", &i1,&i2,&i3); 
		      mech->sections[i].criticals[j].type=i1; 
		      mech->sections[i].criticals[j].data=i2;
		      mech->sections[i].criticals[j].mode=i3;
		    } 
		}
	      fscanf(fp, "%d %d\n", &i1, &i2);
	      mech->type = i1;
	      mech->move = i2;
	      notify(player, "Loading complete!");
	      fclose(fp);
	      } else {
		notify(player, "Unable to read that template.");
	      }
	    } else {
	      notify(player, "Unable to load that template.");
	    }
	  }
	} else {
	  notify(player, "You must set a target for repairs.");
	}
    }
}

void mechrep_Rrestore(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char openfile[50];
struct mech_data readmech;
FILE *fp;
dbref mynum;
struct mechrep_data *rep;
int i, j;
int i1,i2,i3,i4,i5,i6;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
	
	if(CheckData(player, mech)) {
	  if (strlen(mech->mech_type) == 0)
	  {
	      notify(player,"Sorry, I don't know what type of mech this is");
	      return;
	  }
	  sprintf(openfile, "mechs/%s", mech->mech_type);
	  fp = fopen(openfile, "r");
	  for (i = 0; !fp && i < numsubdirs; i++)
	  {
		sprintf(openfile, "mechs/%s/%s",subdirs[i], mech->mech_type);
		fp = fopen(openfile, "r");
	  }
	  if(fp) {
	    if (i == 0)
		notify(player, tprintf("Restoring %s", mech->mech_type));
	    else
	        notify(player, tprintf("Restoring %s from %s dir", 
					mech->mech_type,subdirs[i-1]));

	    mech->status=TORSO_NORMAL;
            mech->pilotskill = 5;
            mech->gunneryskill = 4;
	    mech->critstatus=0;
	    mech->pilot= -1;
	    mech->pilotstatus=0;
            mech->aim= NUM_SECTIONS;
	    mech->unconcious=0;
	    mech->pilotskillbase=0;
	    mech->turndamage=0;
	    mech->weapheat=0;
	    mech->heat=0;
	    mech->engineheat=0;
	    mech->basetohit=0;
	    mech->speed=0;
	    mech->desired_speed=0;
	    mech->facing = 0;
	    mech->desiredfacing = 0;
	    mech->goingx = mech->goingy = 0;
	    mech->jumpheading = 0;
	    mech->target = -1;
	    mech->chgtarget = -1;
	    mech->dfatarget = -1;
	    mech->targx = -1;
	    mech->targy = -1;
	    mech->clock = 0;
	    for(i=0; i<NUM_SECTIONS; i++) {
	      mech->sections[i].armor=0;
	      mech->sections[i].internal=0;
	      mech->sections[i].rear=0;
	      mech->sections[i].basetohit=0;
	      mech->sections[i].config=0;
	      FillDefaultCriticals(mech, i);
	    }

	    fscanf(fp, "%d %d %d %d %d %f %f %d\n", &i1,&i2,&i3,&i4,&i5,
		   &mech->maxspeed, &mech->jumpspeed, &i6);
	    mech->tons=i1; 
	    mech->tac_range=i2; 
	    mech->lrs_range=i3; 
	    mech->scan_range=i4;
	    mech->numsinks=i5; 
	    mech->specials=i6;
	    for(i=0;i<NUM_SECTIONS;i++) 
	      {
		fscanf(fp, "%d %d %d %d\n", &i1,&i2,&i3,&i4);
		mech->sections[i].armor=i1;
		mech->sections[i].internal=i2;
		mech->sections[i].rear=i3;
		mech->sections[i].config=i4;
		for(j=0;j<NUM_CRITICALS;j++) 
		  {
		    fscanf(fp, "%d %d %d\n", &i1,&i2,&i3);
		    mech->sections[i].criticals[j].type=i1;
		    mech->sections[i].criticals[j].data=i2;
		    mech->sections[i].criticals[j].mode=i3;
		  } 
	      }
	    fscanf(fp, "%d %d\n", &i1, &i2);
	    mech->type = i1;
	    mech->move = i2;
	    notify(player, "Restoration complete!");
	    fclose(fp);
	  } else {
	    notify(player, "Unable to restore this mech!.");
	  }
	}
      } else {
	notify(player, "You must set a target for restoration.");
      }
    }
}

void mechrep_Rsavetemp(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[1];
  FILE *fp;
  char openfile[50];
  struct mechrep_data *rep;
  int i, j;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
	
	if(CheckData(player, mech)) {
	  if(mech_parseattributes(buffer, args, 1)==1) {
	    notify(player, tprintf("Saving %s", args[0]));
	    sprintf(openfile, "mechs/");
	    strcat(openfile, args[0]);
	    if((fp=fopen(openfile, "w"))) {
	      fprintf(fp, "%d %d %d %d %d %.2f %.2f %d\n", 
		      mech->tons, mech->tac_range, 
		      mech->lrs_range,mech->scan_range,mech->numsinks, 
		      mech->maxspeed, mech->jumpspeed,
		      mech->specials);
	      for(i=0;i<NUM_SECTIONS;i++) 
		{
		  fprintf(fp, "%d %d %d %d\n", mech->sections[i].armor, 
			  mech->sections[i].internal, mech->sections[i].rear,
			  mech->sections[i].config);
		  for(j=0;j<NUM_CRITICALS;j++) 
		    {
		      fprintf(fp, "%d %d %d\n", 
			      mech->sections[i].criticals[j].type, 
			      mech->sections[i].criticals[j].data,
			      mech->sections[i].criticals[j].mode);
		    } 
		}
	      fprintf(fp, "%d %d\n", mech->type, mech->move);
	      notify(player, "Saving complete!");
	      fclose(fp);
	    } else {
	      notify(player, "Unable to open/create mech file! Sorry.");
	    }
	  } else {
	    notify(player, "You must specify a template name!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rsetarmor(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[4];
  int argc;
  int index;
  int temp;
  struct mechrep_data *rep;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!= -1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
 
	if(CheckData(player, mech)) {
	  argc=mech_parseattributes(buffer, args, 4);
	  if(argc > 1) {
	    index=ArmorSectionFromString(mech->type, args[0]);
	    if(index== -1) {
	      notify(player, "Not a legal area. Must be HEAD, CTORSO");
	      notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	      notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	      return;
	    }
	    
	    argc--;
	    if(argc) {
	      temp=atoi(args[1]);
	      if(temp<0) {
		notify(player, "Invalid armor value!");
	      } else {
		notify(player, "Front armor set!");
		mech->sections[index].armor=temp;
	      }
	      argc--;
	    }
	    
	    if(argc) {
	      temp=atoi(args[2]);
	      if(temp<0) {
		notify(player, "Invalid Internal armor value!");
	      } else {
		notify(player, "Internal armor set!");
		mech->sections[index].internal=temp;
	      }
	      argc--;
	    }	
	    
	    if(argc) {
	      temp=atoi(args[3]);
	      if(index==CTORSO || index==RTORSO || index==LTORSO) {
		if(temp<0) {
		  notify(player, "Invalid Rear armor value!");
		} else {
		  notify(player, "Rear armor set!");
		  mech->sections[index].rear=temp;
		}
	      } else {
		notify(player, "Only the torso can have rear armor.");
	      }
	    }	
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Raddweap(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[20];
  int argc;
  int index;
  int weapindex;
  int loop, temp;
  struct mechrep_data *rep;
  int isrear = 0;
  int istc = 0;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!= -1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 

	if(CheckData(player, mech)) {
	  argc=mech_parseattributes(buffer, args, 20);
	  if(argc >= 3) {
	    index=ArmorSectionFromString(mech->type, args[1]);
	    if(index==-1) {
	      notify(player, "Not a legal area. Must be HEAD, CTORSO");
	      notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	      notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	      return;
	    }
	    weapindex=WeaponIndexFromString(args[0]);
	    if(weapindex==-1) {
	      notify(player, "That is not a valid weapon!");
	      notify(player, "Valid weapons are:");
	      DumpWeapons(player);
	      return;
	    }

	    if (args[argc-1][0] == 'T' || args[argc-1][0] == 't' ||
	        args[argc-1][1] == 'T' || args[argc-1][1] == 't')
	    {
		istc = 1;
	    }
	
	    if (args[argc-1][0] == 'R' || args[argc-1][0] == 'r' ||
	        args[argc-1][1] == 'R' || args[argc-1][1] == 'r')
	    {
		isrear = 1;
	    }

	    if (isrear || istc)
		argc--;
	
	    /* Subtract off our two arguments */
	    argc-=2;
	    if(argc<GetWeaponCrits(mech,weapindex)) {
	      notify(player, "Not enough critical slots specified!");
	    } else {
	      for(loop=0; loop<GetWeaponCrits(mech,weapindex); loop++) {
		temp=atoi(args[2+loop]);
		temp--; /* From 1 based to 0 based */
		if(temp<0 || temp>NUM_CRITICALS) {
		  notify(player, "Bad critical location!");
		  return;
		}
		mech->sections[index].criticals[temp].type=
		  (weapindex + WEAPON_BASE_INDEX);
	        mech->sections[index].criticals[temp].mode = 0;
                if (isrear)
		  mech->sections[index].criticals[temp].mode |= REAR_MOUNT;
		if (istc)
		  mech->sections[index].criticals[temp].mode |= ON_TC;
	      }
	      if(weapindex==IS_AMS)
		mech->specials |= IS_ANTI_MISSILE_TECH;
	      if(weapindex==CL_AMS)
		mech->specials |= CL_ANTI_MISSILE_TECH;

	      notify(player, "Weapon added.");
	    }   
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rreload(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
struct mech_data *mech;
char *args[4];
int argc;
int index;
int weapindex;
int subsect;
struct mechrep_data *rep;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 

	if(CheckData(player, mech)) {
	  argc=mech_parseattributes(buffer, args, 4);
	  if(argc > 2) {
	    weapindex=WeaponIndexFromString(args[0]);
	    if(weapindex==-1) {
	      notify(player, "That is not a valid weapon!");
	      notify(player, "Valid weapons are:");
	      DumpWeapons(player);
	      return;
	    }
	    
	    index=ArmorSectionFromString(mech->type, args[1]);
	    if(index==-1) {
	      notify(player, "Not a legal area. Must be HEAD, CTORSO");
	      notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	      notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	      return;
	    }
	    
	    subsect=atoi(args[2]);
	    subsect--; /* from 1 based to 0 based */
	    if(subsect<0 || subsect >NUM_CRITICALS) {
	      notify(player, "Subsection out of range!");
	      return;
	    } 
	    if(MechWeapons[weapindex].ammoperton==0) {
	      notify(player, "That weapon doesn't require ammo!");
	    } else {
	      mech->sections[index].criticals[subsect].type=
		(AMMO_BASE_INDEX + weapindex);
	      mech->sections[index].criticals[subsect].data=
		MechWeapons[weapindex].ammoperton;
	      mech->sections[index].criticals[subsect].mode=0;
              if(argc > 3) 
                {
                  if(args[3][0] == 'L' || args[3][0] == 'l')
                    mech->sections[index].criticals[subsect].mode |= LBX_MODE;
                  else if(args[3][0] == 'A' || args[3][0] == 'a')
                    mech->sections[index].criticals[subsect].mode |= ARTEMIS_MODE;
                  else if(args[3][0] == 'N' || args[3][0] == 'n')
                    mech->sections[index].criticals[subsect].mode |= NARC_MODE;
                }
	      notify(player, "Weapon loaded!");
	    }
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      } else {
	notify(player, "You must set a target for repairs.");
      }
    }
}

void mechrep_Rrepair(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[4];
  int argc;
  int index;
  int temp;
  struct mechrep_data *rep;

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
 
	if(CheckData(player, mech)) {
	  argc=mech_parseattributes(buffer, args, 4);
	  if(argc > 2) {
	    index=ArmorSectionFromString(mech->type, args[0]);
	    if(index==-1) {
	      notify(player, "Not a legal area. Must be HEAD, CTORSO");
	      notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	      notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	      return;
	    }
	    
	    temp=atoi(args[2]);
	    if(temp<0) {
	      notify(player, "Illegal value for armor!");
	    }
	    
	    switch(args[1][0]) {
	    case 'A':
	    case 'a':
	      /* armor */
	      mech->sections[index].armor=temp;
	      notify(player, "Armor repaired!");
	      break;
	    case 'I':
	    case 'i':
	      /* internal */
	      mech->sections[index].internal=temp;
	      notify(player, "Internal structure repaired!");
	      break;
	    case 'C':
	    case 'c':
	      /* criticals */
              temp--;
	      if(temp >= 0 && temp<NUM_CRITICALS) {
		mech->sections[index].criticals[temp].data=0;
		notify(player, "Critical location repaired!");
	      } else {
		notify(player, "Critical Location out of range!");
	      }
	      break;
	    case 'R':
	    case 'r':
	      /* rear */
	      if(index==CTORSO || index==LTORSO || index==RTORSO) {
		mech->sections[index].rear=temp;
		notify(player, "Rear armor repaired!");
	      } else {
		notify(player, 
		   "Only the center, rear and left torso have rear armor!");
	      }
	      break;
	    default:
	      notify(player, "Illegal Type-> must be ARMOR, INTERNAL, CRIT, REAR");
	      return;
	    }
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      }  else {
	notify(player, "You must set a target for repairs.");
      }
    }
}


/*
  ADDSP <ITEM> <LOCATION> <SUBSECT> [<DATA>]
*/
void mechrep_Raddspecial(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  char *args[4];
  int argc;
  int index;
  int itemcode;
  int subsect;
  int newdata;
  struct mechrep_data *rep;
  int max;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
 
	if(CheckData(player, mech)) {
	  argc=mech_parseattributes(buffer, args, 4);
	  if(argc > 2) {

	    itemcode=FindSpecialItemCodeFromString(args[0]);
	    if(itemcode==-1) {
	      notify(player, "That is not a valid special object!");
	      DumpMechSpecialObjects(player);
	      return;
	    }

	    index=ArmorSectionFromString(mech->type, args[1]);
	    if(index==-1) {
	      notify(player, "Not a legal area. Must be HEAD, CTORSO");
	      notify(player, "LTORSO, RTORSO, RLEG, LLEG, RARM, LARM");
	      notify(player, "TURRET, ROTOR, RSIDE, LSIDE, FRONT, BACK");
	      return;
	    }
	    
	    subsect=atoi(args[2]);
	    subsect--;
	    max=NUM_CRITICALS;
	    if(index==LLEG || index==RLEG || index==HEAD) {
	      max=6;
	    }
	    if(subsect<0|| subsect >= max) {
	      notify(player, "Subsection out of range!");
	      return;
	    }
	    
	    if(argc==4) {
	      newdata=atoi(args[3]);
	    } else {
	      newdata=0;
	    }

	    mech->sections[index].criticals[subsect].type=itemcode;
	    mech->sections[index].criticals[subsect].data=newdata;
	    switch(itemcode-SPECIAL_BASE_INDEX) 
	      {
	      case CASE:
		mech->sections[index].config |= CASE_TECH;
		notify(player, "CASE Technology added to section.");
		break;
	      case TRIPLE_STRENGTH_MYOMER:
		mech->specials |= TRIPLE_MYOMER_TECH;
		notify(player, "Triple Strength Myomer Technology added to 'Mech.");
		break;
	      case MASC:
		mech->specials |= MASC_TECH;
		notify(player, 
			"Myomer Accelerator Signal Circuitry added to 'Mech.");
		break;
	      case C3_MASTER:
		mech->specials |= C3_MASTER_TECH;
		notify(player, "C3 Command Unit added to 'Mech.");
                break;
              case C3_SLAVE:
                mech->specials |= C3_SLAVE_TECH;
                notify(player, "C3 Slave Unit added to 'Mech.");
                break;
              case ARTEMIS_IV:
                mech->sections[index].criticals[subsect].data--;
                mech->specials |= ARTEMIS_IV_TECH;
                notify(player, "Artemis IV Fire-Control System added to 'Mech.");
                notify(player, tprintf("System will control the weapon which starts at slot %d.",newdata));
                break;
              case ECM:
                mech->specials |= ECM_TECH;
                notify(player, "Guardian ECM Suite added to 'Mech.");
                break;
              case BEAGLE_PROBE:
                mech->specials |= BEAGLE_PROBE_TECH;
                notify(player, "Beagle Active Probe added to 'Mech.");
                break;
	      }

	    notify(player, "Critical slot filled.");
	  } else {
	    notify(player, "Invalid number of arguments!");
	  }
	}
      }  else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Rshowtech(player, data, buffer)
dbref player;
void *data;
char *buffer;
{
struct mechrep_data *rep;
struct mech_data *mech;
int i;
char location[20];

    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
	if(CheckData(player, mech)) {
	  notify(player,"--------Advanced Technology--------");
	  if(mech->specials & TRIPLE_MYOMER_TECH)
	    notify(player,"Triple Strength Myomer");
	  if(mech->specials & MASC_TECH)
	    notify(player,"Myomer Accelerator Signal Circuitry");
	  for(i=0;i<NUM_SECTIONS;i++)
	    if(mech->sections[i].config & CASE_TECH) {
	      ArmorStringFromIndex(i, location, mech->type);
	      notify(player,tprintf("Cellular Ammunition Storage Equipment in %s",location));
	    }
          if (mech->specials & CLAN_TECH)
          {
            notify(player,"Mech is set to Clan Tech.  This means:");
            notify(player,"    Mech automatically has Double Heat Sink Tech");
            notify(player,"    Mech automatically has CASE in all sections");
          }
          if (mech->specials & DOUBLE_HEAT_TECH)
            notify(player,"Mech uses Double Heat Sinks");
	  if(mech->specials & CL_ANTI_MISSILE_TECH)
	    notify(player,"Clan style Anti-Missile System");
	  if(mech->specials & IS_ANTI_MISSILE_TECH)
	    notify(player,"Inner Sphere style Anti-Missile System");
          if(mech->specials & FLIPABLE_ARMS)
            notify(player,"The arms may be flipped into the rear firing arc");
          if(mech->specials & C3_MASTER_TECH)
            notify(player,"C3 Command Computer");
          if(mech->specials & C3_SLAVE_TECH)
            notify(player,"C3 Slave Computer");
          if(mech->specials & ARTEMIS_IV_TECH)
            notify(player,"Artemis IV Fire-Control System");
          if(mech->specials & ECM_TECH)
            notify(player,"Guardian ECM Suite");
          if(mech->specials & BEAGLE_PROBE_TECH)
            notify(player,"Beagle Active Probe");
  	}
      }  else {
	notify(player, "You must set a target for repairs.");
      }
    }
}

void mechrep_Rdeltech(player, data, buffer)
dbref player;
void *data;
char *buffer;
{
struct mechrep_data *rep;
struct mech_data *mech;
int i, j;
char location[20];
unsigned char Type;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *)
	  FindObjectsData(rep->current_target); 
	if(CheckData(player, mech)) {
	  for(i=0;i<NUM_SECTIONS;i++)
	    if((mech->sections[i].config & CASE_TECH)
	       || (mech->specials & TRIPLE_MYOMER_TECH ) 
               || (mech->specials & MASC_TECH)) {
	      for(j=0;j<NUM_CRITICALS;j++) {
		Type=mech->sections[i].criticals[j].type;
	        if(Type==(CASE+SPECIAL_BASE_INDEX)
                   || Type==(TRIPLE_STRENGTH_MYOMER+SPECIAL_BASE_INDEX)
		   || Type==(MASC+SPECIAL_BASE_INDEX))
		  mech->sections[i].criticals[j].type=EMPTY;
	      }
	      mech->sections[i].config &= ~CASE_TECH;
	  }
	  mech->specials = 0;
	  notify(player,"Advanced Technology Deleted");
        }
      }  else {
	notify(player, "You must set a target for repairs.");
      }
    }
  }
}

void mechrep_Raddtech(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mechrep_data *rep;
  struct mech_data *mech;
  char *args[1];
  int index;

  if(power(db[player].owner,POW_TEMPLATE)) {
    rep=(struct mechrep_data *) data;
    if(CheckData(player, rep)) {
      if(rep->current_target!=-1) {
	mech=(struct mech_data *) FindObjectsData(rep->current_target);
	if(CheckData(player, mech)) {
	  if(1==mech_parseattributes(buffer, args, 1)) {
               if (string_prefix("Clan_Tech",args[0]))
               {
                   notify(player,"Mech set to be clan tech");
                   mech->specials |= CLAN_TECH;
               }
               else if (string_prefix("IS_Tech",args[0]))
               {
                   notify(player,"Mech set to be IS tech");
                   mech->specials &= ~CLAN_TECH;
               }
               else if (string_prefix("Double_HS",args[0]))
               {
                   notify(player,"Mech set to have Double Heat Sinks");
                   mech->specials |= DOUBLE_HEAT_TECH;
               }
               else if (string_prefix("Flipable_arms",args[0]))
               {
                   notify(player,"Mech set to have flipable arms.");
                   mech->specials |= FLIPABLE_ARMS;
               }
               else
               {
                   notify(player,
			    "Tech must be a prefix of one of the following:");
		   notify(player,"Clan_Tech");
		   notify(player,"IS_Tech");
		   notify(player,"Double_HS");
		   notify(player,"Flipable_Arms");
               }
	  } else {
	    notify(player, "You may only add one tech at a time!");
	  }
	}
      } else notify(player, "You must set a target first!");
    }
  }
}
