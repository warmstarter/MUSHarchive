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

void mech_scan(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct map_struct *mech_map;
  char *args[3];
  int mapx,mapy;
  char targetID[2];
  dbref target;
  int numargs, loop;
  int found_target=0;
  struct mech_data *tempMech=NULL;
  float fx,fy,fz;
  float range,enemyX,enemyY,enemyZ;
  char mech_name[100];

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) 
    {
      notify(player, "You are on an invalid map! Map index reset!");
      mech_shutdown(player,data,"");
      send_mechinfo(tprintf("scan:invalid map:Mech: %d  Index: %d",
                                                mech->mynum,mech->mapindex));
      mech->mapindex= -1;
      return;
    }
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
        {
          notify(player,"You are unconscious....zzzzz");
          return;
        }
      if(!(mech->status & STARTED)) 
      {
          notify(player, "Scanners are not online (start the mech first!)");
          return;
      }
    numargs=mech_parseattributes(buffer, args, 3);
    if(numargs==1) 
      {
        /* Scan Target */
        targetID[0]=args[0][0];
        targetID[1]=args[0][1];
        target=FindTargetDBREFFromMapNumber(mech, targetID);
        tempMech=(struct mech_data *) FindObjectsData(target);
        if(tempMech)
          {
            range=FindRange(mech->fx,mech->fy,mech->fz,
			    tempMech->fx,tempMech->fy,tempMech->fz);
            if((int)range > mech->scan_range)
              { 
              notify(player, "Target is out of scanner range.");
              return;
            }  
              if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
              {
                notify(player, "Target is not in line of sight!");
                return;
              }
          } 
        else 
          {
            notify(player, "No such target.");
            return;
          }
      } 
    else if(numargs==2) 
      {
        /* scan x, y */
        mapx=atoi(args[0]);
        mapy=atoi(args[1]);
        MapCoordToRealCoord(mapx,mapy,&fx,&fy);
        range=FindRange(mech->fx,mech->fy,mech->fz,fx,fy,fz);
        if(mapx<0 || mapx>mech_map->map_width || 
           mapy < 0 || mapy >mech_map->map_height ||
           (int)range > mech->scan_range) 
          {
            notify(player, "Those coordinates are out of scanner range.");
            return;
          }
        if(!InLineOfSight(mech,tempMech,mapx,mapy,range)) 
          {
            notify(player, "Coordinates are not in line of sight!");
            return;
          }
	if(mech_map->map[mapy][mapx].terrain==WATER)
	  fz= -ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
	else
	  fz=ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
        /* look for enemies in that hex... */
        for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
          {
            if(mech_map->mechsOnMap[loop] != mech->mynum 
               && mech_map->mechsOnMap[loop] != -1) 
              {
                tempMech=(struct mech_data *) 
                  FindObjectsData(mech_map->mechsOnMap[loop]);
                if(tempMech) 
                  {
                    if(mapx==tempMech->x && mapy==tempMech->y)
                      {
                        found_target=1;
                      }
                  }
                else 
                  { 
                    send_mechinfo("Error in mech_scan routine!");
                  }
              }
          }
        if(!found_target) 
          {
            tempMech=(struct mech_data *) NULL;
          }
      } 
    else if(numargs==0) 
      {
        /* scan current target... */
          target=mech->target;
        tempMech=(struct mech_data *) FindObjectsData(target);
        if(tempMech)
          {
            range=FindRange(mech->fx,mech->fy,mech->fz,
			    tempMech->fx,tempMech->fy,tempMech->fz);
            if((int)range > mech->scan_range)
              { 
                notify(player, "Target is out of scanner range.");
                return;
              }  
            if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
              {
                notify(player, "Target is not in line of sight!");
                return;
              }
          } 
        else 
          {
            if(!FindTargetXY(mech,&enemyX,&enemyY,&enemyZ)) 
              {
                notify(player, "No default target set!");
                return;
              } 
            else 
              {
                mapx=mech->targx;
                mapy=mech->targy;
                MapCoordToRealCoord(mapx,mapy,&fx,&fy);
                range=FindRange(mech->fx,mech->fy,mech->fz,fx,fy,fz);
                if(mapx<0 || mapx>mech_map->map_width || 
                   mapy < 0 || mapy >mech_map->map_height ||
                   (int)range > mech->scan_range) 
                  { 
                    notify(player, "Target hex is out of scanner range.");
                    return;
                  }  
                if(!InLineOfSight(mech,tempMech,mapx,mapy,range)) 
                  {
                    notify(player, "Target hex is not in line of sight!");
                    return;
                  }
		if(mech_map->map[mapy][mapx].terrain==WATER)
		  fz= -ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
		else
		  fz=ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
                /* look for enemies in that hex... */
                for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
                  {
                    if(mech_map->mechsOnMap[loop] != mech->mynum 
                       && mech_map->mechsOnMap[loop] != -1) 
                      {
                        tempMech=(struct mech_data *) 
                          FindObjectsData(mech_map->mechsOnMap[loop]);
                        if(tempMech) 
                          {
                            if(mapx==tempMech->x && mapy==tempMech->y)
                              {
                                found_target=1;
                              }
                          }
                        else 
                          { 
                            send_mechinfo("Error in mech_scan routine!");
                          }
                      }
                  }
                if(!found_target) 
                  {
                    tempMech=(struct mech_data *) NULL;
                  }
              }
          }
      } 
    else 
      {
        notify(player, "Wrong number of arguments to scan!");
        return; 
      }
    if(tempMech) 
      {
        PrintEnemyStatus(player, mech, tempMech, range);

        get_mech_atr(mech, mech_name, "MECHNAME");

        mech_notify(tempMech,MECHSTARTED,
                        tprintf("You are being scanned by %s:[%c%c]", 
                        mech_name, mech->ID[0], mech->ID[1]));
      }
  }
}

