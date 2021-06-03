#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"*/

#include "../header.h"
#include "mech.h"

void mech_lrsmap(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
struct mech_data *mech;
int loop, b_width, e_width, b_height, e_height;
int argc, i;
int x, y, mapx, mapy;
struct mech_data *tempMech;
char topbuff[LRS_DISPLAY_WIDTH+20];
char midbuff[LRS_DISPLAY_WIDTH+20];
char botbuff[LRS_DISPLAY_WIDTH+20];
char trash1[10];
char trash2[10];
char *args[5];
char terrain;
char hex_type;
struct map_struct *mech_map;
float fx,fy,fz;
int bearing;
float range;
int target;
int displayHeight;
ATTR *typeAttr;
char move_type_up, move_type_low;


  mech=(struct mech_data *) data;
  if(CheckData(player, mech))
    {
      if(mech->unconcious)
	{
	  notify(player,"You are unconcious...zzzzzz");
	  return;
	}
      if(mech->mapindex == -1)
	{
	  notify(player, "You are not on any map!");
	  return;
	}

      if(!(mech->status & STARTED)) {
	notify(player, "Sensors are not online!");
	return;
      }
      mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
      if(!mech_map)
	{
	  notify(player, "You are on an invalid map! Map index reset!");
	  mech_shutdown(player,data,"");
	  send_mechinfo(tprintf("lrsmap:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
	  mech->mapindex= -1;
	  return;
	}

      mapx = mech->x;
      mapy = mech->y;
      argc=mech_parseattributes(buffer, args, 4);
      if(argc==3)
	{
	  bearing = atoi(args[1]);
	  range = atof(args[2]);
          if((int)range > mech->lrs_range)
            {
              notify(player, "Those coordinates are out of sensor range!");
              return;
            }
          FindXY(mech->fx,mech->fy,bearing,range,&fx,&fy);
          RealCoordToMapCoord(&x,&y,fx,fy);
	}
      else if(argc==2)
        {
          target=FindTargetDBREFFromMapNumber(mech, args[1]);
          tempMech=(struct mech_data *) FindObjectsData(target);
          if(tempMech)
            {
              range=FindRange(mech->fx,mech->fy,mech->fz,
			      tempMech->fx,tempMech->fy,tempMech->fz);
              if((int)range > mech->lrs_range)
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
          x=tempMech->x;
          y=tempMech->y;
        }
      else if(argc==1)
	{
	  x = mech->x;
	  y = mech->y;
	}
      else
	{
	  notify(player, "Invalid number of parameters!");
	  return;
	}
      if(args[0][0]!='M' && args[0][0]!='E' && args[0][0]!='T' &&
         args[0][0]!='m' && args[0][0]!='e' && args[0][0]!='t')
	{
	  notify(player, "Unknown sensor type!");
	  return;
	}

      /*typeAttr=atr_str(GOD, player, "LRSHEIGHT");*/
      if (!typeAttr)
      {
	  displayHeight = LRS_DISPLAY_HEIGHT;
      }
     /* else if (sscanf(atr_get(player,typeAttr),"%d",&displayHeight) != 1 ||
	  displayHeight > 40 || displayHeight < 10)
      {
        notify(player,
	   "Illegal LRSHeight attribute.  Must be between 10 and 40");
	displayHeight = LRS_DISPLAY_HEIGHT;
      }
*/
      displayHeight = (displayHeight <= 2*mech->lrs_range)
        ? displayHeight : 2*mech->lrs_range;

      displayHeight = (displayHeight <= mech_map->map_height)
        ? displayHeight : mech_map->map_height;

      if (!(displayHeight % 2))
	  displayHeight++;

      /* x and y hold the viewing center of the map */
      b_width = x - LRS_DISPLAY_WIDTH2;
      b_width = (b_width < 0) ? 0 : b_width;
      e_width = b_width + LRS_DISPLAY_WIDTH;
      if (e_width >= mech_map->map_width)
      {
	  e_width = mech_map->map_width-1;
	  b_width = e_width - LRS_DISPLAY_WIDTH;
          b_width = (b_width < 0) ? 0 : b_width;
      }
      hex_type = ((b_width % 2) == 0) ? 'o' : 'e';

      b_height = y - displayHeight/2;
      b_height = (b_height < 0) ? 0 : b_height;
      e_height = b_height + displayHeight;
      if (e_height > mech_map->map_height)
      {
	  e_height = mech_map->map_height;
	  b_height = e_height - displayHeight;
	  b_height = (b_height < 0) ? 0 : b_height;
      }
      strcpy(topbuff, "    ");
      strcpy(midbuff, "    ");
      strcpy(botbuff, "    ");
      for(i=b_width;i<=e_width; i++)
        {
          sprintf(trash1, "%3d", i);
          sprintf(trash2, "%c", trash1[0]);
          strcat(topbuff, trash2);
          sprintf(trash2, "%c", trash1[1]);
          strcat(midbuff, trash2);
          sprintf(trash2, "%c", trash1[2]);
          strcat(botbuff, trash2);
        }
      notify(player, topbuff);
      notify(player, midbuff);
      notify(player, botbuff);

      switch(args[0][0])
        {
        case 'M':
        case 'm':
          mech_map->map[mech->y][mech->x].B='*';
          /* Place all the other mechs on the map */
          for(loop=0; loop<MAX_MECHS_PER_MAP; loop++)
            {
              if(mech_map->mechsOnMap[loop] != mech->mynum
                 && mech_map->mechsOnMap[loop] != -1)
                {
                  tempMech=(struct mech_data *)
                    FindObjectsData(mech_map->mechsOnMap[loop]);
                  if(tempMech)
                    {
                      if(InLineOfSight(mech,tempMech,
                                       tempMech->x,tempMech->y,
                                       FindRange(mech->fx,mech->fy,mech->fz,
                                                 tempMech->fx,tempMech->fy,
						 tempMech->fz)))
                        {
			  switch(tempMech->move)
			    {
			    case MOVE_BIPED:
			      move_type_up='B';
			      move_type_low='b';
			      break;
			    case MOVE_TRACK:
			      move_type_up='T';
			      move_type_low='t';
			      break;
			    case MOVE_WHEEL:
			      move_type_up='W';
			      move_type_low='w';
			      break;
			    case MOVE_HOVER:
			      move_type_up='H';
			      move_type_low='h';
			      break;
			    case MOVE_VTOL:
			      move_type_up='V';
			      move_type_low='v';
			      break;
			    case MOVE_HULL:
			      move_type_up='N';
			      move_type_low='n';
			      break;
			    case MOVE_FOIL:
			      move_type_up='F';
			      move_type_low='f';
			      break;
			    default:
			      move_type_up='U';
			      move_type_low='u';
			      break;
			    }
			      
                          mapx= tempMech->x;
                          mapy= tempMech->y;
                          if(tempMech->team == mech->team)
                            {
                              mech_map->map[mapy][mapx].B=move_type_low;
                            }
                          else
                            {
                              mech_map->map[mapy][mapx].B=move_type_up;
                            }
                        }
                    }
                  else
                    {
                      send_mechinfo("Error in mapping routine!");
                    }
                }
            }

          if(hex_type=='e')
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, " %c", mech_map->map[loop][i+1].B);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, "%c ", mech_map->map[loop+1][i].B);
			strcat(botbuff, trash2);
		    }
                  }
		if (i == e_width && loop < (mech_map->map_height-1))
		{
		    strcat(botbuff, tprintf("%c",mech_map->map[loop+1][i].B));
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }
          else
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, "%c ", mech_map->map[loop][i].B);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, " %c", mech_map->map[loop+1][i+1].B);
			strcat(botbuff, trash2);
		    }
                  }
		if (i == e_width)
		{
		    strcat(topbuff, tprintf("%c",mech_map->map[loop][i].B));
		    strcat(botbuff, " ");
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }

          /* Remove mechs from the map */
          mech_map->map[mech->y][mech->x].B=
            mech_map->map[mech->y][mech->x].terrain;
          for(loop=0; loop<MAX_MECHS_PER_MAP; loop++)
            {
              if(mech_map->mechsOnMap[loop] != -1)
                {
                  tempMech=(struct mech_data *)
                    FindObjectsData(mech_map->mechsOnMap[loop]);
                  if(tempMech)
                    {
                      mapx= tempMech->x;
                      mapy= tempMech->y;
                      mech_map->map[mapy][mapx].B=
                        mech_map->map[mapy][mapx].terrain;
                    }
                  else
                    {
                      send_mechinfo("Error in mapping routine!");
                    }
                }
            }
          break;
        case 'T':
        case 't':
          if(hex_type=='e')
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, " %c", mech_map->map[loop][i+1].terrain);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, "%c ",mech_map->map[loop+1][i].terrain);
			strcat(botbuff, trash2);
		    }
                  }
		if (i == e_width && loop < (mech_map->map_height-1))
		{
		    strcat(botbuff,
			      tprintf("%c",mech_map->map[loop+1][i].terrain));
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }
          else
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, "%c ", mech_map->map[loop][i].terrain);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, " %c", mech_map->map[loop+1][i+1].terrain);
			strcat(botbuff, trash2);
		    }
                  }
		if (i == e_width)
		{
		    strcat(topbuff,
				tprintf("%c",mech_map->map[loop][i].terrain));
		    strcat(botbuff," ");
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }
          break;
        case 'E':
        case 'e':
          if(hex_type=='e')
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, " %1d", mech_map->map[loop][i+1].elev);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, "%1d ", mech_map->map[loop+1][i].elev);
			strcat(botbuff, trash2);
		    }
		  }
		if (i == e_width && loop < (mech_map->map_height-1))
		{
		    strcat(botbuff,tprintf("%1d",mech_map->map[loop+1][i].elev));
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }
          else
            {
              for(loop=b_height; loop<e_height; loop++) {
                sprintf(topbuff, "%3d ", loop);
                strcpy(botbuff, "    ");
                for(i=b_width; i < e_width; i=i+2)
                  {
                    sprintf(trash1, "%1d ", mech_map->map[loop][i].elev);
                    strcat(topbuff, trash1);
		    if (loop < (mech_map->map_height-1))
		    {
			sprintf(trash2, " %1d", mech_map->map[loop+1][i+1].elev);
			strcat(botbuff, trash2);
		    }
                  }
		if (i == e_width)
		{
		    strcat(topbuff,tprintf("%1d",mech_map->map[loop][i].elev));
		    strcat(botbuff," ");
		}
                sprintf(trash2, " %-3d", loop+1);
                strcat(botbuff, trash2);
                notify(player, topbuff);
		if (loop < (mech_map->map_height-1))
		    notify(player, botbuff);
              }
            }
          break;
        }
    }
  else
    {
      notify(player,"Those coordinates are out of sensor range.");
    }
}

