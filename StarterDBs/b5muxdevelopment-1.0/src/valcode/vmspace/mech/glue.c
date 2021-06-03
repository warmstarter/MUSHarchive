/*
  Glue.c
  This is the code that will allow for certain objects to have special
  hardcoded commands set on them.


  2.5.93- rdm created (rdm4@acpub.duke.edu)
*/

#include <stdio.h>
#include <sys/file.h>


#ifdef TEST
typedef int DBREf;
typdef char ATTR;
char *atr_get();
void notify();
ATTR *atr_str();
#else
#include "config.h"
#include "externs.h"
#include "db.h"
#endif

#include <stdio.h>

/*** #include all the prototype here! ****/
#include "mech.h"
#include "debug.h"
#include "mechrep.h"
/*
#include "map.h"
*/
#include "comp.h"

#include "glue.h"

/* Prototypes */
/*************CALLABLE PROTOS*****************/
/* Called on database save/load */
void LoadSpecialObjects();
void SaveSpecialObjects();
void UpdateSpecialObjects(); /* Called once a second */

/* Main entry point */
int HandledCommand();

/* called when user creates/removes hardcode flag */
void CreateNewSpecialObject();
void DisposeSpecialObject();

/* Debug routines */
void DumpSpecialObjectTable();

/*************PERSONAL PROTOS*****************/
void *FindObjectsData();
static struct SpecialObjectInstance *GetSpecialObjectInstance();
static void DoSpecialObjectHelp();
static int SaveSpecialObjectsToFile();
static int LoadSpecialObjectsFromSaveFile();
static struct SpecialObjectInstance *NewSpecialObject();
static void AddSpecialObjectToTable();
static int SetObjectsData();
static struct SpecialObjectInstance *RemoveSpecialObjectFromTable();
static int WhichSpecial();
static void AllocNewSpecialObjectsOnLoad();
/*********************************************/

void SortSpecialCommands();
int numspecialcommands[NUM_SPECIAL_OBJECTS];
struct CommandsStruct **SortedCommands[NUM_SPECIAL_OBJECTS];


/* Main entry point */
int HandledCommand(player,location,command)
dbref player; 
dbref location; 
char *command; 
{
struct SpecialObjectStruct *typeOfObject;
struct SpecialObjectInstance *SOI;
int type;
int dir;
int first, last, current;

  SOI = GetSpecialObjectInstance(location);
  if(!SOI) 
    return 0;

  type = SOI->type;
  typeOfObject = &SpecialObjects[type];
  
  if(strncmp(command, "HELP", 4)==0) {
    DoSpecialObjectHelp(player, typeOfObject->type, typeOfObject->commands,
					typeOfObject->power_needed);
    return 1;
  } else {
    first = 0;
    last = numspecialcommands[type] - 1;
    dir = 1;
    while (dir && (first <= last))
    {
	current = (first+last)/2;
	dir = (int)strncasecmp(command,SortedCommands[type][current]->name,
				SortedCommands[type][current]->len);
	if (dir < 0)
	    last = current-1;
	else
	    first = current+1;
    }

    if (!dir)
    {
	if (SortedCommands[type][current]->helpmsg[0] != '@' ||
		    power(db[player].owner,typeOfObject->power_needed))
	    SortedCommands[type][current]->func(player, SOI->data, 
			command+SortedCommands[type][current]->len);
	else
	    notify(player,"Sorry, that command is restricted!");
	return 1;
    }
    else
	return 0;
  }
}

