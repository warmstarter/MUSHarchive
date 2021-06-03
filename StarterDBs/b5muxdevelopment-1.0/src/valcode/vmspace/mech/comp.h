/* -*-C-*-
*******************************************************************************
*
* File:         comp.h
* RCS:          @(#)$Header: /usr/proj/cvs/b5mux/src/valcode/vmspace/mech/comp.h,v 1.1 2001/01/19 23:12:24 wenk Exp $
* Description:  Online computer defs
* Author:       Wes Hardaker
* Created:      Wed Jul 14 03:41:46 1993
* Modified:     Tue Jul 20 13:00:38 1993 (hardaker) hardaker@hemlock
* Language:     C
* Package:      glue code -- special objects
* Status:       Experimental (Do Not Distribute)
*
* (c) Copyright 1993, University of California, all rights reserved.
*
*******************************************************************************
*/
#define dbref int

#define MAXSTR 300
#define DBDIR "computers/"
#define FILESDIR "files/"
#define PERSONDIR "personel/"

/* dir flag list */
#define WRITEABLE 0x1

/* file flag list */

/* #define WRITEABLE 0X1 */  /* use the same as dir list */
/* leave 0x1-0x8 for dir/crossing */
#define HIDDEN 0x10     /* file is hidden */
#define ISDIR  0x20     /* file is actually a directory */

struct file_list
{
  char filename[MAXSTR];
  char subject[MAXSTR];
  dbref owner;
  char flags;
  char prank;
  struct file_list *next;
};


struct comp_dir
{
  char path[MAXSTR];
  struct file_list *files;
  char flags;
  dbref owner;
};

struct comp_data
{
  char faction[MAXSTR];
  dbref user;
  struct comp_dir curdir;
};

/* Prototypes */
void comp_setfaction();
void newfreecomp();
void comp_ls();
void comp_login();
void comp_top();
void ReadDir();
