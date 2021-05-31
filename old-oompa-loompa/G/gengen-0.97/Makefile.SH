: Makefile.SH

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
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac
CURRENT=.
DIR=`echo $CURRENT/ | sed -e 's/\.\///g'`
echo "Extracting ${DIR}Makefile (with variable substitutions)"

DATE=`date`

$spitshell >Makefile <<!GROK!THIS!
########################################################################
# Makefile generated from Makefile.SH on $DATE
#
# @COPYRIGHT@
#

SHELL = /bin/sh
TOP = .
CURRENT = $CURRENT
DIR = $DIR

########################################################################
# Parameters set by Configure -- edit config.sh if changes are needed

CTAGS = ctags
MAKE = make
RM = $rm -f
ECHO = $echo
POD2MAN=$pod2man
INSTALL = $install
BINDIR = $installbin
MANDIR = $installman1dir
NROFF = $nroff

!GROK!THIS!
$spitshell >>Makefile <<'!NO!SUBS!'
########################################################################
#
#

PROGS = gengen
VERSION=0.97
POD = gengen.pod
MAN = gengen.1
TXT = gengen.txt

default:
	@$(ECHO) "Use one of the following commands:"
	@$(ECHO) "  make all               Build program, man page, text manual"
	@$(ECHO) "  make install           Install program and man page"
	@$(ECHO) "  make install-prog      Install program only"
	@$(ECHO) "  make install-man       Install man page only"
	@$(ECHO) "  make clobber           Clean everything up to pre-Configure state."


clobber:
	$(RM) config.sh config.h
	$(RM) $(PROGS) $(MAN) $(TXT)
	$(RM) -r .config

all: $(MAN) $(TXT)

$(MAN): $(POD)
	-$(POD2MAN) --section=1 $(POD) > $(MAN)

$(TXT): $(MAN)
	-$(NROFF) -man $(MAN) > $(TXT)

install: $(MAN) $(TXT) install-prog install-man 

install-prog:
	$(INSTALL) -m 755 $(PROGS) $(BINDIR)

install-man: $(MAN)
	-$(INSTALL) -m 755 $(MAN) $(MANDIR)

dist.tar.gz: clobber
	makedist -q -c gengen-$(VERSION)
	tar cvf gengen-$(VERSION).tar gengen-$(VERSION)
	gzip gengen-$(VERSION).tar
	rm -rf gengen-$(VERSION)

!NO!SUBS!
chmod 644 Makefile
$eunicefix Makefile

