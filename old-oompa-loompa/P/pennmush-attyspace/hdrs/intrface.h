/* intrface.h */

#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "copyrite.h"
#include "conf.h"
#include "mushdb.h"

/* these symbols must be defined by the interface */

typedef struct descriptor_data DESC;
extern DESC *cdesc;

extern int shutdown_flag;	/* if non-zero, interface should shut down */
extern void emergency_shutdown _((void));
extern void boot_desc _((DESC *d));	/* remove a player */
extern DESC *player_desc _((dbref player));	/* find descriptors */
extern DESC *inactive_desc _((dbref player));	/* find descriptors */
extern DESC *port_desc _((int port));	/* find descriptors */
extern void flag_broadcast _((object_flag_type inflags, object_flag_type intoggles, const char *fmt,...));
extern void raw_notify _((dbref player, const char *msg));
extern dbref short_page _((const char *match));
extern void do_doing _((dbref player, const char *message));

/* the following symbols are provided by game.c */

/* max length of command argument to process_command */
#define MAX_COMMAND_LEN 4096
#define BUFFER_LEN ((MAX_COMMAND_LEN)*2)

#define SBUF_LEN 32
#define DOING_LEN 40

extern void process_command _((dbref player, char *command, dbref cause, int from_port));

/* from player.c */
extern dbref create_player _((const char *name, const char *password, const char *host, const char *ip));
extern dbref connect_player _((const char *name, const char *password, const char *host, const char *ip));
extern void check_last _((dbref player, const char *host, const char *ip));
extern void check_lastfailed _((dbref player, const char *host));

extern int init_game_dbs _((const char *conf));
extern void init_game_config _((const char *conf));
extern void dump_database _((void));
extern void panic _((const char *message));

#endif
