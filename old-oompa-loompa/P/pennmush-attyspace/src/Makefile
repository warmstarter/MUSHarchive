# Makefile for PennMUSH


###############################################################################
#         You shouldn't have to change anything in this file.                 #
###############################################################################

# stupid SYS V shell
SHELL=/bin/sh

# for lint target
LINT=lint
LINTFLAGS=-haz $(DEFINES)
LINTFILT=egrep -v '(possible pointer|long assign|not yet im|:$$)'

# Libs
LIBS=$(CLIBS) $(RLIBS) $(ILIBS)

CFLAGS=$(CCFLAGS) $(RDEFS) $(IDEFS)

# Used for protoizing
#.c.o:
#	$(CC) $(CFLAGS) -aux-info $<.X -c $<

# uncomment the following line if you want to use the concentrator
#CONC = concentrate

OUTFILES = buildinf netmud mkindx $(CONC) info_slave

# List of C files, used for make depend:
C_TREK=	magazine.c \
	space_variables.c space_ansi.c space_utils.c \
	space_balance.c space_error.c space_unparse.c \
	space_notify.c space_alert.c space_format.c \
	space_report.c space_iterate.c space_get.c \
	space_put.c space_misc.c space_read.c space_write.c \
	space_set.c space_status.c space_function.c

C_FILES=access.c atr_tab.c attrib.c boolexp.c bsd.c cmdlocal.c cmds.c \
	command.c compress.c conf.c cque.c create.c db.c destroy.c \
	extchat.c extmail.c filecopy.c flags.c function.c fundb.c funlist.c \
	funlocal.c funmath.c funmisc.c funstr.c funtime.c funufun.c game.c \
	htab.c ident.c local.c lock.c log.c look.c mycrypt.c mymalloc.c \
	match.c memcheck.c move.c parse.c player.c plyrlist.c \
	predicat.c privtab.c regexp.c \
	rrob.c rpage.c rwho.c services.c set.c sig.c speech.c strdup.c \
	strutil.c timer.c unparse.c utils.c version.c warnings.c wild.c wiz.c $(C_TREK)

H_TREK =  ../hdrs/magazine.h ../hdrs/space.h

H_FILES = ../hdrs/attrib.h ../hdrs/conf.h ../hdrs/copyrite.h ../hdrs/mushdb.h \
	  ../hdrs/flags.h ../hdrs/externs.h ../hdrs/globals.h \
	  ../hdrs/help.h ../hdrs/intrface.h ../hdrs/match.h \
	  ../hdrs/lock.h ../hdrs/warnings.h ../hdrs/mymalloc.h ../hdrs/ident.h \
	  ../hdrs/memcheck.h ../hdrs/oldattrb.h ../options.h \
	  ../hdrs/game.h ../hdrs/atr_tab.h ../hdrs/dbdefs.h ../hdrs/ansi.h \
	  ../hdrs/version.h ../hdrs/getpgsiz.h ../hdrs/myregexp.h \
	  ../config.h ../confmagic.h ../hdrs/function.h ../hdrs/parse.h \
	  ../hdrs/extmail.h ../hdrs/extchat.h \
	  ../hdrs/htab.h ../hdrs/command.h ../hdrs/htab.h ../hdrs/privtab.h $(H_TREK)

# .o versions of above - these are used in the build
O_TREK= \
magazine.o \
space_alert.o \
space_ansi.o \
space_balance.o \
space_error.o \
space_format.o \
space_function.o \
space_get.o \
space_iterate.o \
space_misc.o \
space_notify.o \
space_put.o \
space_read.o \
space_report.o \
space_set.o \
space_status.o \
space_unparse.o \
space_utils.o \
space_variables.o \
space_write.o

O_FILES= \
access.o \
atr_tab.o \
attrib.o \
boolexp.o \
bsd.o \
cmdlocal.o \
cmds.o \
command.o \
compress.o \
conf.o \
cque.o \
create.o \
db.o \
destroy.o \
extchat.o \
extmail.o \
filecopy.o \
flags.o \
function.o \
fundb.o \
funlist.o \
funlocal.o \
funmath.o \
funmisc.o \
funstr.o \
funtime.o \
funufun.o \
game.o \
htab.o \
ident.o \
local.o \
lock.o \
log.o \
look.o \
mycrypt.o \
mymalloc.o \
match.o \
memcheck.o \
move.o \
parse.o \
player.o \
plyrlist.o \
predicat.o \
privtab.o \
regexp.o \
rob.o \
rpage.o \
rwho.o \
services.o \
set.o \
sig.o \
speech.o \
strdup.o \
strutil.o \
timer.o \
unparse.o \
utils.o \
version.o \
warnings.o \
wild.o \
wiz.o $(O_TREK)


# This is a dummy target, in case you type 'make' in the source
# directory (likely for emacs users who M-x compile.)
# This means that the top-level make had better specifically 'make all' :)
first:
	(cd ..; make)

all: $(OUTFILES)


netmud: $(O_FILES)
	@echo "Making netmud."
	-mv -f netmud netmud~
	$(CC) $(LDFLAGS) -o netmud $(O_FILES) $(LIBS)

# By default, db.c initially allocates enough space for 10000 objects, then
#   grows the space if needed.  To change this value, include
#   -DDB_INITIAL_SIZE=xxxx where xxxx is the new value (minimum 1).

mkindx: mkindx.c
	@echo "Making mkindx."
	$(CC) $(CCFLAGS) -o mkindx.o -c mkindx.c
	$(CC) $(LDFLAGS) -o mkindx mkindx.o

info_slave: info_slave.c ident.c strdup.c sig.c
	@echo "Making info_slave."
	$(CC) $(CCFLAGS) $(IDEFS) -c info_slave.c
	$(CC) $(LDFLAGS) -o info_slave info_slave.o ident.o strdup.o sig.o $(LIBS)


# ../hdrs/buildinf.h contains build information used by version.c:
# time/date of build and CFLAGS
# It should always be out of date.
buildinf:
	-rm -f ../hdrs/buildinf.h
	@echo "/* This file generated automatically from Makefile */" >> ../hdrs/buildinf.h
	@echo "#define BUILDDATE \"`date`\"" >> ../hdrs/buildinf.h
	@echo "#define COMPILER \"$(CC)\"" >> ../hdrs/buildinf.h
	@echo "#define CCFLAGS \"$(CCFLAGS)\"" >> ../hdrs/buildinf.h
	@echo "#define RDEFS \"$(RDEFS)\"" >> ../hdrs/buildinf.h
	@echo "#define IDEFS \"$(IDEFS)\"" >> ../hdrs/buildinf.h

# If funlocal.c doesn't exist, we want to build it from
# funlocal.dst.
funlocal.c: funlocal.dst
	if [ ! -f funlocal.c ]; then \
	  cp funlocal.dst funlocal.c; \
	fi

cmdlocal.c: cmdlocal.dst
	if [ ! -f cmdlocal.c ]; then \
	  cp cmdlocal.dst cmdlocal.c; \
	fi

local.c: local.dst
	if [ ! -f local.c ]; then \
	  cp local.dst local.c; \
	fi

