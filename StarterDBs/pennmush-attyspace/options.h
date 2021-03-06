/* options.h */

#ifndef __OPTIONS_H
#define __OPTIONS_H

/* *********** READ THIS BEFORE YOU MODIFY ANYTHING IN THIS FILE *********** */
/* WARNING:  All options in this file have the ability to signifigantly change
 * the look and feel and sometimes even internal behavior of the program.
 * The ones shipped as the default have been extensively tested.  Others have
 * been tested to a (usually) lesser degree, and therefore might still have
 * latent bugs.  If you change any of them from the default, PLEASE check
 * to make sure that you know the full effects of what you are changing. And
 * if you encounter any errors or compile time problems with any options
 * other than the default settings, PLEASE inform
 * pennmush-bugs@pennmush.org
 * immediately, so that they can be fixed.  The same goes for any other bug
 * you might find in using this software.  All efforts will be made to fix
 * errors encountered, but unless given a FULL description of the error,
 * (IE telling me that logging in doesn't work is insufficient.  telling
 * me that logging in with WCREAT undefined still gives you the registration
 * message is a lot better.  MOST effective would be a full dbx trace, or a
 * patch for the bug.)  Enjoy using the program.
 */
/***************************************************************************/

/*---------------- Internals with many options ------------------------*/

/* The MUSH can encrypt player passwords if you have an encryption function
 * and so desire. This value determines how encryption should be done:
 *  0 -- Don't encrypt passwords, store them as plaintext.
 *       Highly portable, and highly insecure. Not recommended
 *  1 -- Encrypt passwords using the system crypt(3) function.
 *       Recommended for loading dbs from Unix systems that were
 *       compiled this way, but not for starting new MUSHes.
 *  2 -- Encrypt passwords using SHS. SHS works under both Windows and
 *       Unix and if your passwords are encrypted with SHS, you can move
 *       your db between platforms without any trouble. Recommended.
 */
#define CRYPT_SYSTEM	1

/* Malloc package options */
/* malloc() is the routine that allocates memory while the MUSH is
 * running. Because mallocs vary a lot from operating system to operating
 * system, you can choose to use one of the mallocs we provide instead of
 * your operating system's malloc.
 * Set the value of MALLOC_PACKAGE  to one of these values:
 *  0 -- Use my system's malloc. Required for Win32 systems.
 *       Recommended for FreeBSD and Linux.
 *       Otherwise, use only as a last resort.
 *  1 -- Use the CSRI malloc package in normal mode.
 *       Recommended for most operating systems.
 *  2 -- Use the CSRI malloc package in debug mode.
 *  3 -- Use the Satoria malloc (smalloc) package.
 *       Won't work with Linux or FreeBSD, and not generally recommended.
 *  4 -- Use the Satoria malloc package in debug mode.
 *  5 -- Use the GNU malloc (gmalloc) package.
 *       Doesn't work on Alpha processors or FreeBSD systems, and
 *       reportedly flaky on Linux. Requires an ANSI compiler.
 *  6 -- Same as 0, kept for compatibility.
 */
#define MALLOC_PACKAGE	0

/* What type of attribute compression should the MUSH use?
 * Your options are:
 * 1 - the default Huffman compression which has been in use since
 *     pl10. In theory, this should be the best compression,
 *     possibly at the cost of some speed. Sometimes has trouble
 *     on linux systems for some reason.
 * 2 - the bigram compression from pl9 and earlier (but autotuned)
 *     This should be intermediate in compression and speed, and
 *     and at least one database that won't load under Huffman,
 *     will load under bigram. Use if Huffman won't work.
 * 3 - Nick Gammon's word-based compression algorithm.
 *     In theory, this should be considerably faster than Huffman
 *     when decompressing, and considerably slower when compressing.
 *     (But you decompress a lot more often). Compression ratio
 *     is worse than Huffman for small dbs (<1.5Mb of text), but
 *     better for larger dbs. Win32 systems must use this.
 * 0 - No compression at all. Very fast, but your db in memory
 *     will be big - at least as large as your on-disk db.
 *     Possibly suitable for the building stages of a small MUSH.
 * You can change this at any time, with no worries. It only affects
 * the in-memory compression of attribute/mail text, not the disk
 * db compression. Recommend to keep it at 1.
 */
#define COMPRESSION_TYPE	3


/*------------------------- Other internals ----------------------*/


/* Define this if you don't wish your log file to be split up into several
 * components. This saves on file descriptors and makes it easy to tail -f
 * the log, but is inconvenient for quick scanning.
 */
/* #define SINGLE_LOGFILE /* */

