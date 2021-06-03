#ifdef TEST
typedef int dbref;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*
#include "externs.h"
#include "db.h"
*/
#include "../header.h"
#include "mech.h"

void mech_ams(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  mech = (struct mech_data *) data;

  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
      if((mech->specials & IS_ANTI_MISSILE_TECH) || (mech->specials & CL_ANTI_MISSILE_TECH)) 
	{
	  if (mech->status & AMS_ENABLED)
	    {
	      mech_notify(mech,MECHALL,"Anti-Missile System turned OFF");
	      mech->status &= ~AMS_ENABLED;
	    }
	  else
	    {
	      mech_notify(mech,MECHALL,"Anti-Missile System turned ON");
	      mech->status |= AMS_ENABLED;
	    }
	}
      else
	notify(player,"This mech is not equipped with AMS");
    }
}

void mech_fliparms(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  mech = (struct mech_data *) data;

  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
        if (mech->specials & FLIPABLE_ARMS)
        {
            if (mech->status & FLIPPED_ARMS)
            {
                mech_notify(mech,MECHALL,
                		"Arms have been flipped to FORWARD position");
	        mech->status &= ~FLIPPED_ARMS;
            }
            else
            {
                mech_notify(mech,MECHALL,
                		"Arms have been flipped to REAR position");
	        mech->status |= FLIPPED_ARMS;
            }
        }
        else
            notify(player,"You cannot flip the arms in this mech");
    }
}

void mech_ultra(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[1];
struct mech_data *mech;
int index;
int section,critical,weaptype;

  mech = (struct mech_data *) data;
  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
        if (1==mech_parseattributes(buffer,args,1))
        {
            if (sscanf(args[0],"%d",&index) == 1)
            {
                if (index < 0)
                {
                    notify(player, 
                        "The weapons system chirps: 'Illegal Weapon Number!'");
		    return;
	        }

	        weaptype=FindWeaponNumberOnMech(mech,index,&section,&critical);
	        if(weaptype== -1) {
	  	  notify(player, 
       "The weapons system chirps: 'Illegal Weapon Number!'");
	  	  return;
	        } else if (weaptype== -2) {
		  notify(player,
       "The weapons system chirps: 'That Weapon has been destroyed!'");
		  return;
	        }

	        weaptype = mech->sections[section].criticals[critical].type;
                weaptype -= WEAPON_BASE_INDEX;
	        if (MechWeapons[weaptype].special == ULTRA)
	        {
	if (mech->sections[section].criticals[critical].mode & ULTRA_MODE)
	{
	    mech->sections[section].criticals[critical].mode &= ~ULTRA_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to normal fire mode",index));
	}
	else
	{
	    mech->sections[section].criticals[critical].mode |= ULTRA_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to ultra fire mode",index));
	}
	        }
	        else
	        {
	            notify(player,"That weapon cannot be set ULTRA!");
	        }
            }
            else
                notify(player,"That isn't a legal weapon");
        }
        else
            notify(player,"Please specify a weapon number.");
    }
}

void mech_lbx(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[1];
struct mech_data *mech;
int index;
int section,critical,weaptype;

  mech = (struct mech_data *) data;
  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
        if (1==mech_parseattributes(buffer,args,1))
        {
            if (sscanf(args[0],"%d",&index) == 1)
            {
                if (index < 0)
                {
                    notify(player, 
                        "The weapons system chirps: 'Illegal Weapon Number!'");
		    return;
	        }

	        weaptype=FindWeaponNumberOnMech(mech,index,&section,&critical);
	        if(weaptype== -1) {
	  	  notify(player, 
       "The weapons system chirps: 'Illegal Weapon Number!'");
	  	  return;
	        } else if (weaptype== -2) {
		  notify(player,
       "The weapons system chirps: 'That Weapon has been destroyed!'");
		  return;
	        }

	        weaptype = mech->sections[section].criticals[critical].type;
                weaptype -= WEAPON_BASE_INDEX;
	        if (MechWeapons[weaptype].special == LBX)
	        {
	if (mech->sections[section].criticals[critical].mode & LBX_MODE)
	{
	    mech->sections[section].criticals[critical].mode &= ~LBX_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to normal fire mode",index));
	}
	else
	{
	    mech->sections[section].criticals[critical].mode |= LBX_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to LBX fire mode",index));
	}
	        }
	        else
	        {
	            notify(player,"That weapon cannot be set LBX!");
	        }
            }
            else
                notify(player,"That isn't a legal weapon");
        }
        else
            notify(player,"Please specify a weapon number.");
    }
}