void mech_tacmap(player,data,buffer)
dbref player;
void *data;
char *buffer;
{
  struct mech_data *mech;
  int loop, b_width, e_width, b_height, e_height;
  int argc, i;
  int x, y, mapx, mapy;
  struct mech_data *tempMech;
  char topbuff[3*MAP_DISPLAY_WIDTH+10];
  char midbuff[3*MAP_DISPLAY_WIDTH+10];
  char botbuff[3*MAP_DISPLAY_WIDTH+10];
  int team_count=0;
  char trash1[10];
  char trash2[10];
  char *args[4];
  char terrain[2];
  char elev[2];
  struct map_struct *mech_map;
  char hex_type;
  float fx, fy;
  float range;
  int bearing;
  int target;
  int displayHeight;
  ATTR *typeAttr;

  mech=(struct mech_data *) data;
  if(CheckData(player, mech))
    {
      if(mech->unconcious)
	{
	  notify(player,"What a nice place to take a nap...zzzzzz");
	  return;
	}
      if(mech->mapindex == -1)
	{
	  notify(player, "You are not on any map!");
	  return;
	}
      if(!(mech->status & STARTED)) {
	notify(player, "Sensors are not online!");
	return;
      }
      mech_map=(struct map_struct *) FindObjectsData(mech->mapindex);
      if(!mech_map) {
	notify(player, "You are on an invalid map! Map index reset!");
	mech_shutdown(player,data,"");
	send_mechinfo(tprintf("tacmap:invalid map:Mech: %d  Index: %d",
						mech->mynum,mech->mapindex));
	mech->mapindex= -1;
	return;
      }

      mapx = mech->x;
      mapy = mech->y;
      argc=mech_parseattributes(buffer, args, 4);
      if(argc==2)
	{
	  bearing = atoi(args[0]);
	  range = atof(args[1]);
          if((int)range > mech->tac_range)
            {
              notify(player, "Those coordinates are out of sensor range!");
              return;
            }
          FindXY(mech->fx,mech->fy,bearing,range,&fx,&fy);
          RealCoordToMapCoord(&x,&y,fx,fy);
	}
      else if(argc==1)
        {
          target=FindTargetDBREFFromMapNumber(mech, args[0]);
          tempMech=(struct mech_data *) FindObjectsData(target);
          if(tempMech)
            {
              range=FindRange(mech->fx,mech->fy,mech->fz,
			      tempMech->fx,tempMech->fy,tempMech->fz);
              if((int)range > mech->tac_range)
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
          x=tempMech->x;
          y=tempMech->y;
        }
      else if(argc==0)
	{
	  x = mech->x;
	  y = mech->y;
	}
      else
	{
	  notify(player, "Invalid number of parameters!");
	  return;
	}

      mech_map->map[mapy][mapx].A='*';
      /* Place all the other mechs on the map */
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++)
        {
          if(mech_map->mechsOnMap[loop] != mech->mynum
             && mech_map->mechsOnMap[loop] != -1)
            {
              tempMech=(struct mech_data *)
                FindObjectsData(mech_map->mechsOnMap[loop]);
              if(tempMech)
                {
                  if(InLineOfSight(mech,tempMech,tempMech->x,tempMech->y,
                                   FindRange(mech->fx,mech->fy,mech->fz,
                                             tempMech->fx,tempMech->fy,
					     tempMech->fz)))
                    {

                      mapx= tempMech->x;
                      mapy= tempMech->y;
                      if(tempMech->team == mech->team)
                        {
                          mech_map->map[mapy][mapx].A=tempMech->ID[0] + 32;
                          mech_map->map[mapy][mapx].B=tempMech->ID[1] + 32;
                        }
                      else
                        {
                          mech_map->map[mapy][mapx].A=tempMech->ID[0];
                          mech_map->map[mapy][mapx].B=tempMech->ID[1];
                        }

                    }
                }
              else
                {
                  send_mechinfo("Error in mapping routine!");
                }
            }
        }

      /*typeAttr=atr_str(GOD, player, "TACHEIGHT");*/
	typeAttr=0;
      if (!typeAttr)
      {
	  displayHeight = MAP_DISPLAY_HEIGHT;
      }
      /*else if (sscanf(atr_get(player,typeAttr),"%d",&displayHeight) != 1 ||
	  displayHeight > 24 || displayHeight < 5)
      {
        notify(player,"Illegal TacHeight attribute.  Must be between 5 and 24");
	displayHeight = MAP_DISPLAY_HEIGHT;
      }
*/
      displayHeight = (displayHeight <= 2*mech->tac_range)
        ? displayHeight : 2*mech->tac_range;

      displayHeight = (displayHeight <= mech_map->map_height)
        ? displayHeight : mech_map->map_height;

      /* x and y hold the viewing center of the map */
      b_width = x - MAP_DISPLAY_WIDTH2;
      b_width = (b_width < 0) ? 0 : b_width;
      e_width = b_width + MAP_DISPLAY_WIDTH;
      if (e_width >= mech_map->map_width)
      {
	  e_width = mech_map->map_width-1;
	  b_width = e_width-MAP_DISPLAY_WIDTH;
	  b_width = (b_width < 0) ? 0 : b_width;
      }
      hex_type = ((b_width % 2) == 0) ? 'e' : 'o';

      b_height = y - displayHeight/2;
      b_height = (b_height < 0) ? 0 : b_height;
      e_height = b_height + displayHeight;
      if (e_height > mech_map->map_height)
      {
	  e_height = mech_map->map_height;
	  b_height = mech_map->map_height-displayHeight;
	  b_height = (b_height < 0) ? 0 : b_height;
      }

      strcpy(topbuff, "     ");
      strcpy(midbuff, "     ");
      strcpy(botbuff, "     ");
      for(i=b_width;i<=e_width; i++)
        {
          sprintf(trash1, "%3d", i);
          sprintf(trash2, "%c  ", trash1[0]);
          strcat(topbuff, trash2);
          sprintf(trash2, "%c  ", trash1[1]);
          strcat(midbuff, trash2);
          sprintf(trash2, "%c  ", trash1[2]);
          strcat(botbuff, trash2);
        }
      notify(player, topbuff);
      notify(player, midbuff);
      notify(player, botbuff);

      if(hex_type=='e')
        {
          for(loop=b_height; loop<e_height; loop++) {
            sprintf(topbuff, "%3d ", loop);
            strcpy(botbuff, "    ");
            for(i=b_width; i < e_width; i=i+2)
              {
                if(mech_map->map[loop][i].terrain==' ')
                  terrain[0]='_';
                else terrain[0]=mech_map->map[loop][i].terrain;

                if(mech_map->map[loop][i].elev)
                  elev[0]=mech_map->map[loop][i].elev + 48;
                else elev[0]=terrain[0];

		if(mech_map->map[loop][i+1].terrain==' ')
		  terrain[1]='_';
		else terrain[1]=mech_map->map[loop][i+1].terrain;

		if(mech_map->map[loop][i+1].elev)
		  elev[1]=mech_map->map[loop][i+1].elev + 48;
		else elev[1]=terrain[1];

                sprintf(trash1, "/%c%c\\%c%c",
                        mech_map->map[loop][i].A,
                        mech_map->map[loop][i].B,
                        terrain[1],
                        elev[1]);

		if (loop < (mech_map->map_height-1))
		    sprintf(trash2, "\\%c%c/%c%c",
                        terrain[0],
                        elev[0],
                        mech_map->map[loop+1][i+1].A,
                        mech_map->map[loop+1][i+1].B);
		else
		    sprintf(trash2, "\\%c%c/XX", terrain[0], elev[0]);

                strcat(topbuff, trash1);
                strcat(botbuff, trash2);
              }
            sprintf(trash1, "%3d", loop);
            strcat(topbuff, trash1);
            notify(player, topbuff);
            notify(player, botbuff);
          }
        }
      else
        {
          for(loop=b_height; loop<e_height; loop++) {
            sprintf(topbuff, "%3d ", loop);
            strcpy(botbuff, "    ");
            for(i=b_width; i < e_width; i=i+2)
              {
                if(mech_map->map[loop][i].terrain==' ')
                  terrain[0]='_';
                else terrain[0]=mech_map->map[loop][i].terrain;

                if(mech_map->map[loop][i].elev)
                  elev[0]=mech_map->map[loop][i].elev + 48;
                else elev[0]=terrain[0];

		if(mech_map->map[loop][i+1].terrain==' ')
		  terrain[1]='_';
		else terrain[1]=mech_map->map[loop][i+1].terrain;

		if(mech_map->map[loop][i+1].elev)
		  elev[1]=mech_map->map[loop][i+1].elev + 48;
		else elev[1]=terrain[1];

		sprintf(trash1, "\\%c%c/%c%c",
			terrain[0],
			elev[0],
			mech_map->map[loop][i+1].A,
			mech_map->map[loop][i+1].B);

		if (loop < (mech_map->map_height-1))
		    sprintf(trash2, "/%c%c\\%c%c",
                        mech_map->map[loop+1][i].A,
                        mech_map->map[loop+1][i].B,
                        terrain[1],
                        elev[1]);
		else
		    sprintf(trash2, "/XX\\%c%c", terrain[1], elev[1]);

                strcat(topbuff, trash1);
                strcat(botbuff, trash2);
              }
            sprintf(trash1, "%3d", loop);
            strcat(topbuff, trash1);
            notify(player, topbuff);
            notify(player, botbuff);
          }
        }

      /* Remove mechs from the map */
      mech_map->map[mech->y][mech->x].A=
        mech_map->map[mech->y][mech->x].terrain;
      for(loop=0; loop<MAX_MECHS_PER_MAP; loop++)
        {
          if(mech_map->mechsOnMap[loop] != -1)
            {
              tempMech=(struct mech_data *)
                FindObjectsData(mech_map->mechsOnMap[loop]);
              if(tempMech)  /* don't really need to call LOS again */
                {
                  mapx= tempMech->x;
                  mapy= tempMech->y;
                  mech_map->map[mapy][mapx].A=
                    mech_map->map[mapy][mapx].terrain;
                  mech_map->map[mapy][mapx].B=
                    mech_map->map[mapy][mapx].terrain;
                }
              else
                {
                  send_mechinfo("Error in mapping routine!");
                }
            }
        }
    }
  else
    {
      notify(player, "Those coordinates are out of tactical sensor range.");
    }
}