/* Defining this adds a simple tracking allocator of allocs and frees that
 * keeps ref counts of what sort of memory is allocated, and how many.
 * Good for testing for Memory leaks. Don't, however, define this unless
 * 1. It's really necessary.  2. You know what you're doing.
 */
/* #define MEM_CHECK /* */

/* Defining this will add 4 bytes to every object in memory, and that is
 * the local_data void *pointer.
 * Usefull for local hackers, who can store stuff like skills, languages,
 * combat stats etc in a record this points to, and then store/reload it
 * in the local_* hooks.
 * If you are NOT one of those who modify the local C source extensivly,
 * this will be COMPLETELY worthless to you.
 */
/* #define LOCAL_DATA /* */

/* If defined, use the info_slave to get information from identd and DNS,
 * instead of having the MUSH do it directly.  This may help reduce lag
 * from new logins.  This does _not_ work under Win32.
 */
/* #define INFO_SLAVE /* */

/* This option uses the old method of coping with newlines.
 * If you're converting from a database that might have hard newlines,
 * load with this defined and do a "@dump/paranoid". Otherwise, ignore it.
 */
/* #define OLD_NEWLINES /* */


/*------------------------- MUSH Features ----------------------*/

/* This allows floating point operations. If you are on a very slow
 * system, you might not wish to define this. It also makes the server
 * somewhat larger. Define this if you find it useful.
 */
#define FLOATING_POINTS /* */

/* This option is to control whether functions may have side effects
 * affecting the db.  With it on, the functions create(), open(),
 * dig(), link(), and set() exist, corresponding to the @commands of
 * the same name.  Also, the 2 parameter versions of zone(), parent(),
 * and lock() are enabled by this define.
 */
#define FUNCTION_SIDE_EFFECTS /* */

/* Comment this out if you don't wish to use the built-in mail system.
 * The @mail command provides a flexible hardcoded mail system, which
 * uses its own database to store messages.
 */
#define USE_MAILER /* */

/*
 * Defining MAIL_SUBJECTS adds subject lines to @mail. This modifies
 * the maildb format, but you can reverse it.
 */
#define MAIL_SUBJECTS /* */

/* Defining ALLOW_NOSUBJECT (which only applies if MAIL_SUBJECTS
 * is defined) marks mail sent with no subject as having subject
 * '(no subject)'. The default is for the subject of the mail to
 * be the first 30 characters of the message when not specfied
 */
#define ALLOW_NOSUBJECT /* */

/*  The chat channels system allows players to talk cross-MUSH to each
 *  other, without needing to be in the same room. Whether or not you
 *  want this depends on what type of MUSH you want.
 */
#define CHAT_SYSTEM /* */

/* Quotes limit players to a fixed number of objects.
 * Wizards can check and set quotas on players.
 * See also restricted_building in game/mush.cnf for another way
 * to slow database growth.
 */
#define QUOTA /* */

/*
 * Define the following to enable the ROYALTY flag.  Players with
 * this flag set have limited wiz powers: ie, they can look, examine,
 * and @tel like wizards, but may not change things like wizards.
 */
#define ROYALTY_FLAG /* */


/*------------------------------ DB ----------------------------------*/

/* If defined, enables the MUSH Building Warning system, which checks
 * for various building problems at a configurable interval or at
 * a player's request. Pretty neat stuff. Will typically add 4 bytes to each
 * object in memory. Recommended.
 */
#define USE_WARNINGS /* */

/* If defined, enables tracking of creation times for all objects,
 * attribute modification times for non-player objects,
 * and number of login failures for player objects.
 * Will typically add 8 bytes to each object in memory
 * Tracking login failures and modification times is a good security
 * measure, too. Also enables the ctime() and mtime() functions.
 * On the whole, recommended unless you're really pinched for memory
 * (i.e., for a 10,000 object db, you can't afford using another 80kb)
 *
 */
#define CREATION_TIMES /* */

/* The following four options control the semantics of empty and
 * deleted attributes.  The configuration recommended for new MUSHes
 * is with all of these options on.  Old MUSHes which do not want
 * to port code may want to use only EMPTY_ATTRS; this recreates the
 * pre-1.6.9 behavior.  Any configuration with EMPTY_ATTRS but not
 * DUMP_EMPTY_ATTRS is _NOT_ recommmended, with the possible exception
 * of emulating the old behavior.
 *
 * With EMPTY_ATTRS, empty (no value) attributes are retained on
 * objects, keeping their attribute flags and locked status.
 * Without this option, such attributes are fully deleted.
 * Recommended.
 */
/* #define EMPTY_ATTRS /* */