void mech_artemis(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[1];
struct mech_data *mech;
int index;
int section,critical,weaptype;

  mech = (struct mech_data *) data;
  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
        if (1==mech_parseattributes(buffer,args,1))
        {
            if (sscanf(args[0],"%d",&index) == 1)
            {
                if (index < 0)
                {
                    notify(player, 
                        "The weapons system chirps: 'Illegal Weapon Number!'");
		    return;
	        }

	        weaptype=FindWeaponNumberOnMech(mech,index,&section,&critical);
	        if(weaptype== -1) {
	  	  notify(player, 
       "The weapons system chirps: 'Illegal Weapon Number!'");
	  	  return;
	        } else if (weaptype== -2) {
		  notify(player,
       "The weapons system chirps: 'That Weapon has been destroyed!'");
		  return;
	        }
                if(!FindArtemisForWeapon(mech,section,critical))
                  {
                    mech_notify(mech, MECHALL, "You have no Artemis system for that weapon.");
                    return;
                  }
                
	        weaptype = mech->sections[section].criticals[critical].type;
                weaptype -= WEAPON_BASE_INDEX;
	        if (MechWeapons[weaptype].type == TMISSILE)
	        {
	if (mech->sections[section].criticals[critical].mode & ARTEMIS_MODE)
	{
	    mech->sections[section].criticals[critical].mode &= ~ARTEMIS_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to fire normal missiles",index));
	}
	else
	{
	    mech->sections[section].criticals[critical].mode |= ARTEMIS_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to fire Artemis IV compatible missiles.",index));
	}
	        }
	        else
	        {
	            notify(player,"That weapon cannot be set ARTEMIS!");
	        }
            }
            else
                notify(player,"That isn't a legal weapon");
        }
        else
            notify(player,"Please specify a weapon number.");
    }
}

void mech_narc(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
char *args[1];
struct mech_data *mech;
int index;
int section,critical,weaptype;

  mech = (struct mech_data *) data;
  if (CheckData(player,mech))
    {
      if(mech->unconcious)
	{
	  notify(player, "You are unconcious...zzzzzz");
	  return;
	}
        if (1==mech_parseattributes(buffer,args,1))
        {
            if (sscanf(args[0],"%d",&index) == 1)
            {
                if (index < 0)
                {
                    notify(player, 
                        "The weapons system chirps: 'Illegal Weapon Number!'");
		    return;
	        }

	        weaptype=FindWeaponNumberOnMech(mech,index,&section,&critical);
	        if(weaptype== -1) {
	  	  notify(player, 
       "The weapons system chirps: 'Illegal Weapon Number!'");
	  	  return;
	        } else if (weaptype== -2) {
		  notify(player,
       "The weapons system chirps: 'That Weapon has been destroyed!'");
		  return;
	        }

	        weaptype = mech->sections[section].criticals[critical].type;
                weaptype -= WEAPON_BASE_INDEX;
	        if (MechWeapons[weaptype].type == TMISSILE)
	        {
	if (mech->sections[section].criticals[critical].mode & NARC_MODE)
	{
	    mech->sections[section].criticals[critical].mode &= ~NARC_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to fire normal missiles",index));
	}
	else
	{
	    mech->sections[section].criticals[critical].mode |= NARC_MODE;
	    mech_notify(mech,MECHALL,
	    	tprintf("Weapon %d has been set to fire NARC Beacon compatible missiles.",index));
	}
	        }
	        else
	        {
	            notify(player,"That weapon cannot be set NARC!");
	        }
            }
            else
                notify(player,"That isn't a legal weapon");
        }
        else
            notify(player,"Please specify a weapon number.");
    }
}



