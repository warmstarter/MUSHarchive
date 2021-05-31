/* mail.h - header for Javelin's extended mailer */

#ifndef _EXTMAIL_H
#define _EXTMAIL_H

/* Some of this isn't implemented yet, but heralds the future! */
#define M_READ		0x0001
#define M_UNREAD	0x0FFE
#define M_CLEARED	0x0002
#define M_URGENT	0x0004
#define M_MASS		0x0008
#define M_EXPIRE	0x0010
#define M_RECEIPT	0x0020
#define M_TAG		0x0040
#define M_FORWARD	0x0080
/* 0x0100 - 0x0F00 reserved for folder numbers */
#define M_FMASK		0xF0FF
#define M_ALL		0x1000	/* In mail_selector, all msgs in all folders */
#define M_MSUNREAD	0x2000	/* Mail selectors */
#define M_REPLY		0x4000
#define M_FOLDER	0x8000	/* In mail selector, all msgs in cur folder */
/* 0x4000 - 0x8000 available */

#define MAX_FOLDERS	15
#define FOLDER_NAME_LEN (BUFFER_LEN / 30)
#define FolderBit(f) (256 * (f))
#define Urgent(m)	(m->read & M_URGENT)
#define Mass(m)		(m->read & M_MASS)
#define Expire(m)	(m->read & M_EXPIRE)
#define Receipt(m)	(m->read & M_RECEIPT)
#define Forward(m)	(m->read & M_FORWARD)
#define Reply(m)	(m->read & M_REPLY)
#define Tagged(m)	(m->read & M_TAG)
#define Folder(m)	((m->read & ~M_FMASK) >> 8)
#define Read(m)		(m->read & M_READ)
#define Cleared(m)	(m->read & M_CLEARED)
#define Unread(m)	(!Read(m))
#define All(ms)		(ms.flags & M_ALL)
#define AllInFolder(ms)	(ms.flags & M_FOLDER)
#define MSFolder(ms)	((ms.flags & ~M_FMASK) >> 8)

typedef unsigned int mail_flag;

struct mail_selector {
  int low, high;
  mail_flag flags;
  dbref player;
  int days, day_comp;
};

typedef int folder_array[MAX_FOLDERS + 1];
#define FA_Init(fa,x) \
for (x = 0; x <= MAX_FOLDERS; x++) \
fa[x] = 0

#ifdef MAIL_SUBJECTS
#define SUBJECT_COOKIE	'/'
#define SUBJECT_LEN	60
#endif

#define MDBF_SUBJECT	0x1
#define MDBF_ALIASES	0x2

#endif				/* _EXTMAIL_H */
