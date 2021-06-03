#ifdef TEST
typedef int dbref;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"*/
#include "../header.h"
#include "mech.h"
#include "weapons.h"

int mech_parseattributes(buffer,args,maxargs)
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

int CheckData(player,data)
dbref player; 
void *data; 
{
  int returnValue=0;

  if(data==NULL) {
    notify(player, "There is a problem with that item.");
    notify(player, "The data is not properly allocated.");
    notify(player, "Please notify a director of this.");
  } else {
    returnValue=1;
  }

  return (returnValue);
}

void FillDefaultCriticals(mech,index)
struct mech_data *mech; 
int index; 
{
  int loop;
  
  for(loop=0; loop<NUM_CRITICALS; loop++) {
    mech->sections[index].criticals[loop].type=EMPTY;
    mech->sections[index].criticals[loop].data=0;
    mech->sections[index].criticals[loop].mode = 0;
  }

  if(mech->type==CLASS_MECH)
    switch (index) {
    case HEAD:
      mech->sections[index].criticals[0].type=LIFE_SUPPORT+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[1].type=SENSORS+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[2].type=COCKPIT+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[4].type=SENSORS+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[5].type=LIFE_SUPPORT+SPECIAL_BASE_INDEX;
      break;
      
    case CTORSO:
      mech->sections[index].criticals[0].type=ENGINE+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[1].type=ENGINE+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[2].type=ENGINE+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[3].type=GYRO+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[4].type=GYRO+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[5].type=GYRO+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[6].type=GYRO+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[7].type=ENGINE+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[8].type=ENGINE+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[9].type=ENGINE+SPECIAL_BASE_INDEX;
      break;
      
    case RTORSO:
    case LTORSO:
      break;
      
    case LARM:
    case RARM:
    case LLEG:
    case RLEG:
      mech->sections[index].criticals[0].type=SHOULDER_OR_HIP+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[1].type=UPPER_ACTUATOR+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[2].type=LOWER_ACTUATOR+SPECIAL_BASE_INDEX;
      mech->sections[index].criticals[3].type=
	HAND_OR_FOOT_ACTUATOR+SPECIAL_BASE_INDEX;
      break;
    }
}
    
int ArmorSectionFromString(type, string)
char type;
char *string; 
{
  int index= -1;

  if(type == CLASS_MECH)
    switch (string[0]) {
    case 'H':
    case 'h':
      index=HEAD;
      break;
    case 'C':
    case 'c':
      index=CTORSO;
      break;
    case 'L':
    case 'l':
      switch (string[1]) {
      case 'T':
      case 't':
	index=LTORSO;
	break;
      case 'A':
      case 'a':
	index=LARM;
	break;
      case 'L':
      case 'l':
	index=LLEG;
	break;
      }
      break;
    case 'R':
    case 'r':
      switch (string[1]) {
      case 'T':
      case 't':
	index=RTORSO;
	break;
      case 'A':
      case 'a':
	index=RARM;
	break;
      case 'L':
      case 'l':
	index=RLEG;
	break;
      }
      break;
    }
  else if(type == CLASS_VEH_GROUND || type == CLASS_VEH_NAVAL)
    switch (string[0]) {
    case 'T':
    case 't':
      index=TURRET;
      break;
    case 'L':
    case 'l':
      index=LSIDE;
      break;
    case 'R':
    case 'r':
      index=RSIDE;
      break;
    case 'F':
    case 'f':
      index=FSIDE;
      break;
    case 'B':
    case 'b':
      index=BSIDE;
      break;
    }
  else if(type == CLASS_VEH_VTOL)
    switch (string[0]) {
    case 'R':
    case 'r':
      if(string[1] == 'O' || string[1] == 'o')
	index=ROTOR;
      else 
	index=RSIDE;
      break;
    case 'L':
    case 'l':
      index=LSIDE;
      break;
    case 'F':
    case 'f':
      index=FSIDE;
      break;
    case 'B':
    case 'b':
      index=BSIDE;
      break;
    }
  return (index);
}

int WeaponIndexFromString(string)
char *string; 
{
  int i;
  int returnValue= -1;

  for(i=0; i<NUM_DEF_WEAPONS; i++) {
    if(strcasecmp(MechWeapons[i].name, string)==0) {
      returnValue=i;
      break; /* out of for loop */
    }
  }
  return (returnValue);
}

