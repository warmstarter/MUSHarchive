/* -*-C-*-
*******************************************************************************
*
* File:         comp.c
* RCS:          @(#)$Header: /usr/proj/cvs/b5mux/src/valcode/vmspace/mech/comp.c,v 1.1 2001/01/19 23:12:24 wenk Exp $
* Description:  Online Computer
* Author:       Wes Hardaker
* Created:      Wed Jul 14 03:36:36 1993
* Modified:     Thu Jul 22 09:55:35 1993 (hardaker) hardaker@hemlock
* Language:     C
* Package:      glue code -- special objects
* Status:       Experimental (Do Not Distribute)
*
* (c) Copyright 1993, University of California, all rights reserved.
*
*******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/file.h>

#include "config.h"
/*#include "externs.h"
#include "db.h"*/

#include "../header.h"
#include "comp.h"

#define SPECIAL_FREE 0
#define SPECIAL_ALLOC 1

void comp_ls(player, data, buffer)
dbref player; 
void *data; 
char *buffer; 
{

  struct file_list *flist;
  struct comp_data *comp;
  char ctmp[MAXSTR];
  
  comp = (struct comp_data *) data;
  if (CheckData(player, comp) && CheckUser(player,comp->user))
    {
      notify(player,"-------------------------------------------------------------------------------");
      notify(player,"flgs filename Subject");
      notify(player,"-------------------------------------------------------------------------------");
      for (flist = comp->curdir.files; flist != NULL; flist = flist->next)
	{
	  sprintf(ctmp,"%c %-10.10s %-50.50s",
		  (flist->flags & ISDIR)?'d':'-',
		  flist->filename,
		  flist->subject);
	  notify(player,ctmp);
        }
      notify(player,"-------------------------------------------------------------------------------");
    }
}

void comp_login(player, data, buffer)
dbref player; 
void *data; 
char *buffer; 
{

  struct file_list *flist;
  struct comp_data *comp;
  
  comp = (struct comp_data *) data;
  if (CheckData(player, comp))
    {
      if (comp->user == -1)
	{
	  notify(player, 
		 tprintf("Login:  %s's Computer Information Network (CIN)", comp->faction));
	  comp->curdir.path[0] = NULL;
	  comp->user = player;
	  ReadDir(&comp->curdir, player, comp->faction);
	}
      else
	{
	  notify(player, "Sorry this terminal is in use.");
	}
    }
}

void comp_logout(player, data, buffer)
dbref player; 
void *data; 
char *buffer; 
{

  struct file_list *flist;
  struct comp_data *comp;
  
  comp = (struct comp_data *) data;
  if (CheckData(player, comp) && CheckUser(player,comp->user))
    {
      notify(player, tprintf("Logging Out:  %s's CIN",comp->faction));
      comp->user = -1;
    }
}


void ReadDir(dir, player, faction)
     struct comp_dir *dir;
     dbref player;
     char *faction;
{
  FILE *FDIR;
  struct file_list **ftmp;
  char ctmp[MAXSTR], curdir[MAXSTR];
  
  sprintf(curdir,"%s%s%s%s/.dir",DBDIR,FILESDIR,faction,dir->path);
  if ((FDIR = fopen(curdir,"r")) == NULL)
    {
      notify(player, 
	     tprintf("Error reading directory %s.  Contact a director.",
		     dir->path));
      return;
    }
  
  if (fscanf(FDIR,"%d\n",&dir->owner) != 1)
    {
      notify(player, 
	     tprintf("Error reading directory %s.  Contact a director.",
		     dir->path));
      fclose(FDIR);
      return;
    }

  ftmp = &dir->files;
  
  while (fgets(ctmp, MAXSTR, FDIR) != NULL)
    {
      (*ftmp)=(struct file_list *) malloc(sizeof(struct file_list));
      strncpy ((*ftmp)->filename,ctmp,strlen(ctmp)-1);
      if (fgets(ctmp,MAXSTR,FDIR) == NULL)
	{
	  notify(player, 
		 tprintf("Error reading directory %s.  Contact a director.",
			 dir->path));
	  fclose(FDIR);
	  return;
	}
      strncpy ((*ftmp)->subject,ctmp,strlen(ctmp)-1);
      if (fscanf(FDIR,"%d %c %c\n",&((*ftmp)->owner), &((*ftmp)->flags), 
		 &((*ftmp)->prank)) != 3)
	{
	  notify(player, 
		 tprintf("Error reading directory %s.  Contact a director.",
			 dir->path));
	  fclose(FDIR);
	  return;
	}
      ftmp = &((*ftmp)->next);
    }
/*  if (ftmp) dir->files = ftmp; */
  fclose(FDIR);
}

int CheckUser(player1, player2)
{
  if (player1 != player2)
    {
      notify(player1, "Sorry, you must log on first.");
      return(0);
    }
  return(1);
}

void comp_top(player, data, buffer)
dbref player; 
void *data; 
char *buffer; 
{

  struct comp_data *comp;

  comp = (struct comp_data *) data;
  if (CheckData(player, comp) && CheckUser(player,comp->user))
    {
      comp->curdir.path[0] = NULL;
      ReadDir(&comp->curdir,player, comp->faction);
    }
}  


void comp_setfaction(player, data, buffer)
dbref player; 
void *data; 
char *buffer; 
{

  struct comp_data *comp;
  char *args[1], tbuff[MAXSTR];

  if (mech_parseattributes(buffer, args, 1) != 1)
    {
      notify(player, "You need to specify a faction.");
      return;
    }
  
  comp = (struct comp_data *) data;
  if (CheckData(player, comp))
    {
      strcpy(comp->faction,args[0]);
      sprintf(tbuff, "Faction set to %s",comp->faction);
      notify(player,tbuff);
      comp->curdir.path[0] = NULL;
      ReadDir(&comp->curdir,player, comp->faction);
    }
}  


void newfreecomp(key,data,selector)
     dbref key;
     void **data;
     int selector;
{
  
  struct comp_data *new;

  switch(selector)
    {
    case SPECIAL_ALLOC:
      new=(struct comp_data *) malloc(sizeof(struct comp_data));
      if (new)
	{
	  new->faction[0] = NULL;
	  new->curdir.path[0] = NULL;
	  new->user = -1;
/*	  ReadDir(&new->curdir,1); */
	  *data = new;
	}
      else
	{
	  send_mechinfo("Special_ALLOC failed");
	}
      break;
    case SPECIAL_FREE:
      if (*data)
	{
	  free(*data);
	}
      else
	{
	  send_mechinfo("Special Free failed");
	}
      break;
    }
}

	
	      
  
      

