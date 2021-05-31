#!/usr/local/bin/perl5 -w
#
# Process the ic.log file (from stdin):
#  1. Sort it by room and time
#  2. Separate it into distinct conversations. Toss those with <= 5 lines
#  3. Make a list of Conversations
#  4. Sort the list by interestingness
#  5. Spit out each Conversation and how interesting it was
#
require 5.003;
use strict;
use Conversation;
use Line;
use lib '.';
require 'ctime.pl';

sub byplace;
sub byinterestingness;

srand(time);

# Slurp the file
my @text = <STDIN>;

# Remove lines which don't start with a time
@text = grep(/^\d/,@text);

# Remove lines with OOC: in them
@text = grep($_ !~ /OOC:/,@text);

# Sort it by room and time
@text = sort byplace @text;

# Make a set of Lines out of the file
my @lines = map(new Line($_),@text);

# Make a set of Conversations out of the Lines
my @conversations;
my @buffer;
my $line;
my $lasttime = $lines[0]->time;
my $lastroom = $lines[0]->room;
foreach $line (@lines) {
  if ($line->time - $lasttime > 900) {
	# 15 minutes have passed, this is a new conversation
     push(@conversations,new Conversation(@buffer)) if scalar(@buffer) > 5;
     @buffer = ();
  } elsif ($line->room != $lastroom) {
	# A new room, therefore a new conversation
     push(@conversations,new Conversation(@buffer)) if scalar(@buffer) > 5;
     @buffer = ();
  }
  $lasttime = $line->time;
  $lastroom = $line->room;
  push(@buffer,$line);
}
push(@conversations,new Conversation(@buffer)) if scalar(@buffer) > 5;

# Sort the conversations by interestingness
@conversations = reverse sort byinterestingness @conversations;

# Output each
# This just prints them
# We really will want to email them.
#
my $i = 0;
foreach (@conversations) {
  my $time = ctime($_->start_time);
  chop($time);
  print "---- Start: $time ----\n";
  print "---- Players: ", join(" ",$_->players), " ----\n";
  $_->print(\*STDOUT);
  print "---- End: ", $_->interestingness, " ----\n";
  $i++;
  last if $i == 7;
}

exit 0;

################################# 

sub byinterestingness {
  $a->interestingness <=> $b->interestingness;
}

sub byplace {
  my($aroom,$atime,$broom,$btime);
  $a =~ m#^.*?\[\d+/(\d+)/(\d+)\]#;
  ($aroom,$atime) = ($1,$2);
  $b =~ m#^.*?\[\d+/(\d+)/(\d+)\]#;
  ($broom,$btime) = ($1,$2);
  return $aroom <=> $broom unless $aroom == $broom;
  return $atime <=> $btime;
}
