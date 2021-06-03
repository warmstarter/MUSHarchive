/*
  Map.c
  
  Routines for handling the map special objects.

  4.8.93- rdm created
*/

#define send_mechinfo(x)

#define dbref int
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "db.h"*/
#include "../header.h"
#define GOD 1
#include "mech.h"

/* Selectors */
#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

/* Local prototypes */

const struct hex_struct DefaultMap =
{
  ' ',' ',' ', 0
};

void map_view(player,data,buffer)
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  int loop, b_width, b_height, argc, i;
  int e_width, e_height, x, y;
  char topbuff[3*MAP_DISPLAY_WIDTH+10];
  char midbuff[3*MAP_DISPLAY_WIDTH+10];
  char botbuff[3*MAP_DISPLAY_WIDTH+10];
  char trash1[7];
  char trash2[7];
  char *args[4];
  char hex_type;
  char terrain[2];
  char elev[2];
  int displayHeight;
  ATTR *typeAttr;
  
  map=(struct map_struct *) data;
  if(CheckData(player, map)) 
    {
      /* x and y hold the viewing center of the map */
      argc=mech_parseattributes(buffer, args, 4);
      if(argc==2) 
	{
	  x=atoi(args[0]);
	  y=atoi(args[1]);
	  if((x<map->map_width)&&(y<map->map_height)&&(x>=0)&&(y>=0)) 
	    {
	      notify(player, tprintf("-- Map: %s (%d x %d) --", 
				map->mapname,map->map_width,map->map_height));

	      /*typeAttr=atr_str(GOD, player, "TACHEIGHT");*/
		typeAttr=0;
	      if (!typeAttr)
	      {
		  displayHeight = MAP_DISPLAY_HEIGHT;
	      }
	      /*else if (sscanf(atr_get(player,typeAttr),"%d",&displayHeight)!=1 
			|| displayHeight > 24 || displayHeight < 5)
	      {
		notify(player,
		    "Illegal TacHeight attribute.  Must be between 5 and 24");
		displayHeight = MAP_DISPLAY_HEIGHT;
	      }
		*/
	      displayHeight = (displayHeight <= map->map_height)
		? displayHeight : map->map_height;

	      /* x and y hold the viewing center of the map */
	      b_width = x - MAP_DISPLAY_WIDTH2;
	      b_width = (b_width < 0) ? 0 : b_width;
	      e_width = b_width + MAP_DISPLAY_WIDTH;
	      if (e_width >= map->map_width)
	      {
		  e_width = map->map_width-1;
		  b_width = e_width-MAP_DISPLAY_WIDTH;
		  b_width = (b_width < 0) ? 0 : b_width;
	      }
	      hex_type = ((b_width % 2) == 0) ? 'e' : 'o';

	      b_height = y - displayHeight/2;
	      b_height = (b_height < 0) ? 0 : b_height;
	      e_height = b_height + displayHeight;
	      if (e_height > map->map_height)
	      {
		  e_height = map->map_height;
		  b_height = (map->map_height-displayHeight);
		  b_height = (b_height < 0) ? 0 : b_height;
	      }

	      sprintf(topbuff, "     ");
	      sprintf(midbuff, "     ");
	      sprintf(botbuff, "     ");
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
		    sprintf(botbuff, "    ");
		    for(i=b_width; i < e_width; i=i+2) 
		      {
			if(map->map[loop][i].terrain==' ') terrain[0]='_'; 
			else terrain[0]=map->map[loop][i].terrain;

			if(map->map[loop][i].elev) 
			  elev[0]=map->map[loop][i].elev + 48;
			else elev[0]=terrain[0];

			if(map->map[loop][i+1].terrain==' ') terrain[1]='_';
			else terrain[1]=map->map[loop][i+1].terrain;
			
			if(map->map[loop][i+1].elev) 
			  elev[1]=map->map[loop][i+1].elev + 48;
			else elev[1]=terrain[1];

			sprintf(trash1, "/%c%c\\%c%c", 
				map->map[loop][i].terrain,
				map->map[loop][i].terrain,
				terrain[1],
				elev[1]);

			if (loop < (map->map_height-1))
			    sprintf(trash2, "\\%c%c/%c%c", 
				terrain[0],
				elev[0], 
				map->map[loop+1][i+1].terrain,
				map->map[loop+1][i+1].terrain);
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
		    sprintf(botbuff, "    ");
		    for(i=b_width; i < e_width; i=i+2) 
		      {
			if(map->map[loop][i].terrain==' ') terrain[0]='_';
			else terrain[0]=map->map[loop][i].terrain;

			if(map->map[loop][i].elev) 
			  elev[0]=map->map[loop][i].elev + 48;
			else elev[0]=terrain[0];

			if(map->map[loop][i+1].terrain==' ') terrain[1]='_';
			else terrain[1]=map->map[loop][i+1].terrain;

			if(map->map[loop][i+1].elev) 
			  elev[1]=map->map[loop][i+1].elev + 48;
			else elev[1]=terrain[1];

			sprintf(trash1, "\\%c%c/%c%c", 
				terrain[0],
				elev[0], 
				map->map[loop][i+1].terrain,
				map->map[loop][i+1].terrain);
			if (loop < (map->map_height-1))
			    sprintf(trash2, "/%c%c\\%c%c", 
				map->map[loop+1][i].terrain,
				map->map[loop+1][i].terrain, 
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
	    } 
	  else 
	    {
	      notify(player, "X,Y out of range!");
	    }	   
	} 
      else 
	{
	  notify(player, "Invalid number of parameters!");	
	}
    }
}

void map_addhex(player,data,buffer)
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  int x, y, argc;
  char *args[4], elev;

  map=(struct map_struct *) data;
  if(CheckData(player, map)) {
    argc=mech_parseattributes(buffer, args, 4);
    if(argc==4) {
      x=atoi(args[0]);
      y=atoi(args[1]);
      elev=abs(atoi(args[3]));
      if((x>=0) && (x<map->map_width) && (y>=0) && (y<map->map_height)) {
	if(args[2][0]=='.') 
	  {
	    map->map[y][x].terrain=' ';
	  }
	else 
	  {
	    map->map[y][x].terrain=args[2][0];
	    map->map[y][x].A=args[2][0];
	    map->map[y][x].B=args[2][0];
	  } 
	map->map[y][x].elev=(elev<=MAX_ELEV) ? elev : MAX_ELEV;
	notify(player, "Hex set!");
      } else {
	notify(player, "X,Y out of range!");
      }
    } else {
      notify(player, "Invalid number of parameters!");
    }
  }
}

void map_addrow(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  int y, argc, elev;
  char *args[4], temp[2];
  int i, len;

/*
  map=(struct map_struct *) data;
  if(CheckData(player, map)) {
    argc=mech_parseattributes(buffer, args, 4);
    if(argc==3) {
      y=atoi(args[0]);
      if((y>=0) && (y<map->map_height)) {
	len=strlen(args[1]);
	if(len>map->map_width) len=map->map_width;
	for(i=0; i<len; i++) {
          sprintf(temp, "%c ", args[2][i]);
          elev=atoi(temp);
	  map->map[y][i].C=(args[1][i]=='.') ? ' ' : args[1][i];
	  if(elev!=0) map->map[y][i].D=(elev<=MAX_ELEV) 
	    ? args[2][i] : MAX_ELEV_C;
	  else map->map[y][i].D=args[1][i];
          map->map[y][i].A=map->map[y][i].C;
          map->map[y][i].B=map->map[y][i].C;
	}
	notify(player, "Row set!");
      } else {
	notify(player, "Y out of range!");
      }
    } else {
      notify(player, "Invalid number of parameters!");
    }
  }
*/
  notify(player, "Under Construction");
}

void map_mapname(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  notify(player, "Command disabled");
}

void map_loadmap(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
struct map_struct *map;
char *args[1];
char openfile[50];
struct mech_data readmech;
FILE *fp;
char row[MAPX];
int i, j, height, width, failed=0, temp;

    map=(struct map_struct *) data;

    if(CheckData(player, map)) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	sprintf(openfile, "maps/");
	strcat(openfile, args[0]);
	if((fp=fopen(openfile, "r"))) {
	  notify(player, tprintf("Loading %s", args[0]));
	
	  /* free the old map if there is one there for some reason */
	  if(map->map) 
	    {
	      for(i=map->map_height-1;i>=0;i--) free((char*) (map->map[i]));
	      free((char*) (map->map));
	    }
	
	  fscanf(fp, "%d %d\n", &height, &width);
	  map->map=(hex_struct **) malloc(height*sizeof(hex_struct*));
	  for(i=0;i<height;i++) 
	    {
	      map->map[i]=
		(hex_struct *) malloc(width*sizeof(hex_struct));
	      if (!map->map[i]) 
		{ 
		  send_mechinfo("Loadmap malloc failed! (Map)");
		  failed=1;
		  i = height;
		} 
	    }
	  if(!failed) 
	    {
	      for(i=0;i<height;i++) 
		{
		  fscanf(fp, "%s\n", row);
		  for(j=0;(j<width) && !feof(fp);j++) 
		    {
		      sscanf(&row[2*j], "%c%1d", 
			     &map->map[i][j].terrain, &temp);
		      map->map[i][j].elev=temp;
		      map->map[i][j].terrain=(map->map[i][j].terrain=='.') 
			? ' ' : map->map[i][j].terrain;
		      map->map[i][j].A=map->map[i][j].terrain;
		      map->map[i][j].B=map->map[i][j].terrain;
		    } 
		}
	      if(j!=width || i!=height) 
		 {
		   send_mechinfo("Error: EOF reached prematurely.");
		   fclose(fp);
		   return;
		 }
		 
	      map->map_height=height;
	      map->map_width=width;
	      fscanf(fp, "%d\n", &map->city);
	      for(i=0;i<NUM_MAP_LINKS;i++) 
		fscanf(fp, "%d ", &map->links[i]);
              sprintf(map->mapname, args[0]);
	      notify(player, "Loading complete!");
	      fclose(fp);

	      notify(player,"Clearing Mechs off Newly Loaded Map");
	      map_clearmechs(player,data,"");
	    }
	} else {
	  notify(player, "Unable to read that template.");
	}
      } else {
	notify(player, "Unable to load that template.");
      }
    }
}

