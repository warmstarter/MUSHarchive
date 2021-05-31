/*-------------------------------------------------------------------*
 * malias.c - Global @mail aliases/lists
 *
 * This code implements an extension to extended @mail which allows
 * admin (and others who are so em@powered) to create mail aliases
 * for the MUSH. Optionally, any player can be allowed to.
 *
 * Aliases are used by @mail'ing to !<alias name>
 * Aliases have a name, a description, a list of members (dbrefs), an owner
 * a size (how many members), and two kinds of flags. 
 * nflags control who can use/see an alias name, and mflags 
 * control who can see the alias members. The choices
 * are everyone, alias members, owner, admin
 * 
 * Interface:
 * @malias[/list]
 * @malias/members !name
 * @malias[/create] !name=list-of-members
 * @malias/destroy !name
 * @malias/add !name=list-of-members
 * @malias/remove !name=list-of-members
 * @malias/desc !name=description
 * @malias/nameprivs !name=flags
 * @malias/listprivs !name=flags
 * @malias/stat
 * @malias/chown !name=owner	(Admin only)
 * @malias/nuke 		(Admin only)
 *-------------------------------------------------------------------*/

#include "config.h"
#include "conf.h"
#include "malias.h"
#include "confmagic.h"

#ifdef MAIL_ALIASES

#endif				/* MAIL_ALIASES */
