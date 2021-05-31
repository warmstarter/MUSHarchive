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
echo "Extracting gengen (with variable substitutions)"
: This section of the file will have variable substitutions done on it.
: Move anything that needs config subs from !NO!SUBS! section to !GROK!THIS!.
: Protect any dollar signs and backticks that you do not want interpreted
: by putting a backslash in front.  You may delete these comments.
$spitshell >gengen <<!GROK!THIS!
#!$perlpath -w
!GROK!THIS!

: In the following dollars and backticks do not need the extra backslash.
$spitshell >>gengen <<'!NO!SUBS!'
#
# gengen - MUSH chargen system building tool. Copyright (c) 2000-2002 Alan Schwartz
#          Released under the GNU Public License
#
# Typical usage: gengen < infile | quickbuild > output.msh
#                Then upload output.msh to a MUSH.
# Requires: quickbuild (ftp://ftp.pennmush.org/pub/PennMUSH/Accessories)
#
# Basic gengen file syntax (see the pod/man page for details):
#
# # Comment
# Room name : ATTRIBUTES SET HERE : <method for setting them>
#   Help text, on lines beginning with white space
#
#

require 5.000;
use vars qw(@Function_Object $VERSION %config @attribsets @allattribs);
use Text::Wrap;
use strict;

$VERSION = '0.97';

$config{'command'} = '+char';
$config{'base'} = 'char_';
$config{'atrflags'} = 'wizard mortal_dark';
$config{'addition'} = 'fold';
$config{'topology'} = 'linear';

# One day, we'll support alternative storage systems
#$config{'storage'} = 'attribs';