void map_savemap(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
struct map_struct *map;
char *args[1];
FILE *fp;
char openfile[50];
int i, j;
char row[MAPX];
char temp[5];
char terrain;

    map=(struct map_struct *) data;

    if(CheckData(player, map)) {
      if(mech_parseattributes(buffer, args, 1)==1) {
	notify(player, tprintf("Saving %s", args[0]));
	sprintf(openfile, "maps/");
	strcat(openfile, args[0]);
	if((fp=fopen(openfile, "w"))) {
	  fprintf(fp, "%d %d\n", map->map_height, map->map_width);
	  for(i=0;i<map->map_height;i++) 
	    {
	      sprintf(row,"");
	      for(j=0;j<map->map_width;j++) 
		{
		  terrain=(map->map[i][j].terrain==' ') 
		    ? '.' : map->map[i][j].terrain;
		  
		  sprintf(temp, "%c%1d", terrain, map->map[i][j].elev); 
		  strcat(row, temp); 
		}
	      fprintf(fp, "%s\n", row);
	    }
	  fprintf(fp, "%d\n", map->city);
	  for(i=0;i<NUM_MAP_LINKS;i++) 
	    fprintf(fp, "%d ", map->links[i]);
	  notify(player, "Saving complete!");
	  fclose(fp);
	} else {
	  notify(player, "Unable to open/create map file! Sorry.");
	}
      } else {
	notify(player, "You must specify a map name!");
      }
    }
}

