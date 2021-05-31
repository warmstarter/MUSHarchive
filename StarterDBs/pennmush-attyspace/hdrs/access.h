#ifndef __ACCESS_H
#define __ACCESS_H

/* These flags are can/can't - a site may or may not be allowed to do them */
#define ACS_CONNECT	0x1	/* Connect to non-guests */
#define ACS_CREATE	0x2	/* Create new players */
#define ACS_GUEST	0x4	/* Connect to guests */
#define ACS_REGISTER	0x8	/* Site can use the 'register' command */
/* These flags are set in the 'can' bit, but they mark special processing */
#define ACS_SITELOCK    0x10	/* Marker for where to insert @sitelock */
#define ACS_SUSPECT	0x20	/* All players from this site get SUSPECT */
#define ACS_DENY_SILENT 0x40	/* Don't log failed attempts */
#define ACS_REGEXP      0x80  /* Treat the host pattern as a regexp */

/* This is the usual default access */
#define ACS_DEFAULT		(ACS_CONNECT|ACS_CREATE|ACS_GUEST)

/* Usefile macros */
#define Site_Can_Connect(hname)  site_can_access(hname,ACS_CONNECT)
#define Site_Can_Create(hname)  site_can_access(hname,ACS_CREATE)
#define Site_Can_Guest(hname)  site_can_access(hname,ACS_GUEST)
#define Site_Can_Register(hname)  site_can_access(hname,ACS_REGISTER)
#define Deny_Silent_Site(hname) site_can_access(hname,ACS_DENY_SILENT)
#define Suspect_Site(hname)  site_can_access(hname,ACS_SUSPECT)
#define Forbidden_Site(hname)  (!site_can_access(hname,ACS_DEFAULT))

/* Public functions */
int read_access_file _((void));
void write_access_file _((void));
int site_can_access _((const char *hname, const int flag));
int add_access_sitelock _((dbref player, const char *host, const int can, const int cant));
void do_list_access _((dbref player));
int parse_access_options _((const char *opts, int *can, int *cant, dbref player));

#endif				/* __ACCESS_H */