void mech_report(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct map_struct *mech_map;
  char *args[3];
  int mapx,mapy;
  char targetID[2];
  dbref target;
  int numargs, loop;
  int found_target=0;
  struct mech_data *tempMech=NULL;
  float fx,fy,fz;
  float range,enemyX,enemyY,enemyZ;
  

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("report:invalid map:Mech: %d  Index: %d",
                                                mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  send_mechdebuginfo("Checking player info.");
  if(CheckData(player, mech)) {
    if(mech->unconcious) 
      {
          notify(player,"You are unconscious....zzzzz");
          return;
      }
    if(!(mech->status & STARTED)) {
        notify(player, "Sensors are not online (start the mech first!)");
        return;
    }
    numargs=mech_parseattributes(buffer, args, 3);
    if(numargs==1) {
      /* Scan Target */
      targetID[0]=args[0][0];
      targetID[1]=args[0][1];
      target=FindTargetDBREFFromMapNumber(mech, targetID);
      tempMech=(struct mech_data *) FindObjectsData(target);
      if(tempMech)
        {
          range=FindRange(mech->fx,mech->fy,mech->fz,
			  tempMech->fx,tempMech->fy,tempMech->fz);
          if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
            {
              notify(player, "Target is not in line of sight!");
              return;
            }
        } 
      else 
        {
          notify(player, "No such target.");
          return;
        }
    } else if(numargs==2) {
      /* report x, y */
      mapx=atoi(args[0]);
      mapy=atoi(args[1]);
      MapCoordToRealCoord(mapx,mapy,&fx,&fy);
      range=FindRange(mech->fx,mech->fy,mech->fz,fx,fy,fz);
      if(mapx<0 || mapx>mech_map->map_width || 
         mapy < 0 || mapy >mech_map->map_height) {
        notify(player, "Those coordinates are out of sensor range.");
        return;
      }
      if(!InLineOfSight(mech,tempMech,mapx,mapy,range)) 
        {
          notify(player, "Coordinates are not in line of sight!");
          return;
        }
      if(mech_map->map[mapy][mapx].terrain==WATER)
	fz= -ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
      else
	fz=ZSCALE*(float)(mech_map->map[mapy][mapx].elev);
      /* look for enemies in that hex... */
      for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
        {
          if(mech_map->mechsOnMap[loop] != mech->mynum 
             && mech_map->mechsOnMap[loop] != -1) 
            {
              tempMech=(struct mech_data *) 
                FindObjectsData(mech_map->mechsOnMap[loop]);
              if(tempMech) 
                {
                  if(mapx==tempMech->x && mapy==tempMech->y)
                    {
                      found_target=1;
                    }
                }
              else 
                { 
                  send_mechinfo("Error in mech_scan routine!");
                }
            }
        }
      if(!found_target) 
        {
          notify(player, "No target found.");
          return;
        }
    } else if(numargs==0) {
      /* report current target... */
      target=mech->target;
      tempMech=(struct mech_data *) FindObjectsData(target);
      if(tempMech)
        {
          range=FindRange(mech->fx,mech->fy,mech->fz,
			  tempMech->fx,tempMech->fy,tempMech->fz);
          if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
            {
              notify(player, "Target is not in line of sight!");
              return;
            }
        } 
      else 
        {
          if(!FindTargetXY(mech,&enemyX,&enemyY,&enemyZ)) 
            {
                notify(player, "No default target set!");
                return;
            } 
          else 
            {
              /* look for enemies in that hex... */
              for(loop=0; loop<MAX_MECHS_PER_MAP && !found_target; loop++) 
                {
                  if(mech_map->mechsOnMap[loop] != mech->mynum 
                     && mech_map->mechsOnMap[loop] != -1) 
                    {
                      tempMech=(struct mech_data *) 
                        FindObjectsData(mech_map->mechsOnMap[loop]);
                      if(tempMech) 
                        {
                          if(mapx==tempMech->x && mapy==tempMech->y)
                            {
                              found_target=1;
                            }
                        }
                      else 
                        { 
                          send_mechinfo("Error in mech_report routine!");
                        }
                    }
                }
              if(!found_target) 
                {
		  notify(player, "No target found.");
                  return;
                }
              else 
                {
                  if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,range)) 
                    {
                      notify(player, "Target is not in line of sight!");
                      return;
                    }
                }
            }
        }
    } else {
      notify(player, "Wrong number of arguments to report!");
      return; 
    }
    if(tempMech) {
      PrintReport(player, mech, tempMech, range);
    }
  }
}

