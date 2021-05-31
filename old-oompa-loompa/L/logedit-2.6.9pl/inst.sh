#!/bin/sh -
#
# Find perl!
#
perl="`which perl5`"
if [ "x" = "x$perl" ]; then
  perl="`which perl5.001`"
  if [ "x" = "x$perl" ]; then
    perl="`which perl5.000`"
    if [ "x" = "x$perl" ]; then
      perl="`which perl`"
      if [ "x" = "x$perl" ]; then
	perl=''
      fi
    fi
  fi
fi

echo -n "Where does perl live on your system? [$perl] "
read ans

if [ ! "x" = "x$ans" ]; then
	if [ -d $ans ]; then
		perl="$ans/perl"
	else
		perl="$ans"
	fi
fi

if [ -x $perl ]; then
	exec $perl inst.pl $perl
else
	echo "I can't find a perl program you can use."
	echo "Ask your system administrator where perl is located."
	exit
fi
exit