etags:
	etags *.c ../*.h ../hdrs/*.h

ctags:
	ctags *.c

depend: funlocal.c cmdlocal.c local.c
	makedepend -w10 -- $(CFLAGS) -- $(C_FILES) $(H_FILES)
	../utils/fixdepend.pl Makefile

# Requires GNU indent!
indent:
	(set +e; for file in *.c *.dst ../hdrs/*.h ; do echo $$file; /usr/local/bin/indent -kr -ci2 -ss -psl -ip4 -i2 -cs \
	-T ATRALIAS -T ATRNHASH -T DESC -T CNode -T CONF -T BQUE -T FUN \
	-T NVAL -T i_rec -T f_rec -T USERFN_ENTRY -T CPRIV -T FLAG \
	-T FLAGENT -T FLAG_ALIAS -T tlist -T u -T stat -T tcheck -T ATTR \
	-T ALIST -T CHTAB -T FBLKHDR -T FBLOCK -T OPTTAB -T dbref \
	-T object_flag_type -T channel_type -T boolexp_type -T CHAN \
	-T mail_flag -T help_indx -T lock_type -T lock_list -T MEM \
	-T warn_type -T POWER -T POWER_ALIAS -T acsflag \
	-T switch_mask -T COMLIST -T COMALIAS -T COMMAND -T SWITCH_VALUE \
	-T COMSORTSTRUC -T CHANUSER \
	$$file ; done)

clean:
	-rm -f *.o
	-rm -f a.out core gmon.out $(OUTFILES)

distclean: clean
	-rm -f *~ *.orig *.rej *.bak funlocal.c cmdlocal.c local.c \#*

test_compress: comp_h.c
	$(CC) $(CFLAGS) -o test_compress -DSTANDALONE comp_h.c

decompress: decompress.o compress.o
	-rm -f decompress
	$(CC) $(CFLAGS) -o decompress decompress.o compress.o $(LIBS)

portmsg: portmsg.c
	$(CC) $(CFLAGS) -o portmsg portmsg.c $(LIBS)

# Some dependencies that make depend doesn't handle well
compress.o: comp_b.c comp_h.c comp_w.c
version.o: ../hdrs/buildinf.h
cmds.o: ../hdrs/funs.h ../hdrs/cmds.h
funcrypt.o: ../hdrs/ansi.h

# DO NOT DELETE THIS LINE -- make depend depends on it.

access.o: ../config.h
access.o: ../hdrs/copyrite.h
access.o: ../hdrs/conf.h
access.o: ../options.h
access.o: ../hdrs/externs.h
access.o: ../hdrs/intrface.h
access.o: ../hdrs/mushdb.h
access.o: ../hdrs/attrib.h
access.o: ../hdrs/flags.h
access.o: ../hdrs/lock.h
access.o: ../hdrs/dbdefs.h
access.o: ../hdrs/extchat.h
access.o: ../hdrs/myregexp.h
access.o: ../confmagic.h
access.o: ../hdrs/access.h
access.o: ../hdrs/mymalloc.h
atr_tab.o: ../config.h
atr_tab.o: ../hdrs/conf.h
atr_tab.o: ../hdrs/copyrite.h
atr_tab.o: ../options.h
atr_tab.o: ../hdrs/externs.h
atr_tab.o: ../hdrs/intrface.h
atr_tab.o: ../hdrs/mushdb.h
atr_tab.o: ../hdrs/attrib.h
atr_tab.o: ../hdrs/flags.h
atr_tab.o: ../hdrs/lock.h
atr_tab.o: ../hdrs/dbdefs.h
atr_tab.o: ../hdrs/extchat.h
atr_tab.o: ../hdrs/myregexp.h
atr_tab.o: ../confmagic.h
atr_tab.o: ../hdrs/atr_tab.h
atr_tab.o: ../hdrs/htab.h
atr_tab.o: ../hdrs/privtab.h
atr_tab.o: ../hdrs/mymalloc.h
attrib.o: ../hdrs/copyrite.h
attrib.o: ../config.h
attrib.o: ../hdrs/conf.h
attrib.o: ../options.h
attrib.o: ../hdrs/attrib.h
attrib.o: ../hdrs/dbdefs.h
attrib.o: ../hdrs/extchat.h
attrib.o: ../hdrs/externs.h
attrib.o: ../hdrs/intrface.h
attrib.o: ../hdrs/mushdb.h
attrib.o: ../hdrs/flags.h
attrib.o: ../hdrs/lock.h
attrib.o: ../hdrs/myregexp.h
attrib.o: ../confmagic.h
attrib.o: ../hdrs/match.h
attrib.o: ../hdrs/oldattrb.h
attrib.o: ../hdrs/parse.h
attrib.o: ../hdrs/htab.h
attrib.o: ../hdrs/privtab.h
attrib.o: ../hdrs/mymalloc.h
boolexp.o: ../hdrs/copyrite.h
boolexp.o: ../config.h
boolexp.o: ../hdrs/conf.h
boolexp.o: ../options.h
boolexp.o: ../hdrs/mushdb.h
boolexp.o: ../hdrs/attrib.h
boolexp.o: ../hdrs/flags.h
boolexp.o: ../hdrs/lock.h
boolexp.o: ../hdrs/dbdefs.h
boolexp.o: ../hdrs/extchat.h
boolexp.o: ../hdrs/match.h
boolexp.o: ../hdrs/externs.h
boolexp.o: ../hdrs/intrface.h
boolexp.o: ../hdrs/myregexp.h
boolexp.o: ../confmagic.h
boolexp.o: ../hdrs/parse.h
bsd.o: ../hdrs/copyrite.h
bsd.o: ../config.h
bsd.o: ../hdrs/conf.h
bsd.o: ../options.h
bsd.o: ../hdrs/mushdb.h
bsd.o: ../hdrs/attrib.h
bsd.o: ../hdrs/flags.h
bsd.o: ../hdrs/lock.h
bsd.o: ../hdrs/dbdefs.h
bsd.o: ../hdrs/extchat.h
bsd.o: ../hdrs/intrface.h
bsd.o: ../hdrs/externs.h
bsd.o: ../hdrs/myregexp.h
bsd.o: ../confmagic.h
bsd.o: ../hdrs/globals.h
bsd.o: ../hdrs/help.h
bsd.o: ../hdrs/match.h
bsd.o: ../hdrs/ansi.h
bsd.o: ../hdrs/pueblo.h
bsd.o: ../hdrs/parse.h
bsd.o: ../hdrs/access.h
bsd.o: ../hdrs/version.h
bsd.o: ../hdrs/ident.h
bsd.o: ../hdrs/mymalloc.h
cmdlocal.o: ../hdrs/copyrite.h
cmdlocal.o: ../config.h
cmdlocal.o: ../hdrs/externs.h
cmdlocal.o: ../hdrs/conf.h
cmdlocal.o: ../options.h
cmdlocal.o: ../hdrs/intrface.h
cmdlocal.o: ../hdrs/mushdb.h
cmdlocal.o: ../hdrs/attrib.h
cmdlocal.o: ../hdrs/flags.h
cmdlocal.o: ../hdrs/lock.h
cmdlocal.o: ../hdrs/dbdefs.h
cmdlocal.o: ../hdrs/extchat.h
cmdlocal.o: ../hdrs/myregexp.h
cmdlocal.o: ../confmagic.h
cmdlocal.o: ../hdrs/parse.h
cmdlocal.o: ../hdrs/htab.h
cmdlocal.o: ../hdrs/command.h
cmdlocal.o: ../hdrs/cmds.h
cmdlocal.o: ../hdrs/switches.h
cmds.o: ../hdrs/copyrite.h
cmds.o: ../config.h
cmds.o: ../hdrs/conf.h
cmds.o: ../options.h
cmds.o: ../hdrs/dbdefs.h
cmds.o: ../hdrs/attrib.h
cmds.o: ../hdrs/extchat.h
cmds.o: ../hdrs/mushdb.h
cmds.o: ../hdrs/flags.h
cmds.o: ../hdrs/lock.h
cmds.o: ../hdrs/externs.h
cmds.o: ../hdrs/intrface.h
cmds.o: ../hdrs/myregexp.h
cmds.o: ../confmagic.h
cmds.o: ../hdrs/match.h
cmds.o: ../hdrs/game.h
cmds.o: ../hdrs/extmail.h
cmds.o: ../hdrs/globals.h
cmds.o: ../hdrs/getpgsiz.h
cmds.o: ../hdrs/parse.h
cmds.o: ../hdrs/access.h
cmds.o: ../hdrs/version.h
cmds.o: ../hdrs/function.h
cmds.o: ../hdrs/funs.h
cmds.o: ../hdrs/command.h
cmds.o: ../hdrs/cmds.h
cmds.o: ../hdrs/switches.h
command.o: ../hdrs/copyrite.h
command.o: ../config.h
command.o: ../hdrs/conf.h
command.o: ../options.h
command.o: ../hdrs/dbdefs.h
command.o: ../hdrs/attrib.h
command.o: ../hdrs/extchat.h
command.o: ../hdrs/mushdb.h
command.o: ../hdrs/flags.h
command.o: ../hdrs/lock.h
command.o: ../hdrs/game.h
command.o: ../hdrs/extmail.h
command.o: ../hdrs/externs.h
command.o: ../hdrs/intrface.h
command.o: ../hdrs/myregexp.h
command.o: ../confmagic.h
command.o: ../hdrs/match.h
command.o: ../hdrs/globals.h
command.o: ../hdrs/getpgsiz.h
command.o: ../hdrs/parse.h
command.o: ../hdrs/access.h
command.o: ../hdrs/version.h
command.o: ../hdrs/htab.h
command.o: ../hdrs/function.h
command.o: ../hdrs/funs.h
command.o: ../hdrs/command.h
command.o: ../hdrs/cmds.h
command.o: ../hdrs/switches.h
command.o: ../hdrs/mymalloc.h
command.o: switchinc.c
compress.o: ../config.h
compress.o: ../hdrs/intrface.h
compress.o: ../hdrs/copyrite.h
compress.o: ../hdrs/conf.h
compress.o: ../options.h
compress.o: ../hdrs/mushdb.h
compress.o: ../hdrs/attrib.h
compress.o: ../hdrs/flags.h
compress.o: ../hdrs/lock.h
compress.o: ../hdrs/dbdefs.h
compress.o: ../hdrs/extchat.h
compress.o: comp_h.c
compress.o: ../hdrs/externs.h
compress.o: ../hdrs/myregexp.h
compress.o: ../confmagic.h
compress.o: ../hdrs/mymalloc.h
conf.o: ../hdrs/copyrite.h
conf.o: ../config.h
conf.o: ../hdrs/conf.h
conf.o: ../options.h
conf.o: ../hdrs/externs.h
conf.o: ../hdrs/intrface.h
conf.o: ../hdrs/mushdb.h
conf.o: ../hdrs/attrib.h
conf.o: ../hdrs/flags.h
conf.o: ../hdrs/lock.h
conf.o: ../hdrs/dbdefs.h
conf.o: ../hdrs/extchat.h
conf.o: ../hdrs/myregexp.h
conf.o: ../confmagic.h
conf.o: ../hdrs/pueblo.h
cque.o: ../hdrs/copyrite.h
cque.o: ../config.h
cque.o: ../hdrs/conf.h
cque.o: ../options.h
cque.o: ../hdrs/mushdb.h
cque.o: ../hdrs/attrib.h
cque.o: ../hdrs/flags.h
cque.o: ../hdrs/lock.h
cque.o: ../hdrs/dbdefs.h
cque.o: ../hdrs/extchat.h
cque.o: ../hdrs/intrface.h
cque.o: ../hdrs/match.h
cque.o: ../hdrs/externs.h
cque.o: ../hdrs/myregexp.h
cque.o: ../confmagic.h
cque.o: ../hdrs/parse.h
cque.o: ../hdrs/mymalloc.h
create.o: ../hdrs/copyrite.h
create.o: ../config.h
create.o: ../hdrs/conf.h
create.o: ../options.h
create.o: ../hdrs/mushdb.h
create.o: ../hdrs/attrib.h
create.o: ../hdrs/flags.h
create.o: ../hdrs/lock.h
create.o: ../hdrs/dbdefs.h
create.o: ../hdrs/extchat.h
create.o: ../hdrs/intrface.h
create.o: ../hdrs/externs.h
create.o: ../hdrs/myregexp.h
create.o: ../confmagic.h
create.o: ../hdrs/match.h
db.o: ../hdrs/copyrite.h
db.o: ../config.h
db.o: ../hdrs/conf.h
db.o: ../options.h
db.o: ../hdrs/intrface.h
db.o: ../hdrs/mushdb.h
db.o: ../hdrs/attrib.h
db.o: ../hdrs/flags.h
db.o: ../hdrs/lock.h
db.o: ../hdrs/dbdefs.h
db.o: ../hdrs/extchat.h
db.o: ../hdrs/externs.h
db.o: ../hdrs/myregexp.h
db.o: ../confmagic.h
db.o: ../hdrs/mymalloc.h
destroy.o: ../config.h
destroy.o: ../hdrs/copyrite.h
destroy.o: ../hdrs/conf.h
destroy.o: ../options.h
destroy.o: ../hdrs/mushdb.h
destroy.o: ../hdrs/attrib.h
destroy.o: ../hdrs/flags.h
destroy.o: ../hdrs/lock.h
destroy.o: ../hdrs/dbdefs.h
destroy.o: ../hdrs/extchat.h
destroy.o: ../hdrs/match.h
destroy.o: ../hdrs/externs.h
destroy.o: ../hdrs/intrface.h
destroy.o: ../hdrs/myregexp.h
destroy.o: ../confmagic.h
destroy.o: ../hdrs/globals.h
extchat.o: ../hdrs/copyrite.h
extchat.o: ../config.h
extchat.o: ../hdrs/conf.h
extchat.o: ../options.h
extchat.o: ../hdrs/mushdb.h
extchat.o: ../hdrs/attrib.h
extchat.o: ../hdrs/flags.h
extchat.o: ../hdrs/lock.h
extchat.o: ../hdrs/dbdefs.h
extchat.o: ../hdrs/extchat.h
extchat.o: ../hdrs/intrface.h
extchat.o: ../hdrs/match.h
extchat.o: ../hdrs/externs.h
extchat.o: ../hdrs/myregexp.h
extchat.o: ../confmagic.h
extchat.o: ../hdrs/ansi.h
extchat.o: ../hdrs/privtab.h
extchat.o: ../hdrs/mymalloc.h
extchat.o: ../hdrs/pueblo.h
extmail.o: ../config.h
extmail.o: ../hdrs/copyrite.h
extmail.o: ../hdrs/conf.h
extmail.o: ../options.h
extmail.o: ../hdrs/mushdb.h
extmail.o: ../hdrs/attrib.h
extmail.o: ../hdrs/flags.h
extmail.o: ../hdrs/lock.h
extmail.o: ../hdrs/dbdefs.h
extmail.o: ../hdrs/extchat.h
extmail.o: ../hdrs/intrface.h
extmail.o: ../hdrs/externs.h
extmail.o: ../hdrs/myregexp.h
extmail.o: ../confmagic.h
extmail.o: ../hdrs/match.h
extmail.o: ../hdrs/extmail.h
extmail.o: ../hdrs/parse.h
extmail.o: ../hdrs/mymalloc.h
extmail.o: ../hdrs/pueblo.h
flags.o: ../config.h
flags.o: ../hdrs/conf.h
flags.o: ../hdrs/copyrite.h
flags.o: ../options.h
flags.o: ../hdrs/mushdb.h
flags.o: ../hdrs/attrib.h
flags.o: ../hdrs/flags.h
flags.o: ../hdrs/lock.h
flags.o: ../hdrs/dbdefs.h
flags.o: ../hdrs/extchat.h
flags.o: ../hdrs/externs.h
flags.o: ../hdrs/intrface.h
flags.o: ../hdrs/myregexp.h
flags.o: ../confmagic.h
flags.o: ../hdrs/match.h
flags.o: ../hdrs/htab.h
function.o: ../hdrs/copyrite.h
function.o: ../config.h
function.o: ../hdrs/conf.h
function.o: ../options.h
function.o: ../hdrs/attrib.h
function.o: ../hdrs/dbdefs.h
function.o: ../hdrs/extchat.h
function.o: ../hdrs/externs.h
function.o: ../hdrs/intrface.h
function.o: ../hdrs/mushdb.h
function.o: ../hdrs/flags.h
function.o: ../hdrs/lock.h
function.o: ../hdrs/myregexp.h
function.o: ../confmagic.h
function.o: ../hdrs/function.h
function.o: ../hdrs/parse.h
function.o: ../hdrs/funs.h
function.o: ../hdrs/match.h
function.o: ../hdrs/htab.h
function.o: ../hdrs/mymalloc.h
fundb.o: ../hdrs/copyrite.h
fundb.o: ../config.h
fundb.o: ../hdrs/conf.h
fundb.o: ../options.h
fundb.o: ../hdrs/dbdefs.h
fundb.o: ../hdrs/attrib.h
fundb.o: ../hdrs/extchat.h
fundb.o: ../hdrs/externs.h
fundb.o: ../hdrs/intrface.h
fundb.o: ../hdrs/mushdb.h
fundb.o: ../hdrs/flags.h
fundb.o: ../hdrs/lock.h
fundb.o: ../hdrs/myregexp.h
fundb.o: ../confmagic.h
fundb.o: ../hdrs/match.h
fundb.o: ../hdrs/parse.h
fundb.o: ../hdrs/command.h
fundb.o: ../hdrs/cmds.h
fundb.o: ../hdrs/switches.h
funlist.o: ../hdrs/copyrite.h
funlist.o: ../config.h
funlist.o: ../hdrs/conf.h
funlist.o: ../options.h
funlist.o: ../hdrs/externs.h
funlist.o: ../hdrs/intrface.h
funlist.o: ../hdrs/mushdb.h
funlist.o: ../hdrs/attrib.h
funlist.o: ../hdrs/flags.h
funlist.o: ../hdrs/lock.h
funlist.o: ../hdrs/dbdefs.h
funlist.o: ../hdrs/extchat.h
funlist.o: ../hdrs/myregexp.h
funlist.o: ../confmagic.h
funlist.o: ../hdrs/parse.h
funlist.o: ../hdrs/mymalloc.h
funlocal.o: ../hdrs/copyrite.h
funlocal.o: ../config.h
funlocal.o: ../hdrs/externs.h
funlocal.o: ../hdrs/conf.h
funlocal.o: ../options.h
funlocal.o: ../hdrs/intrface.h
funlocal.o: ../hdrs/mushdb.h
funlocal.o: ../hdrs/attrib.h
funlocal.o: ../hdrs/flags.h
funlocal.o: ../hdrs/lock.h
funlocal.o: ../hdrs/dbdefs.h
funlocal.o: ../hdrs/extchat.h
funlocal.o: ../hdrs/myregexp.h
funlocal.o: ../confmagic.h
funlocal.o: ../hdrs/parse.h
funlocal.o: ../hdrs/function.h
funlocal.o: ../hdrs/funs.h
funmath.o: ../hdrs/copyrite.h
funmath.o: ../config.h
funmath.o: ../hdrs/conf.h
funmath.o: ../options.h
funmath.o: ../hdrs/externs.h
funmath.o: ../hdrs/intrface.h
funmath.o: ../hdrs/mushdb.h
funmath.o: ../hdrs/attrib.h
funmath.o: ../hdrs/flags.h
funmath.o: ../hdrs/lock.h
funmath.o: ../hdrs/dbdefs.h
funmath.o: ../hdrs/extchat.h
funmath.o: ../hdrs/myregexp.h
funmath.o: ../confmagic.h
funmath.o: ../hdrs/parse.h
funmisc.o: ../hdrs/copyrite.h
funmisc.o: ../config.h
funmisc.o: ../hdrs/conf.h
funmisc.o: ../options.h
funmisc.o: ../hdrs/externs.h
funmisc.o: ../hdrs/intrface.h
funmisc.o: ../hdrs/mushdb.h
funmisc.o: ../hdrs/attrib.h
funmisc.o: ../hdrs/flags.h
funmisc.o: ../hdrs/lock.h
funmisc.o: ../hdrs/dbdefs.h
funmisc.o: ../hdrs/extchat.h
funmisc.o: ../hdrs/myregexp.h
funmisc.o: ../confmagic.h
funmisc.o: ../hdrs/version.h
funmisc.o: ../hdrs/htab.h
funmisc.o: ../hdrs/parse.h
funmisc.o: ../hdrs/command.h
funmisc.o: ../hdrs/cmds.h
funmisc.o: ../hdrs/switches.h
funmisc.o: ../hdrs/function.h
funmisc.o: ../hdrs/funs.h
funmisc.o: funcrypt.c
funstr.o: ../hdrs/copyrite.h
funstr.o: ../config.h
funstr.o: ../hdrs/conf.h
funstr.o: ../options.h
funstr.o: ../hdrs/ansi.h
funstr.o: ../hdrs/externs.h
funstr.o: ../hdrs/intrface.h
funstr.o: ../hdrs/mushdb.h
funstr.o: ../hdrs/attrib.h
funstr.o: ../hdrs/flags.h
funstr.o: ../hdrs/lock.h
funstr.o: ../hdrs/dbdefs.h
funstr.o: ../hdrs/extchat.h
funstr.o: ../hdrs/myregexp.h
funstr.o: ../confmagic.h
funstr.o: ../hdrs/globals.h
funstr.o: ../hdrs/match.h
funstr.o: ../hdrs/parse.h
funstr.o: ../hdrs/pueblo.h
funtime.o: ../hdrs/copyrite.h
funtime.o: ../config.h
funtime.o: ../hdrs/conf.h
funtime.o: ../options.h
funtime.o: ../hdrs/externs.h
funtime.o: ../hdrs/intrface.h
funtime.o: ../hdrs/mushdb.h
funtime.o: ../hdrs/attrib.h
funtime.o: ../hdrs/flags.h
funtime.o: ../hdrs/lock.h
funtime.o: ../hdrs/dbdefs.h
funtime.o: ../hdrs/extchat.h
funtime.o: ../hdrs/myregexp.h
funtime.o: ../confmagic.h
funtime.o: ../hdrs/parse.h
funufun.o: ../hdrs/copyrite.h
funufun.o: ../config.h
funufun.o: ../hdrs/conf.h
funufun.o: ../options.h
funufun.o: ../hdrs/externs.h
funufun.o: ../hdrs/intrface.h
funufun.o: ../hdrs/mushdb.h
funufun.o: ../hdrs/attrib.h
funufun.o: ../hdrs/flags.h
funufun.o: ../hdrs/lock.h
funufun.o: ../hdrs/dbdefs.h
funufun.o: ../hdrs/extchat.h
funufun.o: ../hdrs/myregexp.h
funufun.o: ../confmagic.h
funufun.o: ../hdrs/match.h
funufun.o: ../hdrs/parse.h
funufun.o: ../hdrs/mymalloc.h
game.o: ../hdrs/copyrite.h
game.o: ../config.h
game.o: ../hdrs/conf.h
game.o: ../options.h
game.o: ../hdrs/mushdb.h
game.o: ../hdrs/attrib.h
game.o: ../hdrs/flags.h
game.o: ../hdrs/lock.h
game.o: ../hdrs/dbdefs.h
game.o: ../hdrs/extchat.h
game.o: ../hdrs/game.h
game.o: ../hdrs/extmail.h
game.o: ../hdrs/externs.h
game.o: ../hdrs/intrface.h
game.o: ../hdrs/myregexp.h
game.o: ../confmagic.h
game.o: ../hdrs/match.h
game.o: ../hdrs/globals.h
game.o: ../hdrs/getpgsiz.h
game.o: ../hdrs/parse.h
game.o: ../hdrs/access.h
game.o: ../hdrs/version.h
game.o: ../hdrs/command.h
game.o: ../hdrs/cmds.h
game.o: ../hdrs/switches.h
htab.o: ../config.h
htab.o: ../hdrs/copyrite.h
htab.o: ../hdrs/conf.h
htab.o: ../options.h
htab.o: ../hdrs/externs.h
htab.o: ../hdrs/intrface.h
htab.o: ../hdrs/mushdb.h
htab.o: ../hdrs/attrib.h
htab.o: ../hdrs/flags.h
htab.o: ../hdrs/lock.h
htab.o: ../hdrs/dbdefs.h
htab.o: ../hdrs/extchat.h
htab.o: ../hdrs/myregexp.h
htab.o: ../confmagic.h
htab.o: ../hdrs/htab.h
htab.o: ../hdrs/mymalloc.h
ident.o: ../config.h
ident.o: ../hdrs/conf.h
ident.o: ../hdrs/copyrite.h
ident.o: ../options.h
ident.o: ../hdrs/ident.h
ident.o: ../hdrs/mymalloc.h
ident.o: ../hdrs/externs.h
ident.o: ../hdrs/intrface.h
ident.o: ../hdrs/mushdb.h
ident.o: ../hdrs/attrib.h
ident.o: ../hdrs/flags.h
ident.o: ../hdrs/lock.h
ident.o: ../hdrs/dbdefs.h
ident.o: ../hdrs/extchat.h
ident.o: ../hdrs/myregexp.h
ident.o: ../confmagic.h
local.o: ../hdrs/copyrite.h
local.o: ../config.h
local.o: ../hdrs/externs.h
local.o: ../hdrs/conf.h
local.o: ../options.h
local.o: ../hdrs/intrface.h
local.o: ../hdrs/mushdb.h
local.o: ../hdrs/attrib.h
local.o: ../hdrs/flags.h
local.o: ../hdrs/lock.h
local.o: ../hdrs/dbdefs.h
local.o: ../hdrs/extchat.h
local.o: ../hdrs/myregexp.h
local.o: ../confmagic.h
local.o: ../hdrs/parse.h
local.o: ../hdrs/htab.h
local.o: ../hdrs/command.h
local.o: ../hdrs/cmds.h
local.o: ../hdrs/switches.h
lock.o: ../hdrs/copyrite.h
lock.o: ../config.h
lock.o: ../hdrs/conf.h
lock.o: ../options.h
lock.o: ../hdrs/mushdb.h
lock.o: ../hdrs/attrib.h
lock.o: ../hdrs/flags.h
lock.o: ../hdrs/lock.h
lock.o: ../hdrs/dbdefs.h
lock.o: ../hdrs/extchat.h
lock.o: ../hdrs/externs.h
lock.o: ../hdrs/intrface.h
lock.o: ../hdrs/myregexp.h
lock.o: ../confmagic.h
lock.o: ../hdrs/match.h
lock.o: ../hdrs/mymalloc.h
log.o: ../hdrs/copyrite.h
log.o: ../config.h
log.o: ../hdrs/conf.h
log.o: ../options.h
log.o: ../hdrs/externs.h
log.o: ../hdrs/intrface.h
log.o: ../hdrs/mushdb.h
log.o: ../hdrs/attrib.h
log.o: ../hdrs/flags.h
log.o: ../hdrs/lock.h
log.o: ../hdrs/dbdefs.h
log.o: ../hdrs/extchat.h
log.o: ../hdrs/myregexp.h
log.o: ../confmagic.h
look.o: ../config.h
look.o: ../hdrs/copyrite.h
look.o: ../hdrs/conf.h
look.o: ../options.h
look.o: ../hdrs/mushdb.h
look.o: ../hdrs/attrib.h
look.o: ../hdrs/flags.h
look.o: ../hdrs/lock.h
look.o: ../hdrs/dbdefs.h
look.o: ../hdrs/extchat.h
look.o: ../hdrs/intrface.h
look.o: ../hdrs/match.h
look.o: ../hdrs/externs.h
look.o: ../hdrs/myregexp.h
look.o: ../confmagic.h
look.o: ../hdrs/ansi.h
look.o: ../hdrs/pueblo.h
look.o: ../hdrs/parse.h
look.o: ../hdrs/privtab.h
mycrypt.o: ../config.h
mycrypt.o: ../hdrs/conf.h
mycrypt.o: ../hdrs/copyrite.h
mycrypt.o: ../options.h
mycrypt.o: ../confmagic.h
mymalloc.o: ../config.h
mymalloc.o: ../options.h
mymalloc.o: ../confmagic.h
mymalloc.o: csrimalloc.c
mymalloc.o: ../hdrs/conf.h
mymalloc.o: ../hdrs/copyrite.h
mymalloc.o: ../hdrs/mymalloc.h
mymalloc.o: ../hdrs/csrimalloc.h
match.o: ../hdrs/copyrite.h
match.o: ../config.h
match.o: ../hdrs/conf.h
match.o: ../options.h
match.o: ../hdrs/mushdb.h
match.o: ../hdrs/attrib.h
match.o: ../hdrs/flags.h
match.o: ../hdrs/lock.h
match.o: ../hdrs/dbdefs.h
match.o: ../hdrs/extchat.h
match.o: ../hdrs/externs.h
match.o: ../hdrs/intrface.h
match.o: ../hdrs/myregexp.h
match.o: ../confmagic.h
match.o: ../hdrs/globals.h
match.o: ../hdrs/match.h
memcheck.o: ../config.h
memcheck.o: ../hdrs/conf.h
memcheck.o: ../hdrs/copyrite.h
memcheck.o: ../options.h
move.o: ../hdrs/copyrite.h
move.o: ../config.h
move.o: ../hdrs/conf.h
move.o: ../options.h
move.o: ../hdrs/mushdb.h
move.o: ../hdrs/attrib.h
move.o: ../hdrs/flags.h
move.o: ../hdrs/lock.h
move.o: ../hdrs/dbdefs.h
move.o: ../hdrs/extchat.h
move.o: ../hdrs/intrface.h
move.o: ../hdrs/match.h
move.o: ../hdrs/externs.h
move.o: ../hdrs/myregexp.h
move.o: ../confmagic.h
move.o: ../hdrs/globals.h
move.o: ../hdrs/parse.h
move.o: ../hdrs/command.h
move.o: ../hdrs/cmds.h
move.o: ../hdrs/switches.h
parse.o: ../hdrs/copyrite.h
parse.o: ../config.h
parse.o: ../hdrs/conf.h
parse.o: ../options.h
parse.o: ../hdrs/ansi.h
parse.o: ../hdrs/dbdefs.h
parse.o: ../hdrs/attrib.h
parse.o: ../hdrs/extchat.h
parse.o: ../hdrs/externs.h
parse.o: ../hdrs/intrface.h
parse.o: ../hdrs/mushdb.h
parse.o: ../hdrs/flags.h
parse.o: ../hdrs/lock.h
parse.o: ../hdrs/myregexp.h
parse.o: ../confmagic.h
parse.o: ../hdrs/function.h
parse.o: ../hdrs/parse.h
parse.o: ../hdrs/funs.h
parse.o: ../hdrs/globals.h
parse.o: ../hdrs/match.h
parse.o: ../hdrs/mymalloc.h
player.o: ../hdrs/copyrite.h
player.o: ../config.h
player.o: ../hdrs/conf.h
player.o: ../options.h
player.o: ../hdrs/mushdb.h
player.o: ../hdrs/attrib.h
player.o: ../hdrs/flags.h
player.o: ../hdrs/lock.h
player.o: ../hdrs/dbdefs.h
player.o: ../hdrs/extchat.h
player.o: ../hdrs/intrface.h
player.o: ../hdrs/externs.h
player.o: ../hdrs/myregexp.h
player.o: ../confmagic.h
player.o: ../hdrs/access.h
plyrlist.o: ../config.h
plyrlist.o: ../hdrs/copyrite.h
plyrlist.o: ../hdrs/conf.h
plyrlist.o: ../options.h
plyrlist.o: ../hdrs/mushdb.h
plyrlist.o: ../hdrs/attrib.h
plyrlist.o: ../hdrs/flags.h
plyrlist.o: ../hdrs/lock.h
plyrlist.o: ../hdrs/dbdefs.h
plyrlist.o: ../hdrs/extchat.h
plyrlist.o: ../hdrs/intrface.h
plyrlist.o: ../hdrs/globals.h
plyrlist.o: ../hdrs/externs.h
plyrlist.o: ../hdrs/myregexp.h
plyrlist.o: ../confmagic.h
plyrlist.o: ../hdrs/htab.h
predicat.o: ../hdrs/copyrite.h
predicat.o: ../config.h
predicat.o: ../hdrs/conf.h
predicat.o: ../options.h
predicat.o: ../hdrs/externs.h
predicat.o: ../hdrs/intrface.h
predicat.o: ../hdrs/mushdb.h
predicat.o: ../hdrs/attrib.h
predicat.o: ../hdrs/flags.h
predicat.o: ../hdrs/lock.h
predicat.o: ../hdrs/dbdefs.h
predicat.o: ../hdrs/extchat.h
predicat.o: ../hdrs/myregexp.h
predicat.o: ../confmagic.h
predicat.o: ../hdrs/globals.h
predicat.o: ../hdrs/match.h
predicat.o: ../hdrs/ansi.h
predicat.o: ../hdrs/parse.h
predicat.o: ../hdrs/mymalloc.h
privtab.o: ../hdrs/copyrite.h
privtab.o: ../config.h
privtab.o: ../hdrs/conf.h
privtab.o: ../options.h
privtab.o: ../hdrs/privtab.h
privtab.o: ../confmagic.h
privtab.o: ../hdrs/externs.h
privtab.o: ../hdrs/intrface.h
privtab.o: ../hdrs/mushdb.h
privtab.o: ../hdrs/attrib.h
privtab.o: ../hdrs/flags.h
privtab.o: ../hdrs/lock.h
privtab.o: ../hdrs/dbdefs.h
privtab.o: ../hdrs/extchat.h
privtab.o: ../hdrs/myregexp.h
regexp.o: ../config.h
regexp.o: ../hdrs/conf.h
regexp.o: ../hdrs/copyrite.h
regexp.o: ../options.h
regexp.o: ../hdrs/myregexp.h
regexp.o: ../hdrs/intrface.h
regexp.o: ../hdrs/mushdb.h
regexp.o: ../hdrs/attrib.h
regexp.o: ../hdrs/flags.h
regexp.o: ../hdrs/lock.h
regexp.o: ../hdrs/dbdefs.h
regexp.o: ../hdrs/extchat.h
regexp.o: ../hdrs/externs.h
regexp.o: ../confmagic.h
rob.o: ../config.h
rob.o: ../hdrs/copyrite.h
rob.o: ../hdrs/conf.h
rob.o: ../options.h
rob.o: ../hdrs/mushdb.h
rob.o: ../hdrs/attrib.h
rob.o: ../hdrs/flags.h
rob.o: ../hdrs/lock.h
rob.o: ../hdrs/dbdefs.h
rob.o: ../hdrs/extchat.h
rob.o: ../hdrs/intrface.h
rob.o: ../hdrs/match.h
rob.o: ../hdrs/externs.h
rob.o: ../hdrs/myregexp.h
rob.o: ../confmagic.h
rwho.o: ../config.h
rwho.o: ../hdrs/conf.h
rwho.o: ../hdrs/copyrite.h
rwho.o: ../options.h
rwho.o: ../hdrs/mymalloc.h
rwho.o: ../confmagic.h
set.o: ../hdrs/copyrite.h
set.o: ../config.h
set.o: ../hdrs/conf.h
set.o: ../options.h
set.o: ../hdrs/mushdb.h
set.o: ../hdrs/attrib.h
set.o: ../hdrs/flags.h
set.o: ../hdrs/lock.h
set.o: ../hdrs/dbdefs.h
set.o: ../hdrs/extchat.h
set.o: ../hdrs/match.h
set.o: ../hdrs/intrface.h
set.o: ../hdrs/externs.h
set.o: ../hdrs/myregexp.h
set.o: ../confmagic.h
set.o: ../hdrs/ansi.h
set.o: ../hdrs/command.h
set.o: ../hdrs/cmds.h
set.o: ../hdrs/switches.h
set.o: ../hdrs/mymalloc.h
speech.o: ../hdrs/copyrite.h
speech.o: ../config.h
speech.o: ../hdrs/conf.h
speech.o: ../options.h
speech.o: ../hdrs/externs.h
speech.o: ../hdrs/intrface.h
speech.o: ../hdrs/mushdb.h
speech.o: ../hdrs/attrib.h
speech.o: ../hdrs/flags.h
speech.o: ../hdrs/lock.h
speech.o: ../hdrs/dbdefs.h
speech.o: ../hdrs/extchat.h
speech.o: ../hdrs/myregexp.h
speech.o: ../confmagic.h
speech.o: ../hdrs/match.h
speech.o: ../hdrs/parse.h
strdup.o: ../config.h
strdup.o: ../hdrs/conf.h
strdup.o: ../hdrs/copyrite.h
strdup.o: ../options.h
strdup.o: ../hdrs/mymalloc.h
strdup.o: ../confmagic.h
strutil.o: ../config.h
strutil.o: ../hdrs/copyrite.h
strutil.o: ../hdrs/conf.h
strutil.o: ../options.h
strutil.o: ../hdrs/intrface.h
strutil.o: ../hdrs/mushdb.h
strutil.o: ../hdrs/attrib.h
strutil.o: ../hdrs/flags.h
strutil.o: ../hdrs/lock.h
strutil.o: ../hdrs/dbdefs.h
strutil.o: ../hdrs/extchat.h
strutil.o: ../hdrs/globals.h
strutil.o: ../hdrs/ansi.h
strutil.o: ../hdrs/pueblo.h
strutil.o: ../hdrs/mymalloc.h
strutil.o: ../confmagic.h
timer.o: ../hdrs/copyrite.h
timer.o: ../config.h
timer.o: ../hdrs/conf.h
timer.o: ../options.h
timer.o: ../hdrs/mushdb.h
timer.o: ../hdrs/attrib.h
timer.o: ../hdrs/flags.h
timer.o: ../hdrs/lock.h
timer.o: ../hdrs/dbdefs.h
timer.o: ../hdrs/extchat.h
timer.o: ../hdrs/intrface.h
timer.o: ../hdrs/match.h
timer.o: ../hdrs/externs.h
timer.o: ../hdrs/myregexp.h
timer.o: ../confmagic.h
timer.o: ../hdrs/access.h
unparse.o: ../hdrs/copyrite.h
unparse.o: ../config.h
unparse.o: ../hdrs/conf.h
unparse.o: ../options.h
unparse.o: ../hdrs/mushdb.h
unparse.o: ../hdrs/attrib.h
unparse.o: ../hdrs/flags.h
unparse.o: ../hdrs/lock.h
unparse.o: ../hdrs/dbdefs.h
unparse.o: ../hdrs/extchat.h
unparse.o: ../hdrs/externs.h
unparse.o: ../hdrs/intrface.h
unparse.o: ../hdrs/myregexp.h
unparse.o: ../confmagic.h
unparse.o: ../hdrs/ansi.h
unparse.o: ../hdrs/pueblo.h
unparse.o: ../hdrs/parse.h
utils.o: ../hdrs/copyrite.h
utils.o: ../config.h
utils.o: ../hdrs/conf.h
utils.o: ../options.h
utils.o: ../hdrs/intrface.h
utils.o: ../hdrs/mushdb.h
utils.o: ../hdrs/attrib.h
utils.o: ../hdrs/flags.h
utils.o: ../hdrs/lock.h
utils.o: ../hdrs/dbdefs.h
utils.o: ../hdrs/extchat.h
utils.o: ../hdrs/match.h
utils.o: ../hdrs/externs.h
utils.o: ../hdrs/myregexp.h
utils.o: ../confmagic.h
utils.o: ../hdrs/mymalloc.h
version.o: ../config.h
version.o: ../hdrs/copyrite.h
version.o: ../hdrs/conf.h
version.o: ../options.h
version.o: ../hdrs/intrface.h
version.o: ../hdrs/mushdb.h
version.o: ../hdrs/attrib.h
version.o: ../hdrs/flags.h
version.o: ../hdrs/lock.h
version.o: ../hdrs/dbdefs.h
version.o: ../hdrs/extchat.h
version.o: ../hdrs/externs.h
version.o: ../hdrs/myregexp.h
version.o: ../confmagic.h
version.o: ../hdrs/version.h
version.o: ../hdrs/buildinf.h
warnings.o: ../config.h
warnings.o: ../hdrs/copyrite.h
warnings.o: ../hdrs/conf.h
warnings.o: ../options.h
warnings.o: ../hdrs/mushdb.h
warnings.o: ../hdrs/attrib.h
warnings.o: ../hdrs/flags.h
warnings.o: ../hdrs/lock.h
warnings.o: ../hdrs/dbdefs.h
warnings.o: ../hdrs/extchat.h
warnings.o: ../hdrs/intrface.h
warnings.o: ../hdrs/externs.h
warnings.o: ../hdrs/myregexp.h
warnings.o: ../confmagic.h
warnings.o: ../hdrs/match.h
warnings.o: ../hdrs/warnings.h
wild.o: ../config.h
wild.o: ../hdrs/copyrite.h
wild.o: ../hdrs/conf.h
wild.o: ../options.h
wild.o: ../hdrs/intrface.h
wild.o: ../hdrs/mushdb.h
wild.o: ../hdrs/attrib.h
wild.o: ../hdrs/flags.h
wild.o: ../hdrs/lock.h
wild.o: ../hdrs/dbdefs.h
wild.o: ../hdrs/extchat.h
wild.o: ../hdrs/globals.h
wild.o: ../hdrs/externs.h
wild.o: ../hdrs/myregexp.h
wild.o: ../confmagic.h
wild.o: ../hdrs/mymalloc.h
wiz.o: ../hdrs/copyrite.h
wiz.o: ../config.h
wiz.o: ../hdrs/conf.h
wiz.o: ../options.h
wiz.o: ../hdrs/mushdb.h
wiz.o: ../hdrs/attrib.h
wiz.o: ../hdrs/flags.h
wiz.o: ../hdrs/lock.h
wiz.o: ../hdrs/dbdefs.h
wiz.o: ../hdrs/extchat.h
wiz.o: ../hdrs/intrface.h
wiz.o: ../hdrs/match.h
wiz.o: ../hdrs/externs.h
wiz.o: ../hdrs/myregexp.h
wiz.o: ../confmagic.h
wiz.o: ../hdrs/access.h
wiz.o: ../hdrs/parse.h
wiz.o: ../hdrs/mymalloc.h
../hdrs/conf.o: ../hdrs/copyrite.h
../hdrs/conf.o: ../options.h
../hdrs/mushdb.o: ../hdrs/copyrite.h
../hdrs/mushdb.o: ../hdrs/attrib.h
../hdrs/mushdb.o: ../hdrs/flags.h
../hdrs/mushdb.o: ../hdrs/conf.h
../hdrs/mushdb.o: ../options.h
../hdrs/mushdb.o: ../hdrs/lock.h
../hdrs/mushdb.o: ../hdrs/dbdefs.h
../hdrs/mushdb.o: ../hdrs/extchat.h
../hdrs/flags.o: ../hdrs/conf.h
../hdrs/flags.o: ../hdrs/copyrite.h
../hdrs/flags.o: ../options.h
../hdrs/externs.o: ../config.h
../hdrs/externs.o: ../hdrs/copyrite.h
../hdrs/externs.o: ../hdrs/conf.h
../hdrs/externs.o: ../options.h
../hdrs/externs.o: ../hdrs/intrface.h
../hdrs/externs.o: ../hdrs/mushdb.h
../hdrs/externs.o: ../hdrs/attrib.h
../hdrs/externs.o: ../hdrs/flags.h
../hdrs/externs.o: ../hdrs/lock.h
../hdrs/externs.o: ../hdrs/dbdefs.h
../hdrs/externs.o: ../hdrs/extchat.h
../hdrs/externs.o: ../hdrs/myregexp.h
../hdrs/externs.o: ../confmagic.h
../hdrs/intrface.o: ../hdrs/copyrite.h
../hdrs/intrface.o: ../hdrs/conf.h
../hdrs/intrface.o: ../options.h
../hdrs/intrface.o: ../hdrs/mushdb.h
../hdrs/intrface.o: ../hdrs/attrib.h
../hdrs/intrface.o: ../hdrs/flags.h
../hdrs/intrface.o: ../hdrs/lock.h
../hdrs/intrface.o: ../hdrs/dbdefs.h
../hdrs/intrface.o: ../hdrs/extchat.h
../hdrs/match.o: ../hdrs/copyrite.h
../hdrs/match.o: ../hdrs/mushdb.h
../hdrs/match.o: ../hdrs/attrib.h
../hdrs/match.o: ../hdrs/flags.h
../hdrs/match.o: ../hdrs/conf.h
../hdrs/match.o: ../options.h
../hdrs/match.o: ../hdrs/lock.h
../hdrs/match.o: ../hdrs/dbdefs.h
../hdrs/match.o: ../hdrs/extchat.h
../hdrs/lock.o: ../hdrs/copyrite.h
../hdrs/lock.o: ../hdrs/conf.h
../hdrs/lock.o: ../options.h
../hdrs/lock.o: ../hdrs/dbdefs.h
../hdrs/lock.o: ../hdrs/attrib.h
../hdrs/lock.o: ../hdrs/extchat.h
../hdrs/mymalloc.o: ../options.h
../hdrs/ident.o: ../config.h
../hdrs/game.o: ../hdrs/conf.h
../hdrs/game.o: ../hdrs/copyrite.h
../hdrs/game.o: ../options.h
../hdrs/game.o: ../hdrs/dbdefs.h
../hdrs/game.o: ../hdrs/attrib.h
../hdrs/game.o: ../hdrs/extchat.h
../hdrs/game.o: ../hdrs/extmail.h
../hdrs/atr_tab.o: ../hdrs/attrib.h
../hdrs/dbdefs.o: ../hdrs/attrib.h
../hdrs/dbdefs.o: ../options.h
../hdrs/dbdefs.o: ../hdrs/extchat.h
../hdrs/function.o: ../hdrs/copyrite.h
../hdrs/function.o: ../hdrs/attrib.h
../hdrs/function.o: ../hdrs/dbdefs.h
../hdrs/function.o: ../options.h
../hdrs/function.o: ../hdrs/extchat.h
../hdrs/function.o: ../hdrs/parse.h
../hdrs/function.o: ../config.h
../hdrs/function.o: ../confmagic.h
../hdrs/function.o: ../hdrs/funs.h
../hdrs/parse.o: ../hdrs/copyrite.h
../hdrs/parse.o: ../config.h
../hdrs/parse.o: ../hdrs/dbdefs.h
../hdrs/parse.o: ../hdrs/attrib.h
../hdrs/parse.o: ../options.h
../hdrs/parse.o: ../hdrs/extchat.h
../hdrs/parse.o: ../confmagic.h
../hdrs/htab.o: ../config.h
../hdrs/htab.o: ../hdrs/mushdb.h
../hdrs/htab.o: ../hdrs/copyrite.h
../hdrs/htab.o: ../hdrs/attrib.h
../hdrs/htab.o: ../hdrs/flags.h
../hdrs/htab.o: ../hdrs/conf.h
../hdrs/htab.o: ../options.h
../hdrs/htab.o: ../hdrs/lock.h
../hdrs/htab.o: ../hdrs/dbdefs.h
../hdrs/htab.o: ../hdrs/extchat.h
../hdrs/htab.o: ../confmagic.h
../hdrs/command.o: ../config.h
../hdrs/command.o: ../confmagic.h
../hdrs/command.o: ../hdrs/cmds.h
../hdrs/command.o: ../hdrs/switches.h
../hdrs/htab.o: ../config.h
../hdrs/htab.o: ../hdrs/mushdb.h
../hdrs/htab.o: ../hdrs/copyrite.h
../hdrs/htab.o: ../hdrs/attrib.h
../hdrs/htab.o: ../hdrs/flags.h
../hdrs/htab.o: ../hdrs/conf.h
../hdrs/htab.o: ../options.h
../hdrs/htab.o: ../hdrs/lock.h
../hdrs/htab.o: ../hdrs/dbdefs.h
../hdrs/htab.o: ../hdrs/extchat.h
../hdrs/htab.o: ../confmagic.h
../hdrs/privtab.o: ../hdrs/copyrite.h
../hdrs/privtab.o: ../config.h
../hdrs/privtab.o: ../confmagic.h
magazine.o: ../hdrs/magazine.h
space_alert.o: ../hdrs/space.h
space_ansi.o: ../hdrs/space.h
space_balance.o: ../hdrs/space.h
space_error.o: ../hdrs/space.h
space_format.o: ../hdrs/space.h
space_function.o: ../hdrs/space.h
space_get.o: ../hdrs/space.h
space_iterate.o: ../hdrs/space.h
space_misc.o: ../hdrs/space.h
space_notify.o: ../hdrs/space.h
space_put.o: ../hdrs/space.h
space_read.o: ../hdrs/space.h
space_report.o: ../hdrs/space.h
space_set.o: ../hdrs/space.h
space_status.o: ../hdrs/space.h
space_unparse.o: ../hdrs/space.h
space_utils.o: ../hdrs/space.h
space_variables.o: ../hdrs/space.h
space_write.o: ../hdrs/space.h