void PrintReport(player,mech,tempMech,range)
     dbref player;
     struct mech_data *mech, *tempMech;
     float range;
{
  int bearing;
  char buff[100];
  int weaponarc;
  char mech_name[100];
  ATTR *typeAttr;
  
  get_mech_atr(tempMech, mech_name, "MECHNAME");
  if(mech->team!=tempMech->team)
    {
      sprintf(buff, "[%c%c]  %-25.25s Tonnage: %d", tempMech->ID[0], tempMech->ID[1], mech_name, tempMech->tons);
      notify(player, buff);
    }
  else
    {
      sprintf(buff, "[%c%c]  %-25.25s Tonnage: %d", tempMech->ID[0]+32, tempMech->ID[1]+32, mech_name, tempMech->tons);
      notify(player, buff);
    }

  bearing=FindBearing(mech->fx,mech->fy,tempMech->fx,tempMech->fy);
  sprintf(buff, "      Range: %.1f hex\t\tBearing: %d degrees", range, bearing);
  notify(player, buff);
  sprintf(buff, "      Speed: %.1f KPH\t\tHeading: %d degrees", tempMech->speed, tempMech->facing);
  notify(player, buff);
  sprintf(buff, "      X, Y, Z: %3d, %3d, %3d\tHeat: %.0f deg C.", tempMech->x, tempMech->y, tempMech->z, tempMech->heat);
  notify(player, buff);
  if(tempMech->sections[TURRET].internal && tempMech->type!=CLASS_MECH)
    {
      sprintf(buff,"      Turret Facing: %d degrees", tempMech->turretfacing);
      notify(player, buff);
    }

  switch (tempMech->move)
    {
    case MOVE_BIPED:
      notify(player, "      Type: MECH                Movement: BIPED");
      break;
    case MOVE_TRACK:
      notify(player, "      Type: VEHICLE             Movement: TRACKED");
      break;
    case MOVE_WHEEL:
      notify(player, "      Type: VEHICLE             Movement: WHEELED");
      break;
    case MOVE_HOVER:
      notify(player, "      Type: VEHICLE             Movement: HOVER");
      break;
    case MOVE_VTOL:
      notify(player, "      Type: VTOL                Movement: VTOL");
      break;
    case MOVE_HULL:
      notify(player, "      Type: NAVAL               Movement: HULL");
      break;
    case MOVE_FOIL:
      notify(player, "      Type: NAVAL               Movement: HYDROFOIL");
      break;
    }

  weaponarc = InWeaponArc(mech,tempMech->fx,tempMech->fy);
  if(mech->type==CLASS_MECH) 
    {
      switch(weaponarc)
	{
	case FORWARDARC:
	  notify(player, "      In Forward Weapons Arc");
	  break;
	case LARMARC:
	  notify(player, "      In Left Arm Weapons Arc");
	  break;
	case RARMARC:
	  notify(player, "      In Right Arm Weapons Arc");
	  break;
	case REARARC:
	  notify(player, "      In Rear Weapons Arc");
	  break;
	default:
	  notify(player, "      NOT in Any Weapons Arc!");
	  break;
	}
    }
  else
    {
      if(weaponarc >= TURRETARC) 
	{
	  notify(player, "      In Turret Arc");
	  weaponarc -= TURRETARC;
	}
      switch(weaponarc)
	{
	case FORWARDARC:
	  notify(player, "      In Forward Weapons Arc");
	  break;
	case RSIDEARC:
	  notify(player, "      In Right Weapons Arc");
	  break;
	case LSIDEARC:
	  notify(player, "      In Left Weapons Arc");
	  break;
	case REARARC:
	  notify(player, "      In Rear Weapons Arc");
	  break;
	default:
	  notify(player, "      NOT in Any Weapons Arc!");
	  break;
	}
    }
  if(tempMech->status & DESTROYED) notify(player,"      DESTROYED");
  if(!(tempMech->status & STARTED)) notify(player,"      SHUTDOWN");
  if(tempMech->status & FALLEN) 
    switch (tempMech->move) 
      {
      case MOVE_BIPED:
	notify(player, "      FALLEN");
	break;
      case MOVE_TRACK:
	notify(player, "      TRACK DESTROYED");
	break;
      case MOVE_WHEEL:
	notify(player, "      AXLE DESTROYED");
	break;
      case MOVE_HOVER:
	notify(player, "      LIFT FAN DESTROYED");
	break;
      case MOVE_VTOL:
	notify(player, "      ROTOR DESTROYED");
	break;
      case MOVE_HULL:
	notify(player, "      ENGINE ROOM DESTROYED");
	break;
      case MOVE_FOIL:
	notify(player, "      FOIL DESTROYED");
	break;
      }
  if(tempMech->status & JUMPING) notify(player,tprintf("      Mech is Jumping!\tJump Heading: %d",tempMech->jumpheading));
  notify(player, " ");
}