void map_setmapsize(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
struct map_struct *oldmap;
hex_struct **map;
int x, y, i, j, failed=0, argc, x1, y1;
char *args[4];

  oldmap=(struct map_struct *) data;
  if(CheckData(player, oldmap)) {
    argc=mech_parseattributes(buffer, args, 4);
    if(argc==2) {
      x=atoi(args[0]);
      y=atoi(args[1]);
      if((x>=0) && (x<=MAPX) && (y>=0) &&
	 (y<=MAPY)) {
	/* allocate new map space */
	map=(hex_struct **) malloc(y*sizeof(hex_struct*));
	for(i=0;i<y;i++) 
	{
	  map[i]=
	    (hex_struct *) malloc(x*sizeof(hex_struct));
	  if (!map[i]) 
	    { 
	    send_mechinfo("Setmapindx alloc failed! (Map)");  /* notify Jim */
	    failed=1;
	    i = y;
	  }
        }
	if (failed) 
	  {
	   send_mechinfo("Memory allocation failed in setmapsize!");
	  }
	else 
	  {
	    /* Initialize the hexes in the new map to blank */
	    for(i=0;i<y;i++)
	      for(j=0;j<x;j++)
		map[i][j] = DefaultMap;
	    
	    /* Copy old map into new map */
            x1=(oldmap->map_width < x) ? oldmap->map_width : x;
            y1=(oldmap->map_height < y) ? oldmap->map_height : y;
            for(i=0; i<y1; i++)
              for(j=0; j<x1; j++)
                map[i][j] = oldmap->map[i][j];

	    /* Now free the old map */
	    for(i=oldmap->map_height-1;i>=0;i--) free((char*) (oldmap->map[i]));

	    /* set new map size and pointer to new map space */
	    oldmap->map_height=y;
	    oldmap->map_width=x;
	    oldmap->map=map;
	    notify(player, "Size set.");
	  }
      } else {
	notify(player, "X,Y out of range!");
      }
    } else {
      notify(player, "Invalid number of parameters!");
    }
  }
}

