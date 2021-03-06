/* attrs.h - Attribute definitions */
/* $Id: attrs.h,v 1.1 2001/01/15 03:22:19 wenk Exp $ */

#ifndef _ATTRS_H
#define _ATTRS_H

#include "copyright.h"

/* Attribute flags */
#define	AF_ODARK	0x0001	/* players other than owner can't see it */
#define	AF_DARK		0x0002	/* No one can see it */
#define	AF_WIZARD	0x0004	/* only wizards can change it */
#define	AF_MDARK	0x0008	/* Only wizards can see it. Dark to mortals */
#define	AF_INTERNAL	0x0010	/* Don't show even to #1 */
#define	AF_NOCMD	0x0020	/* Don't create a @ command for it */
#define	AF_LOCK		0x0040	/* Attribute is locked */
#define	AF_DELETED	0x0080	/* Attribute should be ignored */
#define	AF_NOPROG	0x0100	/* Don't process $-commands from this attr */
#define	AF_GOD		0x0200	/* Only #1 can change it */
#define	AF_IS_LOCK	0x0400	/* Attribute is a lock */
#define	AF_VISUAL	0x0800	/* Anyone can see */
#define	AF_PRIVATE	0x1000	/* Not inherited by children */
#define AF_DIRTY	0x2000	/* This attribute has been compiled. */

#define	A_OSUCC		1	/* Others success message */
#define	A_OFAIL		2	/* Others fail message */
#define	A_FAIL		3	/* Invoker fail message */
#define	A_SUCC		4	/* Invoker success message */
#define	A_PASS		5	/* Password (only meaningful for players) */
#define	A_DESC		6	/* Description */
#define	A_SEX		7	/* Sex */
#define	A_ODROP		8	/* Others drop message */
#define	A_DROP		9	/* Invoker drop message */
#define	A_OKILL		10	/* Others kill message */
#define	A_KILL		11	/* Invoker kill message */
#define	A_ASUCC		12	/* Success action list */
#define	A_AFAIL		13	/* Failure action list */
#define	A_ADROP		14	/* Drop action list */
#define	A_AKILL		15	/* Kill action list */
#define	A_AUSE		16	/* Use action list */
#define	A_CHARGES	17	/* Number of charges remaining */
#define	A_RUNOUT	18	/* Actions done when no more charges */
#define	A_STARTUP	19	/* Actions run when game started up */
#define	A_ACLONE	20	/* Actions run when obj is cloned */
#define	A_APAY		21	/* Actions run when given COST pennies */
#define	A_OPAY		22	/* Others pay message */
#define	A_PAY		23	/* Invoker pay message */
#define	A_COST		24	/* Number of pennies needed to invoke xPAY */
#define	A_MONEY		25	/* Value or Wealth (internal) */
#define	A_LISTEN	26	/* (Wildcarded) string to listen for */
#define	A_AAHEAR	27	/* Actions to do when anyone says LISTEN str */
#define	A_AMHEAR	28	/* Actions to do when I say LISTEN str */
#define	A_AHEAR		29	/* Actions to do when others say LISTEN str */
#define	A_LAST		30	/* Date/time of last login (players only) */
#define	A_QUEUEMAX	31	/* Max. # of entries obj has in the queue */
#define	A_IDESC		32	/* Inside description (ENTER to get inside) */
#define	A_ENTER		33	/* Invoker enter message */
#define	A_OXENTER	34	/* Others enter message in dest */
#define	A_AENTER	35	/* Enter action list */
#define	A_ADESC		36	/* Describe action list */
#define	A_ODESC		37	/* Others describe message */
#define	A_RQUOTA	38	/* Relative object quota */
#define	A_ACONNECT	39	/* Actions run when player connects */
#define	A_ADISCONNECT	40	/* Actions run when player disconnectes */
#define	A_ALLOWANCE	41	/* Daily allowance, if diff from default */
#define	A_LOCK		42	/* Object lock */
#define	A_NAME		43	/* Object name */
#define	A_COMMENT	44	/* Wizard-accessable comments */
#define	A_USE		45	/* Invoker use message */
#define	A_OUSE		46	/* Others use message */
#define	A_SEMAPHORE	47	/* Semaphore control info */
#define	A_TIMEOUT	48	/* Per-user disconnect timeout */
#define	A_QUOTA		49	/* Absolute quota (to speed up @quota) */
#define	A_LEAVE		50	/* Invoker leave message */
#define	A_OLEAVE	51	/* Others leave message in src */
#define	A_ALEAVE	52	/* Leave action list */
#define	A_OENTER	53	/* Others enter message in src */
#define	A_OXLEAVE	54	/* Others leave message in dest */
#define	A_MOVE		55	/* Invoker move message */
#define	A_OMOVE		56	/* Others move message */
#define	A_AMOVE		57	/* Move action list */
#define	A_ALIAS		58	/* Alias for player names */
#define	A_LENTER	59	/* ENTER lock */
#define	A_LLEAVE	60	/* LEAVE lock */
#define	A_LPAGE		61	/* PAGE lock */
#define	A_LUSE		62	/* USE lock */
#define	A_LGIVE		63	/* Give lock (who may give me away?) */
#define	A_EALIAS	64	/* Alternate names for ENTER */
#define	A_LALIAS	65	/* Alternate names for LEAVE */
#define	A_EFAIL		66	/* Invoker entry fail message */
#define	A_OEFAIL	67	/* Others entry fail message */
#define	A_AEFAIL	68	/* Entry fail action list */
#define	A_LFAIL		69	/* Invoker leave fail message */
#define	A_OLFAIL	70	/* Others leave fail message */
#define	A_ALFAIL	71	/* Leave fail action list */
#define	A_REJECT	72	/* Rejected page return message */
#define	A_AWAY		73	/* Not_connected page return message */
#define	A_IDLE		74	/* Success page return message */
#define	A_UFAIL		75	/* Invoker use fail message */
#define	A_OUFAIL	76	/* Others use fail message */
#define	A_AUFAIL	77	/* Use fail action list */
#define	A_PFAIL		78	/* Invoker page fail message */
#define	A_TPORT		79	/* Invoker teleport message */
#define	A_OTPORT	80	/* Others teleport message in src */
#define	A_OXTPORT	81	/* Others teleport message in dst */
#define	A_ATPORT	82	/* Teleport action list */
#define	A_PRIVS		83	/* Individual permissions */
#define	A_LOGINDATA	84	/* Recent login information */
#define	A_LTPORT	85	/* Teleport lock (can others @tel to me?) */
#define	A_LDROP		86	/* Drop lock (can I be dropped or @tel'ed) */
#define	A_LRECEIVE	87	/* Receive lock (who may give me things?) */
#define	A_LASTSITE	88	/* Last site logged in from, in cleartext */
#define	A_INPREFIX	89	/* Prefix on incoming messages into objects */
#define	A_PREFIX	90	/* Prefix used by exits/objects when audible */
#define	A_INFILTER	91	/* Filter to zap incoming text into objects */
#define	A_FILTER	92	/* Filter to zap text forwarded by audible. */
#define	A_LLINK		93	/* Who may link to here */
#define	A_LTELOUT	94	/* Who may teleport out from here */
#define	A_FORWARDLIST	95	/* Recipients of AUDIBLE output */
#define A_MAILFOLDERS   96	/* @mail folders */
#define	A_LUSER		97	/* Spare lock not referenced by server */
#define	A_LPARENT	98	/* Who may @parent to me if PARENT_OK set */
#define	A_VA		100	/* VA attribute (VB-VZ follow) */
#define A_VM            112     /* VM attribute (VN-VZ follow) */