void LoadSpecialObjects() {
dbref i;
struct SpecialObjectInstance *temp;
int type;

  for (i=0;i<NUM_SOI_ENTRIES;i++)
      SOI_Table[i] = NULL;

  /* Loop through the entire database, and if it has the special */
  /* object flag, add it to our linked list. */
  for(i=0; i<db_top; i++) {
    if(db[i].flags & HARDCODE_FLAG) {
      type = WhichSpecial(i);
      if (type > -1)
      {
	  temp=NewSpecialObject(i,type);
	  if(temp) {
	    AddSpecialObjectToTable(temp);
	  } else {
	    log_important("Error: Allocating Special Object space");
	  }
      }
      else
	  db[i].flags &= ~HARDCODE_FLAG; /* Reset the flag */
    }
  }

  /* Then, for each type of special object, open the file associated */
  /* with it and read in all the objects- search the linked list for */
  /* each object found */

  for(i=0; i<NUM_SPECIAL_OBJECTS; i++) {
    if(!LoadSpecialObjectsFromSaveFile(i)) {
      log_important(tprintf("Error loading %s file!",SpecialObjects[i].type));
    }
    SortSpecialCommands(i);
  }

  /* If there are left over objects, then assume they are new, and */
  /* allocate space for them by calling the new object function */
  AllocNewSpecialObjectsOnLoad(); /* scans the list */
}

void SaveSpecialObjects() {
int i;

  /* Traverse the SpecialObject types... */
  for(i=0; i<NUM_SPECIAL_OBJECTS; i++) {
    if(!SaveSpecialObjectsToFile(i)) {
      log_important("Error saving the special objects!");
    }
  }
}

/* This is called once a second for each special object */
void UpdateSpecialObjects() {
int i;
struct SpecialObjectInstance *temp;
int update[NUM_SPECIAL_OBJECTS];

  for(i=0; i<NUM_SPECIAL_OBJECTS; i++) {
    update[i] = 0;
    if(SpecialObjects[i].updateTime) {
      if (++SpecialObjects[i].timeSinceLastUpdate>=SpecialObjects[i].updateTime)
      {
	if(SpecialObjects[i].updatefunc) 
          update[i] = 1;
	SpecialObjects[i].timeSinceLastUpdate=0;
      }
    }
  }

  for (i=0; i<NUM_SOI_ENTRIES; i++)
  {
    temp = SOI_Table[i];
    while (temp)
    {
	if (update[temp->type])
	    SpecialObjects[temp->type].updatefunc(temp->key, temp->data);
	temp = temp->next;
    }
  }
}

void CreateNewSpecialObject(player,key)
dbref player; 
dbref key; 
{
struct SpecialObjectInstance *new;
struct SpecialObjectStruct *typeOfObject;
ATTR *typeAttr;
int type;

  typeAttr=atr_str(GOD, key, "XTYPE");
  if(!typeAttr) {
    notify(player, "You must @defattr here/XTYPE first, with the type");
    notify(player, "of object defined first");
    notify(player, "Resetting hardcode flag.");
    db[key].flags &= ~HARDCODE_FLAG; /* Reset the flag */
    return;
  }
  
  /* Find the special objects */
  type = WhichSpecial(key);
  if(type > -1) {
    /* We found the proper special object */
    typeOfObject = &SpecialObjects[type];
    new=NewSpecialObject(key,type);
    if(new) {
      AddSpecialObjectToTable(new);
      if(typeOfObject->datasize) {
	typeOfObject->allocfreefunc(key, &(new->data), SPECIAL_ALLOC);
      }
    } else {
      notify(player, "Memory allocation failure!");
    }
  } else {
    notify(player, "That is not a valid XTYPE!");
    notify(player, "Resetting HARDCODE flag.");
    db[key].flags &= ~HARDCODE_FLAG;
  } 
}

void DisposeSpecialObject(player,key)
dbref player; 
dbref key;
{
ATTR *typeAttr;
struct SpecialObjectInstance *dead;
struct SpecialObjectStruct *typeOfObject;
    
  dead=RemoveSpecialObjectFromTable(key);
  
  if(dead==NULL) 
  {
    notify(player, "This object is not in the special object DBASE.");
    notify(player, "Please contact a director about this bug. ");
    return;
  }

  typeOfObject = &SpecialObjects[dead->type];
  if(typeOfObject->datasize) 
      if(dead->data) 
          typeOfObject->allocfreefunc(key, &(dead->data), SPECIAL_FREE);

  /* Now get rid of the linked list bit... */
  free(dead);
}