/* With this option, '&<attr> <obj>;' will delete an attribute,
 * otherwise it will set the attribute empty.  Recommended, unless
 * emulating pre-1.6.9 behavior.
 */
/* #define DELETE_ATTRS /* */

/* With this option, empty attributes will be visible with hasattr(),
 * lattr(), grep(), etc.  Also controls whether empty attrubutes stop
 * searches through parent chains.  Recommended, unless emulating
 * pre-1.6.9 behavior.
 */
/* #define VISIBLE_EMPTY_ATTRS /* */

/* With this option, empty attributes are included in the database dumps.
 * Otherwise, they are left out, never to be seen again.  Recommended,
 * unless emulating pre-1.6.9 behavior.
 */
/* #define DUMP_EMPTY_ATTRS /* */

/* Defining this option keeps garbage objects from being dumped,
 * possibly saving some disk space.  This does not affect the size
 * of the database in memory.  Recommended.
 */
#define DUMP_LESS_GARBAGE /* */


/*------------------------------ FLAGS --------------------------------*/


/* The fixed flag, when set on a player, prevents the player or anything
 * they own from using @tel or home (roy/wizzes exempted from @tel
 * restriction) Nice for enforcing IC travel. :)
 * The only exception is that players are permitted to @tel their objects
 * to their inventory - which makes coding puppets to follow you possible.
 */
#define FIXED_FLAG /* */

/* If defined, enables the Jury and Judge flags, which don't do anything
 * in themselves, but which other MUSHes may find useful
 */
#define JURY_OK /* */

/* If defined, adds support for the UNREGISTERED flag, which you can
 * arrange in mush.conf to have set on new players, and can test for.
 * The only hardcoded restriction is in wiz.c - unreg'd players can't
 * be given powers
 */
#define ONLINE_REG /* */

/* If defined, adds support for the ON_VACATION flag, which a player
 * can set to indicate that they're going to be away from the MUSH
 * for vacation (and which a wizard can test for when doing player purges,
 * or which can aid other players, etc). This flags is automatically
 * cleared when a player logs in, so it should be set just before the
 * player leaves for vacation
 */
#define VACATION_FLAG /* */

/* If defined, adds the UNINSPECTED flag for rooms, which does nothing,
 * but can be tested for in mushcode, etc.
 */
#define UNINSPECTED_FLAG /* */

/*------------------------------ LOCKS -------------------------------*/

/* If defined, @lock/listen sets a lock which controls who can trigger
 * ^patterns and @ahears on the thing.
 */
#define LISTEN_LOCK /* */

/* If defined, @lock/speech sets a lock which controls who can
 * speak/pose/emit in a room.
 */
#define SPEECH_LOCK /* */

/* If defined, @lock/leave sets a lock which controls who can
 * leave an object
 */
#define LEAVE_LOCK /* */

/* If defined, @lock/drop sets a lock which controls who can drop
 * an object.
 */
#define DROP_LOCK /* */

/* If defined, @lock/give sets a lock which controls who can
 * give away an object
 */
#define GIVE_LOCK /* */

/*------------------------- Cosmetic Features --------------------*/

/* This allows extended ANSI codes to be used in functions. This also
 * results in the game having to filter output for ANSI codes; only
 * define this if you're not worried about CPU usage.
 * This also enables the COLOR flag and the FORCE_WHITE flag.
 */
#define EXTENDED_ANSI /* */


/* If you're using the email registration feature, but want to
 * use a mailer other than sendmail, put the full path to the mailer
 * program here. The mailer must accept the -t command-line
 * argument ("get the recipient address from the message header To:").
 * If it doesn't, you could probably write a wrapper for it.
 * Example: #define MAILER "/full/path/to/other/mailer"
/* #define MAILER /* */


/*------------------------- Log Files ----------------------------*/

/* The main log is given on the command line, after the config file.
 * Checkpoints are written to a separate log, as are connects and
 * disconnects from the game, commands used by wizards, report traces,
 * and player commands. The defaults are listed below.
 */

#define CHECKLOG	"log/checkpt.log"
#define CONNLOG	"log/connect.log"
#define WIZLOG	"log/wizard.log"
#define TRACELOG	"log/trace.log"
#define CMDLOG	"log/command.log"

/*----- TrekMUSH defines ----------------------------------
 * Define the following if you want to engage the economy and space
 * and peripheral code used by ATS TrekMUSH.
 */
#define SPACELOG	"log/space.log"

/*----- TrekMUSH defines ----------------------------------
 * Define the following if you want to engage the economy and space
 * and peripheral code used by ATS TrekMUSH.
 */
#define TREKMUSH /* */

#endif
