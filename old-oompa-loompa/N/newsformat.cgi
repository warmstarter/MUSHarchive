#!/usr/local/bin/perl
#############################################################################
#
# Title:   newsformat
# Author:  Myrddin (aka J. Scott Dorr)
# Date:    12-8-95
#
# This script will take a normal text file, and split it up into news.txt
# format: a total of 24 lines per indexed entry including header and footer.
# A new file is created - the old file is left untouched.
#
# Anyone may use this script, give it out, copy it, or modify it, as long as
# the credits are kept intact.
#
# Modifications by Myrddin:
# 1-18-96: 1. Adds a couple of spaces to completely blank lines (needed so
#             that the MUSH prints a 'blank line'. It otherwise skips over 
#             the blank line to the next, non-whitespace only line.
#          2. Added some intelligence regarding where we break/start a new
#             block: if the 18th line is blank, break on it. If the first
#             line of a new block is blank, skip it.
#          3. The 'title' will now be properly centered (previously, a few
#             tabs were thrown in to place the title 'somewhere near center'.
#          4. Title will now read 'MyTitle: (part x of y)' rather than just
#             'MyTitle: (part x)'
#
#############################################################################

unless (@ARGV == 3) {
   print "usage:\tnewsformat <filename> <title> <topic>\n\t(example: newsformat history.doc 'The Dreaming: History' 'history')\n";
   exit;
}

$file =	 $ARGV[0];
$title = $ARGV[1];
$topic = $ARGV[2];

$dbl_line = '='x77;
$thn_line = '-'x77;
$count = 18;
$first = 1;

open(F1,$file) || die "newsformat: Unable to open $file.\n";
open(F2,">$file.news") ||
     die "newsformat: Unable to open $file.news for output.\n";
$num_lines = @lines = +<F1>;
close F1;
chop @lines;

$tot_parts = &round($num_lines,18);

foreach $line (@lines) {
   if ($line =~ /^$/) { $line = '  '; }

   ## If the 18th line is blank, break on it. If the first line of the 
   ## next section is gonna be blank, skip it.

   if (($line =~ /^\s+$/) && $count >= 17) {
      $count++ if $count == 17;
      next;
   }
   
   if ($count == 18) {	## $count is how many lines we have already printed
      $part++;
      if ($first) {	## First time through loop
         $title_bar = &center("$title (part $part of $tot_parts)");
         print F2 "& $topic\n$dbl_line\n$title_bar\n$thn_line\n";
         $first = 0;
      } else {
         if ($part == $tot_parts) {
            print F2 "$thn_line\nConcluded in \'news $topic$part\'\n$dbl_line\n\n";
         } else {
            print F2 "$thn_line\nType \'news $topic$part\' for more\n$dbl_line\n\n";
         }
         $title_bar = &center("$title (part $part of $tot_parts)");
         print F2 "& $topic$part\n$dbl_line\n$title_bar\n$thn_line\n";
      }
      print F2 "$line\n";
      $count = 0;
   } else {
      print F2 "$line\n";
   }

   $count++;
}
print F2 "$dbl_line\n";
close F2;

print "New file saved to ${file}.news\n";

##############################################################################
#
# round: divides a number by another, rounding up.
#
sub round {
   ($portion = int($_[0]/$_[1]))*$_[1] == $_[0]? $portion : $portion +1;
}

##############################################################################
#
# center: will center text.
#
# Usage: center(<text>[,<width>])
#
sub center {
   local($linelength) = $_[1] || 78;
   local($extra) = ($linelength - length($_[0]))/2;
   local($buf) = ' ' x $extra;
   return "$buf$_[0]";
}