void map_listmechs(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  struct mech_data *tempMech;
  int i;
  int count=0;
  char valid[50];
  char ID[2];

  map=(struct map_struct *) data;
  if(CheckData(player, map)) {
    notify(player, "--- Mechs on Map ---");
    for(i=0; i<MAX_MECHS_PER_MAP; i++) {
      if(map->mechsOnMap[i] != -1) 
	{
	  tempMech = (struct mech_data *) FindObjectsData(map->mechsOnMap[i]);
	  if(tempMech) 
            {
              strcpy(valid,"Valid Data");
              ID[0]=tempMech->ID[0];
              ID[1]=tempMech->ID[1];
            }
          else 
            { 
              strcpy(valid,"Invalid Object Data!  Remove this Mech!");
              ID[0]='*';
              ID[1]='*';
            }
	  notify(player, tprintf("Mech DB Number: %d : [%c%c]\t%s",
				 map->mechsOnMap[i],
                                 ID[0],ID[1],
                                 valid));
          count++;
        }
    }
    notify(player, tprintf("%d Mechs On Map",count));
    notify(player, tprintf("%d positions open",MAX_MECHS_PER_MAP-count));
  }
}

void map_clearmechs(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  int i;
  int argc;
  dbref mechnum;
  char *args[1];

  map=(struct map_struct *) data;
  if(CheckData(player, map)) 
    {
      ShutDownMap(player, map->mynum);
    }
}

void map_addlink(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  struct map_struct *map;
  int argc;
  char *args[4];
  dbref link;
  int i;

  map=(struct map_struct *) data;
  if(CheckData(player, map)) {
    argc=mech_parseattributes(buffer, args, 4);
    
    for(i=0; i<argc; i++) {
      link=atoi(args[i]);
      if((link== -1)|| ((link>=0)&&(link<mudstate.db_top))) {
	map->links[i]=link;
	notify(player, tprintf("Link set to: %d", link));
      } else {
	notify(player, "Link value not in valid range!");
      }
    }
  }
}

void map_startxy(player, data, buffer) 
dbref player; 
void *data; 
char *buffer;
{
  notify(player, "Command to be disabled");
}

/* Mem alloc/free routines */
void newfreemap(key,data,selector)
dbref key; 
void **data; 
int selector; 
{
  struct map_struct *new;
  int i, j, failed=0;

  switch(selector) {
  case SPECIAL_ALLOC:
    new=(struct map_struct *) malloc(sizeof(struct map_struct));
    if(new) {
      new->mynum=key;
      new->map_width=DEFAULT_MAP_WIDTH;
      new->map_height=DEFAULT_MAP_HEIGHT;
      /* allocate default map space */
      new->map=(hex_struct **) malloc((new->map_height)*sizeof(hex_struct*));
      for(i=0;i<new->map_height;i++) 
	{
	  new->map[i]=
	    (hex_struct *) malloc((new->map_width)*sizeof(hex_struct));
	  if (!new->map[i]) 
	    { 
	    send_mechinfo( "Special_ALLOC failed! (Map)"); 
            failed=1;
	    i=new->map_height;
            }
        }
      if (!failed) 
	{
	  for(i=0; i<MAX_MECHS_PER_MAP; i++) {
	    new->mechsOnMap[i]= -1;
	  }
	  new->city= -1;      
	  for(i=0; i<NUM_MAP_LINKS; i++) {
	    new->links[i]= -1;
	  }
	  sprintf(new->mapname ,"%s", "Default Map");
	  for(i=0; i<new->map_height; i++)
	    for(j=0; j<new->map_width; j++)
	      new->map[i][j] = DefaultMap;
	  
	  for(i=0; i<MAP_UNUSED_SIZE; i++) {
	    new->unused[i]=0;
	  }
	  *data=new;
	}
      else 
	{
	  send_mechinfo( "Special_ALLOC failed! (Map)");
	}  
      } 
    else 
      {
      send_mechinfo( "Special_ALLOC failed! (Map)");
    }
    break;

  case SPECIAL_FREE:
    if(*data) {
      /* free map internal to the structure */
      new = (struct map_struct *) *data;
      if (new->map != 0x0)
	{
	  for(i=new->map_height-1;i>=0;i--) if (new->map[i]) free((char*) (new->map[i]));
	  free((char*) (new->map));
	}
      /* Now free the structure */
      free(new);
      send_mechinfo( "Free successful (Map)");
    } else {
      send_mechinfo( "Special Free failed (Map)");
    }
    break;
  }
}