void DumpSpecialObjectTable(player)
dbref player;
{
struct SpecialObjectInstance *temp;
char buffer[100];
int loop,i;

  notify(player, "Special Objects:");
  for(loop=0; loop<NUM_SPECIAL_OBJECTS; loop++) {
    sprintf(buffer, "Type: %s", SpecialObjects[loop].type);
    notify(player, buffer);
    for (i=0; i<NUM_SOI_ENTRIES; i++)
    {
	temp = SOI_Table[i];
	while(temp) {
	  if (temp->type == loop)
	  {
	      sprintf(buffer, "Key: %d Memory: %x", temp->key, 
		      (unsigned int) temp->data);
	      notify(player, buffer);
	  }
	  temp=temp->next;
	}
    }
  }
  notify(player, "Done listing");
}

void DumpVehicles(player)
     dbref player;
{
  struct mech_data *mech;
  char buff[100];
  int loop, i, running=0, count=0;
  struct SpecialObjectInstance *temp;
  
  notify(player, "VEHICLE # STATUS      MAP #      PILOT #");
  notify(player, "----------------------------------------");
  for(loop=0; loop<NUM_SPECIAL_OBJECTS; loop++)
    if(!strcmp(SpecialObjects[loop].type,"VEHICLE"))
      for (i=0; i<NUM_SOI_ENTRIES; i++)
	{
	  temp = SOI_Table[i];
	  while(temp) 
	    {
	      if (temp->type == loop)
		{
		  mech = (struct mech_data *) temp->data;
		  if(mech->status & STARTED)
		    {
		      sprintf(buff, "#%5d    RUNNING     #%5d    #%5d",
			      mech->mynum, mech->mapindex, mech->pilot);
		      notify(player, buff);
		      running++;
		    }
		  count++;
		}
	      temp=temp->next;
	    }
	}
  sprintf(buff, "%d vehicles running out of %d vehicles allocated.", running, count);
  notify(player, buff);
  notify(player, "Done listing");
}

void DumpMechs(player)
     dbref player;
{
  struct mech_data *mech;
  char buff[100];
  int loop, i, running=0, count=0;
  struct SpecialObjectInstance *temp;
  
  notify(player, "MECH #    STATUS      MAP #      PILOT #");
  notify(player, "----------------------------------------");
  for(loop=0; loop<NUM_SPECIAL_OBJECTS; loop++)
    if(!strcmp(SpecialObjects[loop].type,"MECH"))
      for (i=0; i<NUM_SOI_ENTRIES; i++)
	{
	  temp = SOI_Table[i];
	  while(temp) 
	    {
	      if (temp->type == loop)
		{
		  mech = (struct mech_data *) temp->data;
		  if(mech->status & STARTED)
		    {
		      sprintf(buff, "#%5d    RUNNING     #%5d    #%5d",
			      mech->mynum, mech->mapindex, mech->pilot);
		      notify(player, buff);
		      running++;
		    }
		  count++;
		}
	      temp=temp->next;
	    }
	}
  sprintf(buff, "%d mechs running out of %d mechs allocated.", running, count);
  notify(player, buff);
  notify(player, "Done listing");
}

void DumpMaps(player)
     dbref player;
{
  struct map_struct *map;
  char buff[100];
  int loop, i, j, count;
  struct SpecialObjectInstance *temp;
  
  notify(player, "MAP #       NAME              X x Y   CITY LINK   MECHS");
  notify(player, "-------------------------------------------------------");
  for(loop=0; loop<NUM_SPECIAL_OBJECTS; loop++)
    if(!strcmp(SpecialObjects[loop].type,"MAP"))
      for (i=0; i<NUM_SOI_ENTRIES; i++)
	{
	  temp = SOI_Table[i];
	  while(temp) 
	    {
	      if (temp->type == loop)
		{
		  count = 0;
		  map = (struct map_struct *) temp->data;
		  for(j=0;j<MAX_MECHS_PER_MAP;j++)
		    if(map->mechsOnMap[j] != -1) count++;
		  sprintf(buff, "#%5d    %-17.17s %3d x%3d    #%5d     %d",
			  map->mynum, map->mapname, map->map_width, 
			  map->map_height, map->city, count);
		  notify(player, buff);
		}
	      temp=temp->next;
	    }
	}
  notify(player, "Done listing");
}

