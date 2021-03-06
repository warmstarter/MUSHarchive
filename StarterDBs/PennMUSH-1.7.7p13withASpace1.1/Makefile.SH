case $CONFIG in
'')
	if test -f config.sh; then TOP=.;
	elif test -f ../config.sh; then TOP=..;
	elif test -f ../../config.sh; then TOP=../..;
	elif test -f ../../../config.sh; then TOP=../../..;
	elif test -f ../../../../config.sh; then TOP=../../../..;
	else
		echo "Can't find config.sh."; exit 1
	fi
	. $TOP/config.sh
	;;
esac
: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac

echo "Extracting Makefile (with variable substitutions)"

: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.
$spitshell >Makefile <<!GROK!THIS!
# Makefile for PennMUSH 

# - System configuration - #

#
# This section of the file should be automatically configured by
# the Configure script. If it doesn't work, you might try starting
# from the Makefile.old that's included instead, and reporting
# your problem (including this Makefile) to Paul/Javelin,
# dunemush@pennmush.org
#
$make_set_make
CC=$cc
CCFLAGS=$optimize -I.. -I../hdrs $ccflags
LDFLAGS=$ldflags
CLIBS=$libs $cryptlib
LNS=$lns
INSTALL=$install
INSTALLDIR=$installdir
CP=$cp
CHMOD=$chmod
!GROK!THIS!

: In the following dollars and backticks do not need the extra backslash.
$spitshell >>Makefile <<'!NO!SUBS!'


# stupid SYS V shell
SHELL=/bin/sh
# Where to install with 'make globalinstall'
GLOBAL_INSTALL=/usr/libexec/pennmush

all: config.h options.h hdrs/cmds.h hdrs/funs.h hdrs/switches.h src/switchinc.c game/mush.cnf
	@echo "Making all in src."
	(cd src; make all "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" "MAKE=$(MAKE)" \
	"MAKEFLAGS=$(MAKEFLAGS)")
	@echo "If the make was successful, use 'make install' to install links."

config.h: Configure
	@echo "Looks like your Configure has been updated."
	@echo "Run that first."
	exit 1

options.h: options.h.dist
	@echo "Please use 'make update' to update your options.h file from options.h.dist"
	@echo "You must cp options.h.dist to options.h and edit it."
	exit 1

hdrs/cmds.h: src/cmds.c src/command.c src/cque.c src/help.c src/set.c Patchlevel
	(cd utils; sh mkcmds.sh commands)

hdrs/switches.h: src/SWITCHES Patchlevel
	(cd utils; sh mkcmds.sh switches)

#src/switchinc.c: hdrs/switches.h
#	(cd utils; sh mkcmds.sh switches)

hdrs/funs.h: src/fun*.c src/bsd.c src/conf.c src/extmail.c src/help.c src/wiz.c Patchlevel
	(cd utils; sh mkcmds.sh functions)

