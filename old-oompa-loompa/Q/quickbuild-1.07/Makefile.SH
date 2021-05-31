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
POD2TEXT=$pod2text
INSTALL = $install
BINDIR = $installbin
MANDIR = $installman1dir

!GROK!THIS!
$spitshell >>Makefile <<'!NO!SUBS!'
########################################################################
#
#

PROGS = quickbuild
VERSION=1.07
POD = quickbuild.pod
MAN = quickbuild.1
TXT = quickbuild.txt

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

$(TXT): $(POD)
	-$(POD2TEXT) $(POD) > $(TXT)

install: $(MAN) $(TXT) install-prog install-man 

install-prog:
	$(INSTALL) -m 755 $(PROGS) $(BINDIR)

install-man: $(MAN)
	-$(INSTALL) -m 755 $(MAN) $(MANDIR)

dist.tar.gz: clobber
	makedist -q -c quickbuild-$(VERSION)
	tar cvf quickbuild-$(VERSION).tar quickbuild-$(VERSION)
	gzip quickbuild-$(VERSION).tar
	rm -rf quickbuild-$(VERSION)

!NO!SUBS!
chmod 644 Makefile
$eunicefix Makefile