void ShutDownMap(player,mapnumber)
     dbref player;
     dbref mapnumber;
{
  struct map_struct *map;
  struct mech_data *mech;
  char buff[100];
  int loop, i, j, count=0;
  struct SpecialObjectInstance *temp;
  
  for(loop=0; loop<NUM_SPECIAL_OBJECTS; loop++)
    if(!strcmp(SpecialObjects[loop].type,"MAP"))
      for (i=0; i<NUM_SOI_ENTRIES; i++)
	{
	  temp = SOI_Table[i];
	  while(temp) 
	    {
	      if (temp->type == loop && temp->key == mapnumber)
		{
		  map = (struct map_struct *) temp->data;
		  for(j=0;j<MAX_MECHS_PER_MAP;j++)
		    if(map->mechsOnMap[j] != -1) 
		      {
			mech = (struct mech_data *)FindObjectsData(map->mechsOnMap[j]);
			if(mech)
			  {
			    notify(player, tprintf("Shutting down Mech #%d and restting map index to -1....",map->mechsOnMap[j]));
			    mech_shutdown(GOD, (void *)mech, "");
			    mech->mapindex = -1;
			    mech->last_x = 0;
			    mech->last_y = 0;
			    mech->x = 0;
			    mech->y = 0;
			  }
			map->mechsOnMap[j] = -1;
		      }
		  notify(player, "Map Cleared");
		  return;
		}
	      temp=temp->next;
	    }
	}
}

/***************** INTERNAL ROUTINES *************/
static int SaveSpecialObjectsToFile(which)
int which;
{
struct SpecialObjectStruct *typeOfObject;
int returnValue=0;
struct SpecialObjectInstance *temp;
int fp;
char buffer[50];
dbref tempkey;
int i;

  typeOfObject = &SpecialObjects[which];
  /* Copy old base */
  sprintf(buffer, "mv db/hcode/%s db/hcode/%s~", 
			typeOfObject->type, typeOfObject->type);
  system(buffer);

  /* Create new one */
  sprintf(buffer, "db/hcode/");
  strcat(buffer, typeOfObject->type);
  fp=open(buffer, (O_WRONLY | O_CREAT), 0644);
  if(fp != -1) 
  {
    for (i=0; i<NUM_SOI_ENTRIES; i++)
    {
	temp = SOI_Table[i];
	while(temp) 
	{
	    /* Save it to the file... */
	    if(temp->type == which && temp->data) 
	    {
	        tempkey=temp->key;
	        write(fp, (char *) &tempkey, sizeof(dbref));
	        write(fp, temp->data, typeOfObject->datasize);
            }
            temp=temp->next;
        }
    }
    returnValue=1;
    close(fp);
  } else {
	notify(41, "Unable to create/open file");
  }
  return (returnValue);
}

static int WhichSpecial(key)
dbref key; 
{
int i;
int returnValue = -1;
ATTR *typeAttr;

  typeAttr=atr_str(GOD, key, "XTYPE");
  if(typeAttr) {
    for(i=0; i<NUM_SPECIAL_OBJECTS; i++) {
      if(!strcmp(SpecialObjects[i].type, atr_get(key, typeAttr))) 
      {
	returnValue= i;
	break;
      }
    }
  }
  return (returnValue);
}


static void AllocNewSpecialObjectsOnLoad() 
{
struct SpecialObjectInstance *temp;
int i;

  for (i=0; i<NUM_SOI_ENTRIES; i++)
  {
      temp = SOI_Table[i];
      while (temp)
      {
	  if (!temp->data)
	  {
	      if (SpecialObjects[temp->type].datasize)
	      {
		  SpecialObjects[temp->type].allocfreefunc
				(temp->key, &(temp->data), SPECIAL_ALLOC);
	      }
	 }
         temp=temp->next;
     }
  }
}