void PrintEnemyStatus(player,mymech,mech,range)
dbref player; 
struct mech_data *mymech,*mech; 
float range;
{
  struct mech_data *tempMech;
  char buff[100];
  float x1, y1;

  if(CheckData(player, mech)) {
    PrintReport(player,mymech,mech,range);
    PrintArmorStatus(player, mech);
    if(mech->status & TORSO_RIGHT) {
      notify(player, "Torso is 60 degrees right");
    } 
    if(mech->status & TORSO_LEFT) {
      notify(player, "Torso is 60 degrees left");
    }
    notify(player, " ");
    PrintEnemyWeaponStatus(mech, player);
  }
}

void mech_bearing(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *tempMech;
  struct map_struct *mech_map;
  char *args[4];
  int argc;
  int ix0, iy0;
  float x0, y0, z0;
  int ix1, iy1;
  float x1, y1, z1;
  float temp;
  char trash[20];
  char buff[100];

  x1=y1= -1;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
        {
          notify(player,"You are unconscious....zzzzz");
          return;
        }
      if(!(mech->status & STARTED)) 
        {
          notify(player, 
                 "Target computer is not online (start the mech first!)");
          return;
        }
      x0=mech->fx;
      y0=mech->fy;
      if(mech->mapindex!=-1)
        {
          mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
          if(!mech_map) {
            notify(player, "You are on an invalid map! Map index reset!");
            mech_shutdown(player,data,"");
            send_mechinfo(tprintf("bearing:invalid map:Mech: %d  Index: %d",
                                                mech->mynum,mech->mapindex));
            mech->mapindex= -1;
            return;
          }
          argc=mech_parseattributes(buffer, args, 4);
          if(argc==0) 
            {
              /* Range to current target */
              if(mech->target!=-1) 
                {
                  tempMech=(struct mech_data *) FindObjectsData(mech->target);
                  if(tempMech)
                    {
                      if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,
                                        FindRange(mech->fx,mech->fy,mech->fz,
                                                  tempMech->fx,
                                                  tempMech->fy,
						  tempMech->fz))) 
                        {
                          notify(player, "Target is not in line of sight!");
                          return;
                        }
                    }
                }
              
              if(!FindTargetXY(mech, &x1, &y1, &z1)) 
                {
                  notify(player, "There is no default target!");
                } 
              else 
                {
                  strcpy(buff, "Bearing to default target is: ");
                }
            } 
          else if(argc==2) 
            {
              /* Range to X, Y */
              ix1=atoi(args[0]);
              iy1=atoi(args[1]);
              if(!(ix1>=0 && ix1<mech_map->map_width && iy1>=0 
                   && iy1<mech_map->map_height)) 
                {
                  notify(player, "Invalid map coordinates!");
                  x1=y1= -1.;
                } 
              else 
                {
                  sprintf(buff, "Bearing to  %d,%d is: ", ix1, iy1);
                  MapCoordToRealCoord(ix1,iy1,&x1,&y1);
                }
            } 
          else if(argc==4) 
              {
                ix0=atoi(args[0]);
                iy0=atoi(args[1]);
                ix1=atoi(args[2]);
                iy1=atoi(args[3]);
                
                if(!(ix1>=0 && ix1<mech_map->map_width 
                     && iy1>=0 && y1<mech_map->map_height 
                     && ix0>=0 && ix0<= mech_map->map_width 
                     && iy0>=0 && iy0<mech_map->map_height)) 
                  {
                    notify(player, "Invalid map coordinates!");
                    x1=y1= -1;
                  } 
                else 
                  {
                    sprintf(buff, "Bearing to %d,%d from %d,%d is: ",
                            ix0, iy0, ix1, iy1);
                  MapCoordToRealCoord(ix1,iy1,&x1,&y1);
                  MapCoordToRealCoord(ix0,iy0,&x0,&y0);
                  }
              } 
            else 
              {
                notify(player, 
                       "Invalid number of attributes to Bearing function!");
              }
          if(x1 != -1) 
            {
              temp=FindBearing(x0, y0, x1, y1);
              sprintf(trash, "%.0f degrees.", temp);
              strcat(buff, trash);
              /*notify(player, buff);*/
              notify_all_from_inside(where_is(player),1, buff);
            }
        }
      else
        {
          notify(player, "You are not on a map!");
        }
    }
}