#define	A_GFAIL		129	/* Give fail message */
#define	A_OGFAIL	130	/* Others give fail message */
#define	A_AGFAIL	131	/* Give fail action */
#define	A_RFAIL		132	/* Receive fail message */
#define	A_ORFAIL	133	/* Others receive fail message */
#define	A_ARFAIL	134	/* Receive fail action */
#define	A_DFAIL		135	/* Drop fail message */
#define	A_ODFAIL	136	/* Others drop fail message */
#define	A_ADFAIL	137	/* Drop fail action */
#define	A_TFAIL		138	/* Teleport (to) fail message */
#define	A_OTFAIL	139	/* Others teleport (to) fail message */
#define	A_ATFAIL	140	/* Teleport fail action */
#define	A_TOFAIL	141	/* Teleport (from) fail message */
#define	A_OTOFAIL	142	/* Others teleport (from) fail message */
#define	A_ATOFAIL	143	/* Teleport (from) fail action */

#define A_LASTPAGE      200     /* Player last paged */
#define A_MAIL		201	/* Message echoed to sender */
#define A_AMAIL		202	/* Action taken when mail received */
#define A_SIGNATURE     203	/* Mail signature */
#define A_DAILY		204	/* Daily attribute to be executed */
#define A_MAILTO	205	/* Who is the mail to? */
#define A_MAILMSG	206	/* The mail message itself */
#define A_MAILSUB	207	/* The mail subject */
#define A_MAILCURF	208	/* The current @mail folder */
#define A_LSPEECH	209	/* Speechlocks */
#define A_PROGCMD	210	/* Command for exectution by @prog */
#define A_MAILFLAGS	211	/* Flags for extended mail */
#define A_DESTROYER	212	/* Who is destroying this object? */
#define A_HTML		213	/* Attribute containing HTML code. */
/* Mike 214-220 */
#define PERCHA_A      	214	/* Perception difficulty */
#define PERCEPT_A	215	/* Actual string shown to player if pass  */
#define FOOD_A		216	/* Food and water list attrib */
/* Val 220-241*/

#define LANG_A		218
#define MSKILL_A	219
#define SKILL_A         220
#define ATRIBS_A	224
#define TSKILL_A        221
#define COMMOD_A	222
#define MONEY_A		223
#define WIELD		225
#define WEAR		226
#define ADVAN_A		227
#define SIZE 		228
#define CARGO		229
#define MCARGO		230
#define FACTION_A	231
#define TITLE_A		232
#define CLASS_A		233
#define XC_A		234
#define YC_A		235
#define ZC_A		236
#define STRUCPTS_A	237
#define RANK_A		238
#define SHIP_A		239
#define COMPUTER_A	240
#define RACE_A		241
#define ITEM_A		242
#define MAINBRIDGE_A	243
#define STRUC_A	244
#define	A_VLIST		252
#define	A_LIST		253
#define	A_STRUCT	254
#define	A_TEMP		255

#define	A_USER_START	256	/* Start of user-named attributes */
#define	ATR_BUF_CHUNK	100	/* Min size to allocate for attribute buffer */
#define	ATR_BUF_INCR	6	/* Max size of one attribute */

#endif