static int LoadSpecialObjectsFromSaveFile(which)
int which; 
{
int returnValue=0;
int fp, i;
char *buffer;
dbref key;
int done=0;
char filepath[50],mapbuffer[50];
struct map_struct *map;
struct mech_data *mech;
struct SpecialObjectStruct *typeOfObject;

  typeOfObject = &SpecialObjects[which];
  sprintf(filepath, "db/hcode/");
  strcat(filepath, typeOfObject->type);
  fp=open(filepath, O_RDONLY, 0644);
  if (fp!=-1) {
    while(!done) {
      /* Allocate buffer for each element-> We only free the extra */
      /* allocated one. */
      buffer=(char *) malloc(typeOfObject->datasize);
      if(buffer) {
	if(sizeof(dbref)==read(fp, (char *) &key, sizeof(dbref))) 
	{
	  if(typeOfObject->datasize==read(fp, buffer, typeOfObject->datasize)) 
	  {
	    if(!SetObjectsData(key, which, (void *) buffer)) 
	    {
	      log_important("Error setting objects data- item not found");
	      free(buffer);
	    } 
	    else 
	    {
		/* Load the map if it is a map (dynamically allocated) */
		if (strcmp(typeOfObject->type,"MAP") == 0)
		{
		    map = (struct map_struct *) buffer;
		    sprintf(mapbuffer," %s",map->mapname);
		    /* Now free the old map */
		    map->map=NULL;
		    if (strcmp(map->mapname,"Default Map") == 0)
		      {
			map->map = 0x0;
			newfreemap(key,(void **) &map,SPECIAL_FREE);
			newfreemap(key,(void **) &map,SPECIAL_ALLOC);
		      }
		    else
		      {
			map_loadmap(1,map,mapbuffer);
		      }
		}
		else if (!strcmp(typeOfObject->type,"MECH") ||
			 !strcmp(typeOfObject->type,"VEHICLE"))
		{
		    mech = (struct mech_data *) buffer;
		    mech->mapindex = -1;
		    mech->status &= ~STARTED;
		    mech->pilot = -1;
		    mech->clock = 0;
		    mech->speed = 0;
		    mech->desired_speed = 0;
		    mech->goingx = -1;
		}
	     }
	  } else {
	    /* We reached the end of the line-> free the last one we */
	    /* set created (we didn't attach it to anything) */
	    returnValue=1;
	    done=1;
	    free(buffer);
	  }
	} else {
	  /* We reached the end of the line-> free the last one we */
	  /* set created (we didn't attach it to anything) */
	  returnValue=1;
	  done=1;
	  free(buffer);
	}   
      } else {
	log_important("Out of Mem loading the Special Objects");
      }
    }
    close(fp);
  } else {
    log_important("Special Object file not found!");
  }

  return (returnValue);
}

static struct SpecialObjectInstance *NewSpecialObject(key,type)
dbref key;
int type;
{
  struct SpecialObjectInstance *returnValue;

  returnValue=(struct SpecialObjectInstance *) 
    malloc(sizeof(struct SpecialObjectInstance));
  if(returnValue) {
    returnValue->key=key;
    returnValue->type=type;
    returnValue->data=NULL;
    returnValue->next=NULL;
  }
  return (returnValue);
}

static struct SpecialObjectInstance *RemoveSpecialObjectFromTable(key)
dbref key; 
{
struct SpecialObjectInstance *temp;
struct SpecialObjectInstance *prev=NULL;
struct SpecialObjectStruct *type;

    temp = SOI_Table[key % NUM_SOI_ENTRIES];
    while (temp && temp->key != key)
    {
	prev = temp;
	temp = temp->next;
    }

    if(temp) /* If we found the match */
    { 
        if(prev) 
	    prev->next=temp->next;
        else
	    SOI_Table[key % NUM_SOI_ENTRIES] = temp->next;
    }
    return (temp);
}
      