# Part 0: Initialize the function object
$Function_Object[0] = <<EOP;
\@create Chargen Function Object
\@power chargen function object=see_all
\@lock chargen function object==me
\@set chargen function object=!NO_COMMAND
&SHEET_HEADER chargen function object=____Character sheet_[repeat(_,50)]
&SHEET_BODY chargen function object=[iter(v(ATTRIBSETS),u(DISPLAY_##,\%0))]
&SHEET_FOOTER chargen function object=\%r[repeat(_,75)]
EOP
@attribsets = ();
@allattribs = ();

#
# Part 1: Read in the file and parse it
#
my $hub = "Chargen Hub Room";
my(@rooms,%room,$exits,%help,$current_room,$attr,$spec);
while (<STDIN>) {
  next if /^#/;		# Skip comments
  next if /^$/;		# Skip blank lines
  my($name,$attr,$spec);
  chop;
  if (/(.+)?\s*=\s*(.*)/) {
    # Configuration stuff
    $config{$1} = $2;
  } elsif (/^(.+)?\s*:\s*(.+)?\s*:\s*(.*)/) {
    ($name,$attr,$spec) = ($1,$2,$3);
    $name =~ s/^\s+//;
    $name =~ s/\s+$//;
  } elsif (/^\s+(\S.*)/) {
    $help{$current_room} .= $1;
  } else {
    # Assume we've got a room name here that does no chargen code
    $name = $_;
  }

  if (defined($name)) {
    # Generate a room, and remember where we are.
    if ($config{'topology'} =~ /spoke/i) {
      my $shortname = &abbreviate($name);
      $exits .= <<EOP;
"$name <$shortname>;$name;$shortname": "$hub" -> "$name"
"Out;o" : "$name" -> "$hub"
EOP
    } else {
      if ($current_room) {
        $exits .= <<EOP;
"Next;nex;ne;n": "$current_room" -> "$name"
ON "Next;nex;ne;n" FROM "$current_room"
\@lock next=isdone/1
\@power next=see_all
ENDON
EOP
      }
    }
    $current_room = $name;
    push(@rooms, $current_room);
    $room{$current_room} = &genroom($current_room, $attr, $spec);
  }
}

$Function_Object[0] .= <<EOP;
&DO_SHEET chargen function object=
   \$$config{'command'}/sheet: 
      \@pemit \%#=[u(SHEET_HEADER)][u(SHEET_BODY,\%#)][u(SHEET_FOOTER)]
EOP
$Function_Object[0] .= "&ATTRIBSETS chargen function object = " . 
	join(" ",@attribsets) . "\n";

#
# Part 2: Spew generated code
#

# Parents and helper object
my $allattribs = join(" ",@allattribs);
if ($config{'topology'} eq 'spokes') {
  print <<EOP;
"Done": "Chargen Room Parent" -> "Chargen Room Parent"
ON "Done" FROM "Chargen Room Parent"
\@power done=see_all
\@power done=link_anywhere
\@link done=variable
&destination done=#0
&isdone done=[not(regmatch(u(missing,\%#),\\\\S))]
&missing done=[setdiff(iter(v(ALLATTRIBS),ucstr($config{'base'}##)),lattr(\%0))]
&allattribs done=$allattribs
\@lock done=isdone/1
\@succ done=You leave character generation.
\@osucc done=is done with character generation, and leaves.
\@odrop done=arrives, newly generated.
\@fail done=You haven't completed all of character generation yet! 
   (You seem to be missing these attributes: [u(missing,\%#)])
ENDON
EXIT PARENT: "Chargen Exit Parent"
IN "Chargen Exit Parent"
\@succ here=You continue to [name(loc(me))].
\@osucc here=goes to [name(loc(me))].
ENDIN
EOP
} else {
  print <<EOP;
EXIT PARENT: "Chargen Exit Parent"
IN "Chargen Exit Parent"
&isdone here=[not(regmatch(xget(\%#,$config{'base'}TODO),\\\\w))]
\@succ here=You continue to the next step.
\@fail here=You haven't finished this step yet.
ENDIN
EOP
}
print <<EOP;
ROOM PARENT: "Chargen Room Parent"
IN "Chargen Room Parent"
\@lock/zone here=#0
&show here=\$$config{'command'} *:
	\@pemit \%#=[switch(0,not(strmatch(\%0,*=*)),,
          match(v(attribs),\%0),
	    That's not one of this step's attributes,
	  \%0: [u(lookup,\%#,\%0)])]
&lookup here=[xget(\%0,$config{'base'}\%1)]
&HELP here = \$$config{'command'}/help: \@pemit \%#=[u(helptext)]
\@desc here=
  Here you will set these attributes: [v(ATTRIBS)]\%r
  [u(DESC_HOWTO)]\%r
  [switch(v(DESC_EXAMPLE),,,[u(DESC_EXAMPLE)]\%r)]
  Type $config{'command'}/help for additional info about these attributes.\%r
  Type $config{'command'} name to view the value of an attribute set here.\%r
  Type $config{'command'}/sheet to view your current character sheet.
EOP
print @Function_Object;
print <<EOP;
\@parent here=chargen function object
ENDIN
ROOM FLAGS: !NO_COMMAND
EOP

print <<EOP if ($config{'topology'} eq 'spokes');
ROOM ZONE: "Chargen Room Parent"
EOP

# Exit topology
print $exits;

print <<EOP if ($config{'topology'} eq 'spokes');
IN "$hub"
\@desc here=Welcome to Character Generation. Visit each of the rooms 
  around this one to establish various aspects of your character. 
  When you've completed them all, type 'done'.
ENDIN
EOP

# Room attributes
foreach (@rooms) {
  print "IN \"$_\"\n";
  print $room{$_},"\n";
  my $text = wrap("", "  ",<<EOP);
&HELPTEXT here = $help{$_} 
EOP
  # mpp (and emulators, like quickbuild) need a blank at line-ends
  $text =~ s/\012/ \012/g;
  print $text;
  print '@tr here/startup', "\n";
  print "ENDIN\n";
}


# genroom - return the code for a room
# As a side effect, muck with the function object
# Args: attribute list, gen spec
sub genroom {
  my ($name,$attribs,$spec) = @_;
  if (defined($attribs)) {
    my $attribset = $attribs;
    $attribset =~ s/^\s+//;
    $attribset =~ s/\s+$//;
    $attribset =~ s/\s+/_/g;
    push(@attribsets,$attribset);
    push(@allattribs,$attribs);
    push(@_,$attribset);
  }
  if (defined($spec)) {
    # Sneaky perlism - calling &function w/o () passes @_ implicitly
    return &genroom_string if ($spec =~ /^string/i);
    return &genroom_roll if ($spec =~ /^roll/i);
    return &genroom_spend if ($spec =~ /^spend/i);
    return &genroom_choose if ($spec =~ /^choose/i);
  }
}

# Build a room that asks players to give some strings.
sub genroom_string {
  my ($name,$attribs,$spec,$attribset) = @_;
  my @attribs = split(" ",$attribs);
  my $room =<<EOP;
&attribs here=@attribs
&desc_howto here=To set each attribute, type $config{'command'} name=value
&desc_example here=For example: $config{'command'} $attribs[0]=My info here
\@set here=wizard
\@aenter here=&$config{'base'}TODO \%# = $attribs
EOP
  foreach (@attribs) {
    $room .= <<EOP;
&SET_$_ here=\$$config{'command'} $_=*:
  &$config{'base'}$_ \%# = \%0;
  \@pemit \%#=You set your $_ to: \%0;
  &$config{'base'}TODO \%# = [setdiff(xget(\%#,$config{'base'}TODO),$_)]
EOP
  }
  push(@Function_Object,<<EOP);
&DISPLAY_$attribset chargen function object = 
  [iter($attribs,\%r[capstr(##)]: [xget(\%0,$config{'base'}##)])]
EOP
  return $room;
}

# Build a room that asks players to choose from a set of options
sub genroom_choose {
  my ($name,$attribs,$spec,$attribset) = @_;
  my @attribs = split(" ",$attribs);
  unless ($spec =~ m!choose\s+(.*)!) {
    print STDERR "Invalid choose spec ignored: $spec\n";
    return;
  }
  my $optlist = $1;
  my @optlist = split(/,\s*/,$1);
  my $mushoptlist = join(",",@optlist);
  my $each = ($#attribs > 0) ?
	"You must give each attribute here a different value." : "";
  my $room =<<EOP;
&options here=$mushoptlist
&attribs here=@attribs
&desc_howto here=To set each attribute, type $config{'command'} name=value\%r
  Values are: $optlist\%r$each
&desc_example here=For example: $config{'command'} $attribs[0]=$optlist[0]\%r
  Type $config{'command'}/reset to start over.
\@set here=wizard
\@aenter here=&$config{'base'}TODO \%# = $attribs
\@startup here=\@drain me; \@notify me
&reset here=\$$config{'command'}/reset: \@wait me= {
        &$config{'base'}TODO \%# = $attribs;
	\@pemit \%#=Your values have all been cleared.;
	\@dol/notify [v(attribs)]=&$config{'base'}## \%#
	}
&valuesleft here=[setdiff(lcstr(v(options)),
		  lcstr(iter(v(attribs),xget(\%#,$config{'base'}##),,\\,)),\\,)]
EOP
  foreach (@attribs) {
    $room .= <<EOP;
&SET_$_ here=\$$config{'command'} $_=*:
  \@wait me= {\@switch 1 = 
	not(match(u(valuesleft,\%#),\%0,\\,)),
	{ \@pemit \%#=Values must be one of: [u(valuesleft,\%#)]; \@notify me },
      { &$config{'base'}$_ \%# = \%0;
        \@pemit \%#=You set your $_ to: \%0;
        &$config{'base'}TODO \%# = [setdiff(xget(\%#,$config{'base'}TODO),$_)];
	\@notify me
      }}
EOP
  }
  push(@Function_Object,<<EOP);
&DISPLAY_$attribset chargen function object = 
  [iter($attribs,\%r[capstr(##)]: [xget(\%0,$config{'base'}##)])]
EOP
  return $room;
}

sub parsedice;

# Build a room that rolls dice
sub genroom_roll {
  my ($name,$attribs,$spec,$attribset) = @_;
  my @attribs = split(" ",$attribs);
  # Parse the spec
  unless ($spec =~ m!roll(/(\d+))?\s+(.*)!) {
    print STDERR "Invalid roll spec ignored: $spec\n";
    return;
  }
  my ($rolls,$dice) = ($2,$3);
  my @dice = split(" ",$dice);
  my $prog = "[add(0," . join(",",map(parsedice($_),@dice)) . ")]";
  my $room =<<EOP;
&attribs here=@attribs
&diceprog here=$prog
\@set here=wizard
&desc_howto here=These attributes have been set for you by rolling dice.
&desc_example here=There is nothing for you to do here but view the values 
	that were rolled.
EOP
  $room .= <<EOP if ($rolls);
&desc_example here=
  You may re-roll your attributes up to $rolls times, but you must 
  reroll all of them at once. Type $config{'command'}/roll to reroll. 
EOP
 my $rollcode = join("; ", map("&$config{'base'}$_ %# = [u(diceprog)]", @attribs));
 $room .= "\@aenter here=$rollcode; ";
 if ($rolls) {
  $room .= <<EOP;
&$config{'base'}ROLLS_[num(me)] \%#=$rolls
&roll here=\$$config{'command'}/roll: 
  \@switch [setr(0,xget(\%#,$config{'base'}ROLLS_[num(me)]))]=>0,
	{ &$config{'base'}ROLLS_[num(me)] \%#=[dec(%q0)];
	  $rollcode;
	  \@pemit \%#=You re-roll those attributes. [dec(%q0)] rerolls left.
	},
	{ \@pemit \%#=You aren't allowed to re-roll now. }
EOP
 } else {
  $room =~ s/; $/\n/;
 }
 $room .= "&ADD_NUMS here=add(\%0,\%1)\n" if ($config{'addition'} eq 'fold');
 my $numrows = 1 + int( $#attribs / 3 );
 push(@Function_Object,<<EOP);
&DISPLAY_$attribset chargen function object = 
  [iter(lnum($numrows),
    \%r[ljust([capstr(extract($attribs,add(1,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(1,mul(##,3)),1)))],25)]
       [ljust([capstr(extract($attribs,add(2,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(2,mul(##,3)),1)))],25)]
       [ljust([capstr(extract($attribs,add(3,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(3,mul(##,3)),1)))],25)]
   )]
EOP
 return $room;
}

sub parsedice {
  my $dice = shift;
  return 0 unless ($dice =~ /(-)?((\d+)(best|worst))?(\d+)(d(\d+))?/);
  my ($neg,$pick,$bw,$num,$sides) = ($1,$3,$4,$5,$7);
  my $result;
  if ($pick) {
    if ($bw eq "best") {
      $result = &additup("extract(revwords(sort(iter(lnum($num),add(1,rand($sides))),n)),1,$pick)");
    } else {
      $result = &additup("extract(sort(iter(lnum($num),add(1,rand($sides))),n),1,$pick)");
    }
  } elsif ($sides) {
    $result = "[die($num,$sides)]";
  } else {
    $result = $num;
  }
  return $neg ? "[sub(0,$result)]" : $result;
}

sub additup {
  my $list = shift;
  if ($config{'addition'} eq 'lmath') {
    return "[lmath(add,$list)]";
  } elsif ($config{'addition'} eq 'ladd') {
    return "[ladd($list)]";
  } elsif ($config{'addition'} eq 's') {
    return "[s(add\\([edit($list,%b,\\,)]\\))]";
  } else {
    return "[fold(add_nums,$list)]";
  }
}
    

sub genroom_spend {
  my ($name,$attribs,$spec,$attribset) = @_;
  my @attribs = split(" ",$attribs);
  # Parse the spec
  unless ($spec =~ /^spend\s+(\d+)\s*(start (\d+))?\s*(max (\d+))?\s*(min (\d+))?\s*((poly|exp)(\d+))?/i) {
    print STDERR "Invalid spend spec ignored: $spec\n";
    return;
  }
  my ($total,$initlvl,$maxlvl,$minlvl,$fn,$mag) = ($1,$3,$5,$7,$9,$10);
  $initlvl = 0 unless $initlvl;
  # Set all attribs to initlvl
  my $resetcode = join("; ", map("&$config{'base'}$_ \%0 = $initlvl", 
                       @attribs))
      . "; &$config{'base'}POINTS \%0=$total";

 my $room =<<EOP;
&attribs here=@attribs
\@set here=wizard
&desc_howto here=
  Each attribute's level starts at $initlvl, and may vary from $minlvl 
  to $maxlvl. You begin with $total points to spend to adjust levels of
  these attributes. Increasing an attribute costs points, and decreasing
  one gives you additional points.\%r
  Type $config{'command'}/set name=level to adjust an attribute to a given level\%r
  Type $config{'command'}/reset to reset these attributes to their initial values and start over.
&resetcode here=$resetcode
\@aenter here=\@wait me={\@tr me/resetcode=\%#; \@notify me}
\@startup here=\@drain me; \@notify me
\@notify here
&maxlvl here=$maxlvl
&minlvl here=$minlvl
&reset here=\$$config{'command'}/reset: \@wait me= {
	\@tr me/resetcode=\%#;
	\@pemit \%#=Your values have all been reset.;
	\@notify me }
&spend here=\$$config{'command'}/set *=*: \@wait me={
  \@switch/first [setq(0,xget(\%#,$config{'base'}POINTS))]0=\%q0, 
	  { \@pemit \%#=You have no points left to spend.; \@notify me },
	[match($attribs,\%0)],
	  { \@pemit \%#=You can only set these attribs here: $attribs;
	    \@notify me },
	[lte(\%1,v(maxlvl))],
	  { \@pemit \%#=You can't set that attribute that high.;
	    \@notify me },
	[gte(\%1,v(minlvl))],
	  { \@pemit \%#=You can't set that attribute that low.;
	    \@notify me },
	[lt(setr(1,u(ptcost,\%#,\%0,\%1)),\%q0)],
	  { \@pemit \%#=You can't afford to set that attribute to that level.;
	    \@notify me },
	{ &$config{'base'}POINTS \%#=
 	    [sub(xget(\%#,$config{'base'}POINTS),\%q1)];
	  &$config{'base'}\%0 \%#=\%1;
	  \@pemit \%#=You set %0 to %1. You have [xget(\%#,$config{'base'}POINTS)] points left.;
	  \@notify me
	}
  }
&ptcost here=[sub(u(abscost,\%2),u(abscost,u(lookup,\%0,\%1)))]
EOP
  # What's the cost to go from lvl 0 to lvl X?
  # poly<n>: Cost is x**n
  if ("\L$fn" eq "poly") {
    # poly<n>: Cost is x**n
    # Special case for n = 1
    $room .= ($mag == 1) ?
	"&abscost here=\%0\n" :
        "&abscost here=[power(\%0,$mag)]\n"
  } else {
    # exp<n>: cost is n**x
    $room .= "&abscost here=[power($mag,\%0)]\n";
  }
  my $numrows = 1 + int( $#attribs / 3 );
  push(@Function_Object,<<EOP);
&DISPLAY_$attribset chargen function object = 
  [iter(lnum($numrows),
    \%r[ljust([capstr(extract($attribs,add(1,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(1,mul(##,3)),1)))],25)]
       [ljust([capstr(extract($attribs,add(2,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(2,mul(##,3)),1)))],25)]
       [ljust([capstr(extract($attribs,add(3,mul(##,3)),1))]:\%b
              [xget(\%0,strcat($config{'base'},extract($attribs,add(3,mul(##,3)),1)))],25)]
   )]
EOP
  return $room;
}

# Abbrievate a Capitalized Phrase as ACP
sub abbreviate {
  my $long = shift;
  my @words = split(" ",$long);
  my @caps = grep(s/^([A-Z]).*/$1/, @words);
  @caps = grep(s/^(.).*/$1/, @words) unless @caps;
  return join("",@caps);
}


!NO!SUBS!
chmod 755 gengen
$eunicefix gengen