void mech_range(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *tempMech;
  struct map_struct *mech_map;
  char *args[4];
  int argc;
  int ix0, iy0;
  float x0, y0, z0;
  int ix1, iy1;
  float x1, y1, z1;
  float temp;
  char trash[20];
  char buff[100];

  x1=y1= -1;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
        {
          notify(player,"You are unconscious....zzzzz");
          return;
        }
      if(!(mech->status & STARTED)) 
        {
          notify(player, 
                 "Target computer is not online (start the mech first!)");
          return;
        }
      x0=mech->fx;
      y0=mech->fy;
      z0=mech->fz;
      if(mech->mapindex!=-1)
        {
          mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
          if(!mech_map) {
            notify(player, "You are on an invalid map! Map index reset!");
            mech_shutdown(player,data,"");
            send_mechinfo(tprintf("range:invalid map:Mech: %d  Index: %d",
                                                mech->mynum,mech->mapindex));
            mech->mapindex= -1;
            return;
          }
          argc=mech_parseattributes(buffer, args, 4);
          if(argc==0) 
            {
              /* Range to current target */
              if(mech->target!=-1) 
                {
                  tempMech=(struct mech_data *) FindObjectsData(mech->target);
                  if(tempMech)
                    {
                      if(!InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,
                                        FindRange(mech->fx,mech->fy,mech->fz,
                                                  tempMech->fx,
                                                  tempMech->fy,
						  tempMech->fz))) 
                        {
                          notify(player, "Target is not in line of sight!");
                          return;
                        }
                    }
                }
              
              if(!FindTargetXY(mech, &x1, &y1, &z1)) 
                {
                  notify(player, "There is no default target!");
                  return;
                } 
              else 
                {
                  strcpy(buff, "Range to default target is: ");
                }
            } 
          else if(argc==2) 
            {
              /* Range to X, Y */
              ix1=atoi(args[0]);
              iy1=atoi(args[1]);
              if(!(ix1>=0 && ix1<mech_map->map_width && iy1>=0 
                   && iy1<mech_map->map_height)) 
                {
                  notify(player, "Invalid map coordinates!");
                  x1=y1= -1.;
                } 
              else 
                {
                  sprintf(buff, "Range to  %d,%d is: ", ix1, iy1);
                  MapCoordToRealCoord(ix1,iy1,&x1,&y1);
		  if(mech_map->map[iy1][ix1].terrain==WATER)
		    z1= -ZSCALE*(float)(mech_map->map[iy1][ix1].elev);
		  else
		    z1=ZSCALE*(float)(mech_map->map[iy1][ix1].elev);
                }
            } 
          else if(argc==4) 
              {
                /* Range to X, Y from given X, Y */
                ix0=atoi(args[0]);
                iy0=atoi(args[1]);
                ix1=atoi(args[2]);
                iy1=atoi(args[3]);
                
                if(!(ix1>=0 && ix1<mech_map->map_width 
                     && iy1>=0 && y1<mech_map->map_height 
                     && ix0>=0 && ix0<= mech_map->map_width 
                     && iy0>=0 && iy0<mech_map->map_height)) 
                  {
                    notify(player, "Invalid map coordinates!");
                    x1=y1= -1;
                  } 
                else 
                  {
                    sprintf(buff, "Range to %d,%d from %d,%d is: ",
                            ix0, iy0, ix1, iy1);
                  MapCoordToRealCoord(ix1,iy1,&x1,&y1);
                  MapCoordToRealCoord(ix0,iy0,&x0,&y0);
		  if(mech_map->map[iy1][ix1].terrain==WATER)
		    z1= -ZSCALE*(float)(mech_map->map[iy1][ix1].elev);
		  else
		    z1=ZSCALE*(float)(mech_map->map[iy1][ix1].elev);
		  if(mech_map->map[iy0][ix1].terrain==WATER)
		    z0= -ZSCALE*(float)(mech_map->map[iy0][ix0].elev);
		  else
		    z0=ZSCALE*(float)(mech_map->map[iy0][ix0].elev);
                  }
              } 
            else 
              {
                notify(player, 
                       "Invalid number of attributes to Range function!");
              }
          if(x1 != -1) 
            {
              temp=FindRange(x0, y0, z0, x1, y1, z1);
              sprintf(trash, "%.1f hexes.", temp);
              strcat(buff, trash);
              /*notify(player, buff);*/
              notify_all_from_inside(where_is(player),1, buff);
            }
        }
      else
        {
          notify(player, "You are not on a map!");
        }
    }
}