static void AddSpecialObjectToTable(newObj)
struct SpecialObjectInstance *newObj; 
{ 
  newObj->next = SOI_Table[newObj->key % NUM_SOI_ENTRIES];
  SOI_Table[newObj->key % NUM_SOI_ENTRIES] = newObj;
} 
  
static int SetObjectsData(key, which, data)
dbref key; 
int which;
void *data; 
{
struct SpecialObjectInstance *temp;
ATTR *typeAttr;
ATRDEF *k;

    typeAttr=atr_str(GOD, key, "XTYPE");
    if(!typeAttr) {
        log_important(tprintf("Type: %d - Error in object- Not XType - #%d ",
					which,key));
	return 0;
    }

    temp = GetSpecialObjectInstance(key);
    if (temp)
    {
	temp->data = data;
	atr_add(key,typeAttr,SpecialObjects[which].type);
	return 1;
    }
    return 0;
}

/*** Support routines ***/
void *FindObjectsData(key)
dbref key; 
{
struct SpecialObjectInstance *temp;

    temp = GetSpecialObjectInstance(key);
    if (temp)
	return temp->data;
    else
	return NULL;
}

static void DoSpecialObjectHelp(player,type,commands,powerneeded)
dbref player; 
char *type; 
CommandsStruct *commands;
int powerneeded;
{
int i=0;
int alo = 0;

  notify(player, tprintf("%s Command Summary:", type));
  while(commands[i].name)
  {
    if (commands[i].helpmsg[0] == '@')
    {
        if (power(db[player].owner,powerneeded))
        {
            alo = 1;
            notify(player,tprintf("%s - %s (R)",
                                commands[i].name, commands[i].helpmsg+1));
        }
    }
    else
    {
        alo = 1;
        notify(player,tprintf("%s - %s",
                                commands[i].name, commands[i].helpmsg));
    }
    i++;
  }
  if (!alo)
      notify(player,
         "Sorry, there are no commands here that you are authorized to use");
}

static struct SpecialObjectInstance *GetSpecialObjectInstance(loc)
dbref loc;
{
struct SpecialObjectInstance *temp;

    if (loc < 0 )
	return NULL;

    temp = SOI_Table[loc % NUM_SOI_ENTRIES];
    while (temp && temp->key != loc)
	temp = temp->next;
    return temp;
}

void SortSpecialCommands(which)
int which;
{
int nc;
int i;
int done;
struct CommandsStruct *temp;

    for (nc=0; SpecialObjects[which].commands[nc].name; nc++)
	;
    numspecialcommands[which] = nc;

    SortedCommands[which] = (struct CommandsStruct **)
		malloc(sizeof(struct CommandsStruct *) * nc);
    
    for (i=0; i<nc; i++)
	SortedCommands[which][i] = &(SpecialObjects[which].commands[i]);

    done = 0;
    while (!done)
    {
	done = 1;
	for (i=0; i<nc-1; i++)
	{
	    if (string_compare(SortedCommands[which][i]->name,
				SortedCommands[which][i+1]->name) > 0)
	    {
		temp = SortedCommands[which][i];
		SortedCommands[which][i] = SortedCommands[which][i+1];
		SortedCommands[which][i+1] = temp;
		done = 0;
	    }
	}
    }
}

/***** TESTING ROUTINES *****/
#ifdef TEST
char *atr_get(a,key)
dbref a; 
char *key; 
{
  return ("MECH");
}

char *atr_get(a,key)
dbref a; 
ATTR *key;
{
  return ("MECH");
}

ATTR *atr_str(a,b,c)
dbref a; 
dbref b; 
char *c; 
{
  return ((ATTR *) 1); /* as long as not zero, we don't fall through */
}

void notify(player,msg)
dbref player; 
char *msg; 
{
  printf("Notifying %d: %s\n", player, msg);
}

void main() {
  char buffer[1000];

  printf("Command >");
  while(1) {
    fgets(buffer, 1000, stdin);
    buffer[strlen(buffer)-1]=0; /* truncate the \n */

    printf("HandledCommand: %d\n", HandledCommand(41, 12, buffer));
    printf("Command >");
  }
}
#endif

