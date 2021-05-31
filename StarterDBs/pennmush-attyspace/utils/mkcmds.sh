#!/bin/sh
#
# Make the table of commands and switches
#
# We run this from the utils directory
#
if [ -f "../nocmds" ]; then
  echo "Not building"
  exit
fi

echo "Rebuilding command switch file"
snum=1
rm -f ../hdrs/cmds.h
rm -f ../hdrs/funs.h
rm -f ../hdrs/switches.h
rm -f ../src/switchinc.c
for s in `cat ../src/SWITCHES | sort`; do
  echo "#define SWITCH_$s $snum" >> ../hdrs/switches.h
  echo "{"			 >> ../src/switchinc.c
  echo "  \"$s\", SWITCH_$s"	 >> ../src/switchinc.c
  echo "}"			 >> ../src/switchinc.c
  echo ","			 >> ../src/switchinc.c
  snum=`expr $snum + 1`
done

echo "Rebuilding command prototype file"
for c in `grep "^COMMAND *(" ../src/*.c | cut -f2 -d\( | cut -f1 -d\) | sort`; do
  echo >>../hdrs/cmds.h "COMMAND_PROTO($c);"
done

echo "Rebuilding function prototype file"
for c in `grep "^FUNCTION *(" ../src/*.c | cut -f2 -d\( | cut -f1 -d\) | sort`; do
  echo >>../hdrs/funs.h "FUNCTION_PROTO($c);"
done

if [ -d "../win32" ]; then
  cp ../hdrs/funs.h ../win32/funs.h
  cp ../hdrs/cmds.h ../win32/cmds.h
fi