void PrintEnemyWeaponStatus(mech,player)
struct mech_data *mech; 
dbref player; 
{
  unsigned char weaparray[MAX_WEAPS_SECTION];
  unsigned char weapdata[MAX_WEAPS_SECTION];
  int critical[MAX_WEAPS_SECTION];
  int count;
  int loop;
  int ii,jj;
  int len;
  char weapname[20];
  char weapbuff[70];
  char tempbuff[50];
  char location[20];
  int running_sum=0;
  int ammoLoc;
  int ammoCrit;
  int rounds;

  notify(player, "================WEAPON SYSTEMS================");
  notify(player, "----- Weapon ------ [##]  Location ---- Status");
  for(loop=0; loop<NUM_SECTIONS; loop++) {
    count=FindWeapons(mech, loop, weaparray, weapdata, critical);
    if(count) {
      ArmorStringFromIndex(loop, tempbuff, mech->type);
      sprintf(location,"%-14.14s",tempbuff);

      for(ii=0; ii<count; ii++) {
        sprintf(weapbuff," %-18.18s [%2d]  ",&MechWeapons[weaparray[ii]].name[3],running_sum+ii);
        strcat(weapbuff, location);
        
        if(weapdata[ii]==CRIT_DESTROYED) 
          {
            strcat(weapbuff, "*****");
          } 
        else 
          { 
            if(weapdata[ii])  
              {
                strcat(weapbuff, "-----");
              } 
            else 
              { 
                strcat(weapbuff, "Ready");
              }
          }
        notify(player, weapbuff);
      }
      running_sum += count;
    }
  }
}

