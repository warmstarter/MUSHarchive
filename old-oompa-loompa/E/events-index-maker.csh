#!/bin/csh
# Usage (assume events file is called events.txt):
#    $0 < events.txt
#    or: cat events.txt | $0

# Hey, I've been wanting this for a long time: an index for events. So I wrote
# this script, which is not pretty, but does the task of creating an index of
# the events. It takes as input the names of all the events files, and sends
# the (hopefully correctly) formatted events index entries to standard output.
# - Schmidt@DUNEmush


# Use your fastest awk, as that'll speed things up if there are a lot
# of events entries. However, this DOES with with plain old awk.
#set AWK=gawk
#set AWK=nawk
set AWK=awk

# Old style syntax
#  grep '^& ' $argv  |cut -c3-100 | tr '[A-Z]' '[a-z]'  | sort |\

  grep '^& '  |cut -c3-100 | tr '[A-Z]' '[a-z]'  | sort |\
$AWK ' \
{x[NR]=$0} \
END {  \
     LINES=15; \
     for (n=0;n<=NR/3/LINES;n++) {  \
       if (n==0) {  \
          printf("& INDEX\n")  \
       }else{  \
          printf("See INDEX%d for more entries.\n\n",n);  \
          printf("& INDEX%d\n",n)  \
       }  \
       last=(n+1)*3*LINES+1 \
       if (last > NR) last = NR \
       for (i=n*3*LINES+1;i<=last;i+=3) {  \
         printf("%-25s %-25s %-25s\n",x[i],x[i+1],x[i+2])  \
       }  \
     }  \
}'
