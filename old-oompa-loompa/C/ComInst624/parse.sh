#!/bin/sh
# Parser to split up file information
#
# Argument 1 must be a file
# Argument 2 and 3 must be available for searching
# Argument 3 is optional, 2 is required.
#
# (c) Ashen-Shugar, all rights reserved.
#

if [ ! -f "$1" ]
then
   echo @@ Invalid file specified @@
   echo @@ Syntax: `basename $0` "<filename> <start> [<end>]" @@
   exit 1
fi

if [ -z "$2" ]
then
   echo @@ Must have starting positioning flag @@
   exit 1
else
   GRPCNT=`grep -c "\<$2\>" $1`
   if [ $GRPCNT -eq 0 ]
   then
      echo @@ STARTING Matched positioning flag non-existant - "$2" @@
      exit 1
   fi
fi

if [ ! -z "$3" ]
then
   GRPCNT=`grep -c "\<$3\>" $1`
   if [ $GRPCNT -eq 0 ]
   then
      echo @@ ENDING Matched positioning flag non-existant - "$3" @@
      exit 1
   fi
fi

FIL_BEG=$2
FIL_END=$3
export FIL_BEG FIL_END
awk '
  BEGIN {
     START=0
     COMF=0
     COMH=0
     FILB=sprintf("\<%s\>",ENVIRON["FIL_BEG"])
     FILE=sprintf("\<%s\>",ENVIRON["FIL_END"])
     printf("@set me=quiet\n")
  }
  match($0, FILB) {
     START=1
  }
  START {
     if ( match ($0, "FunctionDB=" ) != 0 ) {
	if ( COMF == 0 ) {
           printf("@fo me=&F-DB me=get(comsystem/f-db)\n")
	   system("sleep 2")
        }
	sub("FunctionDB=","[v(f-db)]=")
	COMF=1
     }
     if ( match ($0, "HelpDB=" ) != 0 ) {
	if ( COMH == 0 ) {
           printf("@fo me=&H-DB me=get(comsystem/h-db)\n")
	   system("sleep 2")
        }
	sub("HelpDB=","[v(h-db)]=")
	COMH=1
     }
     printf("%s\n", $0 )
  }
  match($0, FILE) {
     START=0
  }
  END {
     if ( COMF )
        printf("&F-DB me\n")
     if ( COMH )
        printf("&H-DB me\n")
     printf("@set me=!quiet\n")
  }
' $1
