#ifndef PLAYERSTATS_H
#define PLAYERSTATS_H

struct player_stats
{

  /* Since I cannot bring some header declarations into the mech combat code,
     the header portion of this structure has been placed in a separate
     file so that it may be safely accessed from hcode. -- MW 93 Oct */

#include "btech_playerstats_hdr.h"

};

extern struct player_stats *create_new_stats P((void));
extern void add_stats_to_table P((struct player_stats *));
extern struct player_stats *get_stats P((dbref));

extern void list_skills P((dbref));
extern void list_attributes P((dbref));
extern void list_advantages P((dbref));

extern int char_rollunskilled P((void));
extern int char_rollskilled P((void));
extern int char_rollsaving P((void));
extern int char_rolld6 P((int));

extern int char_getadvantage P((dbref, char *));

extern int char_getattribute P((dbref, char *));
extern int char_getattrsave P((dbref, char *));
extern int char_getattrsavesucc P((dbref, char *));

extern int char_getcharacteristic P((dbref, char *));
extern int char_getcharacteristicsucc P((dbref, char *));

extern int char_getskillcode P((char *));
extern int char_getcode P((char *, char **, int));

extern int char_getskillbycode P((dbref, int));
extern int char_getskill P((dbref, char *));
extern int char_getskilltargetbycode P((dbref, int, int));
extern int char_getskilltarget P((dbref, char *, int));

extern int char_getskillsuccess P((dbref, char *, int));
extern int char_getskillmargsuccess P((dbref, char *, int));
extern int char_getopposedskill P((dbref, char *, dbref, char *));

extern int char_sumattributes P((dbref, int));
extern void clear_player P((struct player_stats *));

#endif PLAYERSTATS_H