void mech_sight(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech;
  struct map_struct *mech_map;
  char *args[5];
  int argc;
  int weapnum;

  mech=(struct mech_data *) data;
  mech_map=(struct map_struct *)FindObjectsData(mech->mapindex);
  if(!mech_map) {
    notify(player, "You are on an invalid map! Map index reset!");
    mech_shutdown(player,data,"");
    send_mechinfo(tprintf("fireweapon:invalid map:Mech: %d  Index: %d",
                                                mech->mynum,mech->mapindex));
    mech->mapindex= -1;
    return;
  }
  if(CheckData(player, mech)) 
    {
      if(!(mech->status & STARTED)) 
        {
          notify(player, "Fusion reactor is not online!");
          return;
        }
      if(mech->unconcious) 
        {
          notify(player, "You sight your weapons in your dream...zzzzz");
          return;
        }
      argc=mech_parseattributes(buffer, args, 5);
      if(argc>=1) 
        {
          weapnum=atoi(args[0]);
          FireWeaponNumber(player,mech,mech_map,weapnum,argc,args,1);
        } 
      else 
        {
          notify(player, "Not enough arguments to the function");
        }
    }
}

void mech_view(player,data,buffer)
dbref player; 
void *data; 
char *buffer; 
{
  struct mech_data *mech, *target;
  int targetnum;
  char targetID[5];
  char *args[5];
  int argc;
  char target_desc[1024];
  char buff[1024];

  mech=(struct mech_data *) data;
  if(CheckData(player, mech)) 
    {
      if(mech->unconcious) 
        {
          notify(player,"You are unconscious....zzzzzzz");
          return;
        }
      if(!(mech->status & STARTED)) 
        {
          notify(player, "Fusion reactor is not online!");
          return;
        }
      argc=mech_parseattributes(buffer, args, 2);
      if(argc == 0)  /* default target */
        {
          if (mech->target == -1)
            {
              mech_notify(mech,MECHALL, "You do not have a default target set!");
              return;
            }
          target=(struct mech_data *) FindObjectsData(mech->target);
          if(!target)
            {
              mech_notify(mech,MECHALL, "Invalid default target!");
              mech->target = -1;
              return;
            }
          if(get_mech_atr(target, target_desc, "MECHDESC"))
            {
              /*rt_parse(buff, target_desc);*/
              notify(player, buff);
            }
          else
            notify(player, "That target has no markings.");
        }
      else if (argc == 1) /* ID number */
        {
          targetID[0]=args[0][0];
          targetID[1]=args[0][1];
          targetnum=FindTargetDBREFFromMapNumber(mech, targetID);
          if(targetnum == -1) 
            {
              notify(mech->pilot,"That is not a valid target ID!");
              return;
            }
          target = (struct mech_data *)FindObjectsData(targetnum);
          if(!target)
            {
              mech_notify(mech,MECHALL, "Invalid target data!");
              return;
            }
          if(get_mech_atr(target, target_desc, "MECHDESC"))
            {
              /*rt_parse(buff, target_desc);*/
              notify(player, buff);
            }
          else
            notify(player, "That target has no markings.");
        }
      else 
        notify(player, "Invalid number of arguments to function.");
    }
}
