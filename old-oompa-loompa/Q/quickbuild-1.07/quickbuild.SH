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
: This forces SH files to create target in same directory as SH file.
: This is so that make depend always knows where to find SH derivatives.
case "$0" in
*/*) cd `expr X$0 : 'X\(.*\)/'` ;;
esac
echo "Extracting quickbuild (with variable substitutions)"
: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.
$spitshell >quickbuild <<!GROK!THIS!
#!$perlpath
!GROK!THIS!

: In the following dollars and backticks do not need the extra backslash.
$spitshell >>quickbuild <<'!NO!SUBS!'
#
# quickbuild - MUSH building tool. Copyright (c) 1999 Alan Schwartz
#              Released under the GNU Public License
#
# Usage: quickbuild < infile > output.msh
#        Then upload output.msh to a MUSH.
# Requires: Graph and Class::MethodMaker modules from CPAN
#
# Quickbuild file syntax:
#
# # Comment
# "Exit Name" : "Source Room Name" -> "Dest Room Name"
# "Exit Name" : "Source Room Name" <-> "Dest Room Name"
# ROOM FLAGS: <flag list>
# ROOM ZONE: <dbref> or "Room Name"
# ROOM PARENT: <dbref> or "Room Name"
# EXIT FLAGS: <flag list>
# EXIT ZONE: <dbref> or "Room Name" (no, that's not an error)
# EXIT PARENT: <dbref> or "Room Name" (no that's not an error)
# IN "Room Name"
# ... MUSH code in mpp format ...
# ENDIN
# ON "Exit Name" FROM "Source Room"
# ... MUSH code in mpp format ...
# ENDON
#
require 5.000;

# A class for rooms and exits
package Qb::Obj;
use strict;
use Class::MethodMaker
    new_hash_init => 'new',
    get_set => [ qw/parent zone flags code/ ];

package Qb::Room;
use strict;
use vars qw(@ISA);
@ISA = qw(Qb::Obj);
use Class::MethodMaker
    key_attrib => [ qw/name/ ];

package Qb::Exit;
use strict;
use vars qw(@ISA);
@ISA = qw(Qb::Obj);
use Class::MethodMaker
    get_set => [ qw/name src dst/ ],
    key_attrib => [ qw/key/ ];


# The main code
package main;
use strict;
use Graph 0.200;
use vars qw($VERSION %exit_aliases %back_exits %opts $graph);
use Getopt::Long;
use Text::Wrap;

$VERSION = '1.07';

$opts{"brackets"} = 1; # Default to using <B>rackets
GetOptions(\%opts, "help|?", "brackets!", "no-config-file", "config-file=s");
&syntax_and_exit if $opts{"help"};

&read_config_file($opts{"config-file"}) unless $opts{"no-config-file"};

# The big loop
my($t1,$t2);
my %info;
while (<STDIN>) {
  # Ignore comments and blank lines
  next if /^#/;
  next unless /\S/;
  # Ok, what've we got?
  chop;
  if (/^ROOM (ZONE|PARENT|FLAGS?):\s*(.*)/) {
    ($t1,$t2) = ($1,$2);
    $info{"rflags"} = [ split(' ',$t2) ] if $t1 =~ /FLAG/;
    if ($t2 !~ /^#/) {
      # Not a dbref, but a room name
      $t2 =~ s/^"//;
      $t2 =~ s/"\s*$//;
      add_room_code($t2,"\@lock/zone here=#0&!#0") if $t1 eq "ZONE";
      add_room($t2) if $t1 eq "PARENT";
      $t2 =~ s/\s+/_/g;
      $t2 = "[v(ROOM.$t2)]";
    } 
    $info{"rzone"}  = $t2 if $t1 eq "ZONE";
    $info{"rparent"} = $t2 if $t1 eq "PARENT";
    next;
  }  
  if (/^EXIT (ZONE|PARENT|FLAGS?):\s*(.*)/) {
    ($t1,$t2) = ($1,$2);
    $info{"eflags"} = [ split(' ',$t2) ] if $t1 =~ /FLAG/;
    if ($t2 !~ /^#/) {
      # Not a dbref, but a room name
      $t2 =~ s/^"//;
      $t2 =~ s/"\s*$//;
      $t2 =~ s/\s+/_/g;
      $t2 =~ s/\W//g;
      $t2 = "[v(ROOM.$t2)]";
    } 
    $info{"ezone"} = $t2 if $t1 eq "ZONE";
    $info{"eparent"} = $t2 if $t1 eq "PARENT";
    next;
  }  
  if (/"(.*?)"\s*:\s*"(.*?)"\s*(<?->)\s*"(.*?)"/) {
    &add_exit($1,$2,$4,(($3 eq "->") ? 1 : 0),\%info);
    next;
  }
  if (/^IN "(.*)"/) {
    &add_room_code($1,&slurp_code("ENDIN"));
    next;
  }
  if (/^ON "(.*)" FROM "(.*)"/) {
    &add_exit_code($1,$2,&slurp_code("ENDON"));
    next;
  }
  print STDERR "Unknown instruction ignored at line $.:\n";
  print STDERR "  $_\n";
}
 
my $res = &check_graph;
die $res if $res;
print wrap("@@ ","@@ ","$graph"), "\n";
&spew_graph;

exit 0;


sub add_exit {
  my ($exit,$src,$dst,$oneway,$iref) = @_;
  $graph = Graph->new unless $graph;
  # Muck with the exit's alias if it's one we know.
  my $lcexit = "\L$exit";
  $exit = $exit_aliases{$lcexit} if $exit_aliases{$lcexit};
  $exit =~ s/\s*<(.*?)>// unless ($opts{"brackets"});
  # Add the exit to the graph
  $graph->add_edge($src, $dst);
  $graph->set_attribute('exit', $src, $dst, $exit);
  # Create objs for each of the exit's rooms if they don't exist yet.
  # We don't need the refs - we'll find 'em later via name.
  unless (Qb::Room->find_name($src)) {
    new Qb::Room("name" => $src, "zone" => $iref->{"rzone"},
		         "parent" => $iref->{"rparent"},
		         "flags" => $iref->{"rflags"});
  }
  unless (Qb::Room->find_name($dst)) {
    new Qb::Room("name" => $dst, "zone" => $iref->{"rzone"},
		       "parent" => $iref->{"rparent"},
		       "flags" => $iref->{"rflags"});
  }
  # Create an object for the exit
  new Qb::Exit("name" => $exit, "zone" => $iref->{"ezone"},
	       "parent" => $iref->{"eparent"}, "flags" => $iref->{"eflags"},
	       "src" => $src, "dst" => $dst, "key" => "$src\%$exit");
  return if $oneway;
  # Now the return. Do we know the name of the exit back?
  if ($back_exits{$lcexit}) {
    $exit = $back_exits{$lcexit};
    $exit = $exit_aliases{$exit} if $exit_aliases{$exit};
    $exit =~ s/\s*<(.*?)>// unless ($opts{"brackets"});
  }
  $graph->add_edge($dst, $src);
  $graph->set_attribute('exit', $dst, $src, $exit);
  new Qb::Exit("name" => $exit, "zone" => $iref->{"ezone"},
	       "parent" => $iref->{"eparent"}, "flags" => $iref->{"eflags"},
	       "src" => $dst, "dst" => $src, "key" => "$dst\%$exit");
}

sub add_room_code {
  my ($room,$code) = @_;
  my $obj = add_room($room);
  $obj->code("$code\n");
}

sub add_room {
  my $room = shift;
  my $obj = Qb::Room->find_name($room);
  unless ($obj) {
    $obj = new Qb::Room("name" => $room);
    $graph = Graph->new unless $graph;
    $graph->add_vertex($room);
    warn "Adding unlinked room $room (hopefully a parent/zone object?)\n";
  }
  return $obj;
}

sub add_exit_code {
  my ($exit,$room,$code) = @_;
  my $lcexit = "\L$exit";
  $exit = $exit_aliases{$lcexit} if $exit_aliases{$lcexit};
  $exit =~ s/\s*<(.*?)>// unless ($opts{"brackets"});
  my $obj = Qb::Exit->find_key("$room\%$exit");
  $obj ? $obj->code("$code\n")
      : warn("Couldn't find exit $exit from room $room to add code\n");
}

sub slurp_code {
  my $endmarker = shift;
  # Read in code until we reach endmarker. Treat code we read in the
  # way that mpp does. Return the code, in uploadable form. 
  # This is basically a perl mpp:
  # 1. Blank lines and lines starting with @@ are stripped.
  # 2. A non-whitespace, non-'>' character in the first
  # column indicates the start of a new line of MUSHcode. 
  # 3. Leading whitespace on a line is otherwise stripped,
  # and indicates the line is a continuation of the previous line
  # 4. Lines starting with '>' (in the first column) are treated
  # as continuations and are converted from plain ASCII to 
  # "MUSH-ready" ASCII, i.e. spaces -> %b, [ -> \[, etc. %r
  # characters are prepended to any subsequent > lines.
  # 5. In any other line, each tab is converted to a space. 
  # Well, we don't handle #4 for now.
  my $code;
  while (<STDIN>) {
    next if /^@@/;
    next unless /\S/;
    last if /^$endmarker/;
    chop;
    if (/^\S/) {
      $code .= "\n" if defined($code);
      $code .= $_;
      next;
    }
    s/^\s+//;
    s/\t/ /g;
    $code .= $_; 
  }
  return $code;
}

sub check_graph {
  return undef;
}

sub spew_graph {
  # Print out MUSH code. We do it like this.
  # 1. Dig all of the rooms and store their dbrefs
  # 2. Visit each room, and, while there:
  #    a. Open all of the exits leading from that room, applying exit code
  #    b. Apply any room code
  # We attributes on the player to store room dbrefs. We call them
  #   ROOM.<##>. Same for exit dbrefs.
  my %room_attrs;
  # 1. Dig all rooms and track their dbrefs. 
  #   To avoid sync problems, we use: @dig/tel room and @set me=ROOM.<##>:%l
  my @rooms = $graph->vertices;
  my %r;
  my $exit;
  my $neighbor;
  my $flag;
  print "think Digging Rooms\n";
  # Sort the rooms so that we dig those rooms without parents/zones first,
  # as these are the ZMRs/Parent rooms. 
  @rooms = sort { 
    Qb::Room->find_name($a)->{"zone"} cmp Qb::Room->find_name($b)->{"zone"} or
    Qb::Room->find_name($a)->{"parent"} cmp Qb::Room->find_name($b)->{"parent"} 
  } @rooms;
  foreach (@rooms) {
    # a. Dig
    $r{$_} = $_;
    $r{$_} =~ s/\s+/_/g;
    $r{$_} =~ s/\W//g;
    print "\@dig/tel $_\n\@set me=ROOM.$r{$_}:\%l\n";
    # b. Apply room code (which includes @lock/zone for the ZMR)
    my $obj = Qb::Room->find_name($_);
    if ($obj) {
        print "\@chzone here = ", $obj->{"zone"}, "\n"
	    if defined($obj->{"zone"});
        print "\@parent here = ", $obj->{"parent"}, "\n"
	    if defined($obj->{"parent"});
        foreach $flag (@{ $obj->{"flags"} }) {
          print "\@set here = $flag\n";
        }
	print $obj->{"code"} if defined($obj->{"code"});
    }
  }
  # 2. Go to each room
  my $short_exit;
  print "think Linking Rooms\n";
  foreach (@rooms) {
    print "\@tel [v(ROOM.$r{$_})]\n";
    # Open all exits leading from the room
    foreach $neighbor ($graph->successors($_)) {
      $exit = $graph->get_attribute('exit',$_,$neighbor); 
      $short_exit = (split(';',$exit))[0];
      print "\@open $exit = [v(ROOM.$r{$neighbor})]\n";
      my $obj = Qb::Exit->find_key("$_\%$exit");
      if ($obj) {
        print "\@chzone $short_exit = ", $obj->{"zone"}, "\n"
	    if defined($obj->{"zone"});
        print "\@parent $short_exit = ", $obj->{"parent"}, "\n"
	    if defined($obj->{"parent"});
        foreach $flag (@{ $obj->{"flags"} }) {
          print "\@set $short_exit = $flag\n";
        }
	print $obj->{"code"} if defined($obj->{"code"});
      }
    }

  }
}

sub syntax_and_exit {
  print <<EOP;
quickbuild v$VERSION	- offline MUSH building tool

Copyright (c) 1999 Alan Schwartz, same terms as PennMUSH

Usage: quickbuild [options] < inputfile.qbd > outputfile.msh

Options:
    --help|--?          Show this listing
    --nobrackets        Turn off use of <B>racket style exits

See quickbuild manual for syntax of input file.
EOP
 exit 0;
}

sub read_config_file {
  my $file = shift;
  my @files = (".qbcfg", "qb.cfg", $ENV{'HOME'} . "/.qbcfg",
	$ENV{'HOME'} . "/qb.cfg");
  unless ($file) {
    foreach (@files) {
      $file = $_, last if -f $_;
    }
  }
  warn "quickbuild: No configuration file found, continuing.\n", 
	return unless $file;
  open(CFG,$file) or die "quickbuild: Fatal error: Unable to open $file.\n";
  while (<CFG>) {
    next if /^\s*$/;
    next if /^#/;
    if (/^alias\s+"([^"]+)"\s+"([^"]+)"/i) {
      $exit_aliases{$1} = $2;
      next;
    }
    if (/^reverse\s+"([^"]+)"\s+"([^"]+)"/i) {
      $back_exits{$1} = $2;
      $back_exits{$2} = $1;
      next;
    }
    warn "quickbuild: Ignoring unknown line $. in $file\n";
  }
  close(CFG);
}
    


__END__

!NO!SUBS!
chmod 755 quickbuild
$eunicefix quickbuild
