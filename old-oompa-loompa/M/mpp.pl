#!/usr/bin/perl
#
# mpp.pl  - a MUSH "pre-processor" in Perl
#
# (Based on MPP by Joshua Bell, jsbell@acs.ucalgary.ca)
#
# This implementation by Alan "Javelin" Schwartz, dunemush@pennmush.org
# It differs from Josh's mpp in two ways: 
#  1. It does not implement the > line prefix feature, and
#  2. It concatenates continuation lines with a single space between them
#
# Usage: mpp.pl < infile > outfile
#
#    Parsing rules for mpp.pl:
#    ~~~~~~~~~~~~~~~~~~~~~~
# 
#    1. Blank lines and lines starting with @@ are stripped.
# 
#    2. A non-whitespace character in the first
#    column indicates the start of a new line of MUSHcode. 
# 
#    3. Leading whitespace on a line is otherwise stripped to a single space,
#    and indicates the line is a continuation of the previous line. 
#    So break code where a space wouldn't matter, and indent contination lines.
#    
#    4. In any other line, each tab is converted to a space. 
#    Width is not conserved.
# 

my $line;
while (<STDIN>) {
	next if /^@@/;
	next if /^$/;
	s/\t/ /g;
	chomp;
	if (/^\s/) {
	  # Continuation of previous line
	  s/^\s+//;
	  $line .= $_;
	} else {
	  # Start of a new line, print the buffer
	  print "$line\n" if defined($line);
	  $line = $_;
	}
}
print "$line\n" if defined($line);