hdrs/patches.h: patches/*
	(cd utils; sh mkcmds.sh patches)

install: localized all
	-rm -f game/netmush
	-rm -f game/info_slave
	(cd game; \
	$(LNS) ../src/netmud netmush; \
	$(LNS) ../src/info_slave info_slave)
	(cd game/txt; make)
	@echo "If you plan to run multiple MUSHes, consider running 'make customize'"

netmud: 
	(cd src; make netmud "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

access:
	utils/make_access_cnf.sh game

pennmush.pot:
	(cd src; make ../po/pennmush.pot)

localized:
	-echo "Localizing for your locale..."
	-(cd po; make localized)

portmsg:
	(cd src; make portmsg "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

announce:
	(cd src; make announce "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

versions: CHANGES*
	-rm -rf CHANGES*~ CHANGES*bak
	utils/mkvershlp.pl game/txt/hlp CHANGES*

safety:
	$(CP) src/*.c /var/pennmush-bak/src
	$(CP) hdrs/*.h /var/pennmush-bak/hdrs
	$(CP) * /var/pennmush-bak

distdepend: hdrs/funs.h hdrs/cmds.h
	(cd src; make depend "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

# REQUIRES GNU INDENT! DON'T INDENT WITH ANYTHING ELSE!
indent:
	(cd utils; sh mkcmds.sh)
	(cd src; make indent)

protoize:
	(cd src; make protoize "CCFLAGS=$(CCFLAGS)")

!NO!SUBS!

: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.
$spitshell >>Makefile <<!GROK!THIS!

customize: update-conf
	-@$perl utils/customize.pl

update-conf: game/mushcnf.dst
	-@$touch game/mushcnf.dst
	-@$perl utils/update-cnf.pl game/mush.cnf game/mushcnf.dst

game/mush.cnf: game/mushcnf.dst
	-@$touch game/mushcnf.dst
	-@$perl utils/update-cnf.pl game/mush.cnf game/mushcnf.dst

update: update-hdr update-conf

update-hdr:
	-@$touch options.h.dist
	-@$perl utils/update.pl options.h options.h.dist

!GROK!THIS!

: In the following dollars and backticks do not need the extra backslash.
$spitshell >>Makefile <<'!NO!SUBS!'

clean:
	(cd src; make clean)
	(cd game; rm -f netmush info_slave)

distclean: 
	(cd src; make distclean)
	(cd hdrs; rm -f *.orig *~ \#* *.rej *.bak funs.h cmds.h)
	(cd utils; rm -f *.orig *~ \#* *.rej *.bak)
	(cd game; rm -rf *.log netmush info_slave *.orig *.rej *~ *.bak mush.cnf)
	(cd os2; rm -rf *.rej *.orig *~ *.bak)
	(cd game/txt; make clean)

totallyclean: distclean 
	(cd hdrs; rm -rf *.rej)
	(cd src; rm -rf *.rej)
	-rm -f Makefile

distci: distclean ci-src ci-game

ci-src:
	-(yes . | ci -l -f -N$(NAME) FAQ* BUGS COPY* CHANGE* READ* MANIFEST \
	  Configure utils/* Makefile.SH Patchlevel config_h.SH confmagic.h \
	  *.dist src/Makefile src/SWITCHES src/*.c src/*.dst \
	  hdrs/* hints/* os2/*)

ci-game:
	-(yes . | ci -l -f -N$(NAME) game/restart game/mushcnf.dst \
	  game/access.README \
	  game/txt/* game/txt/nws/* game/txt/evt/* game/txt/hlp/* )

diffs: versions
	@make indent > /dev/null 2>&1
	@(cd utils; sh mkcmds.sh) > /dev/null 2>&1
	@(prcs diff -r$(VS) -N pennmush `cat MANIFEST` | grep -v 'Index:')

patch: versions
	@make-patch-header
	@make diffs

etags: 
	(cd src; make etags)

ctags:
	(cd src; make ctags)

dist.tar.Z: distclean dist.tar
	compress /tmp/dist.tar

dist.tar.gz: distclean dist.tar
	gzip /tmp/dist.tar

dist.tar: distclean versions
	(cd utils; sh mkcmds.sh)
	makedist -c pennmush
	tar -cvf /tmp/dist.tar pennmush
	-pgp -sb /tmp/dist.tar
	-rm -rf pennmush

CSRImalloc.tar.Z:
	(cd src/CSRI; make clean)
	(tar -cvFFf /tmp/CSRImalloc.tar `cat exclude.tar`)
	compress /tmp/CSRImalloc.tar

globalinstall: install
	(cd game/txt; make clean compose.sh)
	$(INSTALLDIR) $(GLOBAL_INSTALL)
	$(CP) -R game/* $(GLOBAL_INSTALL)
	rm -f $(GLOBAL_INSTALL)/netmush $(GLOBAL_INSTALL)/info_slave
	$(INSTALL) config.sh $(GLOBAL_INSTALL)/config.sh
	$(INSTALL) src/netmud $(GLOBAL_INSTALL)/netmush
	$(INSTALL) src/info_slave utils/ln-dir.sh $(GLOBAL_INSTALL)
	$(CHMOD) a+rX -R $(GLOBAL_INSTALL)
	@echo "** Files installed in $(GLOBAL_INSTALL). Feel free to move them."
	@echo "** You can run $(GLOBAL_INSTALL)/ln-dir.sh to create a user directory,"
	@echo "** or symlink that to somewhere easier to run. You may wish to strip them."

!NO!SUBS!
chmod 755 Makefile
