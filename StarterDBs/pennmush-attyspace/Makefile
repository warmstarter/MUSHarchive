# Makefile for PennMUSH 

# - System configuration - #

#
# This section of the file should be automatically configured by
# the Configure script. If it doesn't work, you might try starting
# from the Makefile.old that's included instead, and reporting
# your problem (including this Makefile) to Paul/Javelin,
# dunemush@pennmush.org
#

CC=gcc
CCFLAGS=-g -O2 -ffast-math -I.. -I../hdrs
LDFLAGS= -L/usr/local/lib -L/usr/lib
CLIBS=-lm -lc -lcrypt  
LNS=/bin/ln -s


# stupid SYS V shell
SHELL=/bin/sh


all: config.h options.h hdrs/cmds.h hdrs/funs.h game/mush.cnf
	@echo "Making all in src."
	(cd src; make all "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )
	@echo "If the make was successful, use 'make install' to install links"
	@echo "and update text file indexes."

config.h: Configure
	@echo "Looks like your Configure has been updated."
	@echo "Run that first."
	exit 1

options.h: options.h.dist
	@echo "Please use 'make update' to update your options.h file from options.h.dist"
	@echo "You must cp options.h.dist to options.h and edit it."
	exit 1

hdrs/cmds.h: src/SWITCHES src/cmds.c
	(cd utils; sh mkcmds.sh)

hdrs/funs.h: src/fun*.c
	(cd utils; sh mkcmds.sh)

install: all
	-rm -f game/netmush
	-rm -f game/mkindx
	-rm -f game/info_slave
	(cd game; \
	$(LNS) ../src/netmud netmush; \
	$(LNS) ../src/mkindx mkindx; \
	$(LNS) ../src/info_slave info_slave)
	(cd game/txt; make)
	@echo "If you plan to run multiple MUSHes, consider running 'make customize'"

netmud: 
	(cd src; make netmud "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

mkindx:
	(cd src; make mkindx "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

access:
	utils/make_access_cnf.sh game

portmsg:
	(cd src; make portmsg "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

concentrate:
	(cd src; make concentrate "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

install_conc: concentrate
	-rm -f game/concentrate
	(cd game; $(LNS) ../src/concentrate concentrate)

announce:
	(cd src; make announce "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

rwho_made:
	(cd src/RWHO; make CC="$(CC)" CCFLAGS="$(CCFLAGS)")
	(cd src; touch rwho_made)

ident_made:
	(cd src/IDENT; make CC="$(CC)" CCFLAGS="$(CCFLAGS)")
	(cd src; touch ident_made)

dump:
	(cd src; make dump "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )
 
extract:
	(cd src; make extract "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

decompress:
	(cd src; make decompress "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

safety:
	cp src/*.c /var/pennmush-bak/src
	cp hdrs/*.h /var/pennmush-bak/hdrs
	cp * /var/pennmush-bak

distdepend: hdrs/funs.h hdrs/cmds.h
	(cd src; make depend "CC=$(CC)" "CCFLAGS=$(CCFLAGS)" \
	"LDFLAGS=$(LDFLAGS)" "CLIBS=$(CLIBS)" )

# REQUIRES GNU INDENT! DON'T INDENT WITH ANYTHING ELSE!
indent:
	(cd src; make indent)

protoize:
	(cd src; make protoize "CCFLAGS=$(CCFLAGS)")


customize: update-conf
	-@/usr/bin/perl utils/customize.pl

update-conf: game/mushcnf.dst
	-@/usr/bin/touch game/mushcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl game/mush.cnf game/mushcnf.dst

game/mush.cnf: game/mushcnf.dst
	-@/usr/bin/touch game/mushcnf.dst
	-@/usr/bin/perl utils/update-cnf.pl game/mush.cnf game/mushcnf.dst

update: update-hdr update-conf

update-hdr:
	-@/usr/bin/touch options.h.dist
	-@/usr/bin/perl utils/update.pl options.h options.h.dist


clean:
	(cd src; make clean)
	(cd game; rm -f netmush mkindx)

distclean: 
	(cd src; make distclean)
	(cd hdrs; rm -f *.orig *~ \#* *.rej *.bak funs.h cmds.h)
	(cd utils; rm -f *.orig *~ \#* *.rej *.bak)
	(cd game; rm -rf *.log netmush mkindx *.orig *.rej *~ *.bak mush.cnf)
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

diffs: 
	-(prcs diff | grep -v 'Index:')

etags: 
	(cd src; make etags)

ctags:
	(cd src; make ctags)

dist.tar.Z: distclean dist.tar
	compress /tmp/dist.tar

dist.tar.gz: distclean dist.tar
	gzip /tmp/dist.tar

dist.tar: distclean
	makedist -c pennmush
	tar -cvFFf /tmp/dist.tar pennmush
	pgp -sb /tmp/dist.tar
	-rm -rf pennmush

CSRImalloc.tar.Z:
	(cd src/CSRI; make clean)
	(tar -cvFFf /tmp/CSRImalloc.tar `cat exclude.tar`)
	compress /tmp/CSRImalloc.tar

