/*
  Debug.c
  
  File for debug of the hardcode routines.

  2.15.93- rdm created
*/
#define NULL 0
#define dbref int
#include "config.h"
/*#include "externs.h"*/
#include "../header.h"
#include "debug.h"

/* Externs */
extern void LoadSpecialObjects();
extern void SaveSpecialObjects();
extern void DumpSpecialObjectTable();
extern void DumpMechs();
extern void DumpVehicles();
extern void DumpMaps();

void debug_allocfree(key,data,selector)
dbref key;
void **data; 
int selector;
{
  send_mechinfo("ACCK! Debug/allocfree was called! (Shouldn't have been)");
  *data=NULL;
}

void debug_list(player,data,buffer)
dbref player; 
void *data; 
char *buffer;
{
  char *args[3];
  int argc;
  
  argc = mech_parseattributes(buffer, args, 3);
  if(argc == 0)
    DumpSpecialObjectTable(player);
  else if(args[0][0] == 'M' || args[0][0] == 'm') 
    {
      if(args[0][1] == 'E' || args[0][1] == 'e')
	{
	  DumpMechs(player);
	}
      if(args[0][1] == 'A' || args[0][1] == 'a')
	{
	  DumpMaps(player);
	}
    }
  else if(args[0][0] == 'V' || args[0][0] == 'v')
    DumpVehicles(player);
}

void debug_shutdown(player,data,buffer)
dbref player; 
void *data; 
char *buffer;
{
  char *args[3];
  int argc;
  
  argc = mech_parseattributes(buffer, args, 3);
  if(argc > 0)
    {
      ShutDownMap(player,atoi(args[0]));
    }
}

void debug_savedb(player,data,buffer)
dbref player; 
void *data;
char *buffer;
{
  notify(player, "--- Saving ---");
  SaveSpecialObjects();
  notify(player, "---  Done  ---");
}

void debug_loaddb(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  notify(player, "--- Loading ---");
  LoadSpecialObjects();
  notify(player, "---  Done   ---");
}