void DumpWeapons(player)
dbref player; 
{
  int i;
  char buff[200];
  
  notify(player, 
	 tprintf("Name\t\tHeat\tDam\tMin\tShort\tMed\tLong\tCrits\tAmmo/ton"));
  for(i=0; i<NUM_DEF_WEAPONS; i++) {
    sprintf(buff, "%s\t\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
	    MechWeapons[i].name,
	    MechWeapons[i].heat,
	    MechWeapons[i].damage,
	    MechWeapons[i].min,
	    MechWeapons[i].shortrange,
	    MechWeapons[i].medrange,
	    MechWeapons[i].longrange,
	    MechWeapons[i].criticals,
	    MechWeapons[i].ammoperton);
    notify(player, buff);
  }
}

char ArmorStatus(mech,index,type)
struct mech_data *mech; 
int index;
int type; 
{
  char returnValue='D';
  int armorValue=0;

  switch(type) {
  case ARMOR:
    armorValue=mech->sections[index].armor;
    break;
  case INTERNAL:
    armorValue=mech->sections[index].internal;
    break;
  case REAR:
    armorValue=mech->sections[index].rear;
    break;
  }
  if(armorValue>30) {
    returnValue='$';
  } else if(armorValue>20) {
    returnValue='#';
  } else if(armorValue>9) {
    returnValue='@';
  } else {
    returnValue='0'+armorValue;
  }
  return (returnValue);
}

struct mech_special_struct {
  char *name;
  int complen;
  int key;
};

struct mech_special_struct MechSpecials[]={
  {"EMPTY", 5, EMPTY}, 
  {"SHOULDER_OR_HIP", 15, SHOULDER_OR_HIP+SPECIAL_BASE_INDEX}, 
  {"UPPER_ACTUATOR", 14, UPPER_ACTUATOR+SPECIAL_BASE_INDEX}, 
  {"LOWER_ACTUATOR", 14, LOWER_ACTUATOR+SPECIAL_BASE_INDEX}, 
  {"HAND_OR_FOOT_ACTUATOR", 21, HAND_OR_FOOT_ACTUATOR+SPECIAL_BASE_INDEX}, 
  {"LIFE_SUPPORT", 12, LIFE_SUPPORT+SPECIAL_BASE_INDEX}, 
  {"SENSORS", 7, SENSORS+SPECIAL_BASE_INDEX}, 
  {"COCKPIT", 7, COCKPIT+SPECIAL_BASE_INDEX}, 
  {"ENGINE", 6, ENGINE+SPECIAL_BASE_INDEX}, 
  {"GYRO", 4, GYRO+SPECIAL_BASE_INDEX}, 
  {"HEAT_SINK", 9, HEAT_SINK+SPECIAL_BASE_INDEX},
  {"JUMP_JET",  8, JUMP_JET+SPECIAL_BASE_INDEX},
  {"CASE",  4, CASE+SPECIAL_BASE_INDEX},
  {"FERRO_FIBROUS",  13, FERRO_FIBROUS+SPECIAL_BASE_INDEX},
  {"ENDO_STEEL",  10, ENDO_STEEL+SPECIAL_BASE_INDEX},
  {"TRIPLE_STRENGTH_MYOMER",  22, TRIPLE_STRENGTH_MYOMER+SPECIAL_BASE_INDEX},
  {"TARGETING_COMPUTER",  18, TARGETING_COMPUTER+SPECIAL_BASE_INDEX},
  {"MASC", 4, MASC+SPECIAL_BASE_INDEX},
  {"C3_MASTER",  9, C3_MASTER+SPECIAL_BASE_INDEX},
  {"C3_SLAVE",  8, C3_SLAVE+SPECIAL_BASE_INDEX},
  {"ARTEMIS_IV",  10, ARTEMIS_IV+SPECIAL_BASE_INDEX},
  {"ECM",  2, ECM+SPECIAL_BASE_INDEX},
  {"BEAGLE_PROBE",  12, BEAGLE_PROBE+SPECIAL_BASE_INDEX}
};

#define NUM_MECH_SPECIALS ((sizeof(MechSpecials))/                  \
			   sizeof(struct mech_special_struct))

void DumpMechSpecialObjects(player)
dbref player; 
{
  int loop;

  notify(player, "--- Special Objects ---");
  for(loop=0; loop<NUM_MECH_SPECIALS; loop++) {
    notify(player, MechSpecials[loop].name);
  } 
}

int FindSpecialItemCodeFromString(buffer)
char *buffer; 
{
  int returnValue= -1;
  int loop;

  for(loop=0; loop<NUM_MECH_SPECIALS; loop++) {
    if(strncasecmp(buffer, MechSpecials[loop].name,
	       MechSpecials[loop].complen)==0) {
      returnValue=MechSpecials[loop].key;
      break;
    }
  }

  return (returnValue);
}
