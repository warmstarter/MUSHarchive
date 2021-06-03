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

char *default_contactoptions = "!a-z";

void mech_brief(player,data,buffer)
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
	  notify(player,"My what a nice nap...zzzzzz");
	  return;
	}
      if (mech->brief)
        {
          notify(player,"Brief output turned OFF");
          mech->brief = 0;
        }
      else
        {
          notify(player,"Brief output turned ON");
          mech->brief = 1;
        }
    }
}

void mech_contacts(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
  struct mech_data *mech, *tempMech;
  struct map_struct *mech_map;
  int loop, i, j, argc, bearing, buffindex=0;
  char contactID[2], *args[1], bufflist[MAX_MECHS_PER_MAP][100], buff[100];
  float range, rangelist[MAX_MECHS_PER_MAP];
  int mechfound;
  char weaponarc;
  char mech_name[100];
  int see_dead;
  int see_shutdown;
  char can_see[128];
  ATTR *typeAttr;
  char move_type[30];
  int arc;

  mech=(struct mech_data *) data;
  mechfound=0;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("contacts:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  argc=mech_parseattributes(buffer, args, 1);
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
	{
	  notify(player,"You are unconcious...zzzzzz");
	  return;
	}
      if(!(mech->status & STARTED)) 
	{
	  notify(player,"The 'Mech is not started yet.");
	  return;
	}
      for (loop = 0; loop < 128; loop++)
	{
	    can_see[loop] = 0;
	}
      see_dead = 1;
      see_shutdown = 1;

      if (argc == 1)
      {
	  if (args[0][0] == '+')
	  {
	      /*typeAttr = atr_str(GOD, player, "CONTACTOPTIONS");*/
	      if (!typeAttr)
	      {
		  strcpy(buff,default_contactoptions);
	      }
	      else
	      {
		  /*strncpy(buff,atr_get(player,typeAttr),50);*/
		  buff[49] = 0;
		  if (strlen(buff) == 0)
		  {
		      strcpy(buff,default_contactoptions);
		  }
	      }
	  }
	  else
	  {
	      strncpy(buff,args[0],50);
	      buff[49] = 0;
	  }

	  for (loop=0; loop<50 && buff[loop]; loop++)
	  {
	  char c;

	      c = buff[loop];
	      if (c == '!')
		  see_dead = 0;
	      else if (c == '@')
		  see_shutdown = 0;
	      else if (c == '-')
	      {
	      char upper;
	      char lower;
		  if (loop > 0 && buff[loop-1] != '!' && buff[loop-1] != '@')
		      lower = toupper(buff[loop-1]);
		  else 
		      lower = 'A';
		  loop++;
		  if (buff[loop] && buff[loop] != '!' && buff[loop] != '@')
		      upper = toupper(buff[loop]);
		  else
		      upper = 'Z';
		  if (lower <= upper)
		      for (i=lower; i<=upper; i++)
			  can_see[i] = 1;
		  else
		      for (i=upper; i<=lower; i++)
			  can_see[i] = 1;
	      }
	      else 
		  can_see[toupper(c)] = 1;
	  }
      }

      notify(player, "Line of Sight Contacts:");
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++) 
        {
          if(mech_map->mechsOnMap[loop] != mech->mynum 
             && mech_map->mechsOnMap[loop] != -1) 
            {
              tempMech=(struct mech_data *) 
                FindObjectsData(mech_map->mechsOnMap[loop]);
	      if(tempMech && (argc != 1 || 
			   (can_see[tempMech->ID[0]] &&
			   (see_dead || !(tempMech->status & DESTROYED)) &&
			   (see_shutdown || (tempMech->status & STARTED)))))
		{
		  range=FindRange(mech->fx,mech->fy,mech->fz,
				  tempMech->fx,tempMech->fy,tempMech->fz);
		  if(InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
		    {
		      get_mech_atr(tempMech, mech_name, "MECHNAME");

		      if(tempMech->team == mech->team) 
			{
			  contactID[0]=tempMech->ID[0] + 32;
			  contactID[1]=tempMech->ID[1] + 32;
			} 
		      else 
			{
			  contactID[0]=tempMech->ID[0];
			  contactID[1]=tempMech->ID[1];
			}
		      bearing=FindBearing(mech->fx,mech->fy,tempMech->fx,tempMech->fy);
		      arc = InWeaponArc(mech,tempMech->fx,tempMech->fy);
		      if(mech->type==CLASS_MECH) 
			{
			  switch(arc)
			    {
			    case FORWARDARC:
				weaponarc = '*';
				break;
			    case LARMARC:
				weaponarc = 'l';
				break;
			    case RARMARC:
				weaponarc = 'r';
				break;
			    case REARARC:
				weaponarc = 'v';
				break;
			    default:
				weaponarc = ' ';
			      break;
			    }
			}
		      else
			{
			  if(arc >= TURRETARC)
			    {
			      weaponarc = 't';
			      arc -= TURRETARC;
			    }
			  else
			    {
			      switch(arc)
				{
				case FORWARDARC:
				  weaponarc = '*';
				  break;
				case RSIDEARC:
				  weaponarc = 'r';
				  break;
				case LSIDEARC:
				  weaponarc = 'l';
				  break;
				case REARARC:
				  weaponarc = 'v';
				  break;
				default:
				  weaponarc = ' ';
				  break;
				}
			    }
			}

		      switch(tempMech->move)
			{
			case MOVE_BIPED:
			  strcpy(move_type, "BIPED");
			  break;
			case MOVE_TRACK:
			  strcpy(move_type, "TRACKED");
			  break;
			case MOVE_WHEEL:
			  strcpy(move_type, "WHEELED");
			  break;
			case MOVE_HOVER:
			  strcpy(move_type, "HOVER");
			  break;
			case MOVE_VTOL:
			  strcpy(move_type, "VTOL");
			  break;
			case MOVE_HULL:
			  strcpy(move_type, "HULL");
			  break;
			case MOVE_FOIL:
			  strcpy(move_type, "HYDROFOIL");
			  break;
			default:
			  strcpy(move_type, "Unknown");
			  break;
			}
		      
		      if (mech->brief) 
                        {
			  sprintf(buff,"%c[%c%c]%c %-12.12s x: %3d y: %3d z: %3d r: %4.1f b: %3d s: %5.1f h: %3d S: %c%c%c%c",
				  weaponarc,
				  contactID[0],contactID[1],
				  move_type[0],
				  mech_name,
				  tempMech->x, tempMech->y, tempMech->z,
				  range, bearing,
				  tempMech->speed,tempMech->facing,
				  (tempMech->status & FALLEN)?'F':' ',
				  (tempMech->status & DESTROYED)?'D':' ',
				  (tempMech->status & JUMPING)?'J':' ',
				  (tempMech->status & STARTED)?' ':'S');
			  rangelist[buffindex] = range;
			  rangelist[buffindex] += 
			    (tempMech->status & DESTROYED) ? 10000 : 0; 
			  strcpy(bufflist[buffindex++],buff);
                        }
                      else
                        {
                          sprintf(buff, "[%c%c]  %-17.17s Tonnage: %d", contactID[0], contactID[1], mech_name, tempMech->tons);
                          notify(player, buff);
                          sprintf(buff, "      Range: %.1f hex\tBearing: %d degrees", range, bearing);
                          notify(player, buff);
                          sprintf(buff, "      Speed: %.1f KPH\tHeading: %d degrees", tempMech->speed, tempMech->facing);
                          notify(player, buff);
                          sprintf(buff, "      X, Y: %3d, %3d \tHeat: %.0f deg C.", tempMech->x, tempMech->y, tempMech->heat);
                          notify(player, buff);
			  sprintf(buff, "      Movement Type: %s", move_type);
			  notify(player, buff);
			  switch(InWeaponArc(mech,tempMech->fx,tempMech->fy)) 
			  {
				case FORWARDARC:
				   notify(player,"      Mech is in Forward Arc");
				   break;
				case RARMARC:
				   notify(player,"      Mech is in Right Arm Arc");
				   break;
				case LARMARC:
				   notify(player,"      Mech is in Left Arm Arc");
				   break;
				case REARARC:
				   notify(player,"      Mech is in Rear Arc");
				   break;
			  }
                          if(tempMech->status & DESTROYED) notify(player,"      Mech Destroyed");
                          if(!(tempMech->status & STARTED)) notify(player,"      Mech Shutdown");
                          if(tempMech->status & FALLEN) notify(player,"      Mech has Fallen!");
                          if(tempMech->status & JUMPING) 
                            notify(player,tprintf("      Mech is Jumping!\tJump Heading: %d",tempMech->jumpheading));
                          notify(player," ");
                        }
		    }
		} 
	    }
	  else if(mech_map->mechsOnMap[loop] == mech->mynum)
	    mechfound=1;
	}
      if (mech->brief) 
	{
	  /* print a sorted list of detected mechs */
	  /* use the ever-popular bubble sort */
	  j = 0;
	  while (j < buffindex) 
	    {
	      for (loop=j+1;loop<buffindex;loop++) 
		{
		  if (rangelist[loop] > rangelist[j]) 
		    {
		      /* found a larger value; swap elements */
		      strcpy(buff, bufflist[loop]);
		      strcpy(bufflist[loop],bufflist[j]);
		      strcpy(bufflist[j],buff);
		      range = rangelist[loop];
		      rangelist[loop] = rangelist[j];
		      rangelist[j] = range;
		    }
		}
	      j++;
	    }
	  for (loop=0;loop<buffindex;loop++) 
	    notify(player, bufflist[loop]);
	  
	}
      notify(player, "End Contact List");
      if(!mechfound) 
	{
	  notify(player,"You aren't officially on this map.  Removing you.");
	  mech->mapindex = -1;
	  mech->status &= ~STARTED;
	  mech->pilot = -1;
	  mech->speed = 0.0;
	  mech->desired_speed=0.0;
	  mech->target= -1;
	  mech->desiredfacing=mech->facing;
	  mech->clock=0;
	  mech->goingx=-1;
	}
    }
}

















