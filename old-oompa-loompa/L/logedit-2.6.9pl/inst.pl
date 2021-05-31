#!perl
#
# Usage: install-logedit.pl <path-to-perl>
#
# Install logedit.
# 1. Modify logedit.dist, adding the right pathname for perl and
#    setting $unix, since this script is only used for Unix installs
# 2. Copy the resulting logedit to the appropriate directory, and
#    make manpage links, if requested.
#

print "Logedit installer - by Alan Schwartz\n";
print "\nTry to answer all questions as best you can. When in doubt,\n";
print "accept the defaults. :)\n";

# 0. Set stuff up and check the kit
$ENV{'IFS'} = '' if $ENV{'IFS'} ne '';
$ENV{'PATH'} = '/bin:/usr/bin:/usr/ucb:/usr/5bin:/usr/sbin:/usr/local/bin:/usr/bsd';
$shell = $ENV{'SHELL'};
$home = $ENV{'HOME'};
$home = $ENV{'LOGDIR'} unless $home;
$BINDIR = "$ENV{'HOME'}/bin";
$BINDIR = "/usr/local/bin" unless -d $BINDIR && -w _;
$BINDIR = "$ENV{'HOME'}" unless -d $BINDIR && -w _;
$MANDIR = "/usr/local/man/man1";
$MANDIR = "/usr/man/manl" unless -d $MANDIR && -w _;
$MANDIR = "/dev/null" unless -d $MANDIR && -w _;
$CATDIR = "/usr/local/man/cat1";
$CATDIR = "/usr/man/catl" unless -d $CATDIR && -w _;
$CATDIR = "/dev/null" unless -d $CATDIR && -w _;
chop($CP = `which cp`);
chop($NROFF = `which nroff`);
opendir(DIR,".") || die "I can't even see the files here!\n";
@files = readdir(DIR);
closedir(DIR);
foreach $file (@files) {
  $file{$file}++;
}
die "I can't find cp!\n" unless -x $CP;
warn "I can't find nroff, so I can't make formatted man pages\n" unless -x $NROFF;
die "I can't find logedit.dist\n" unless $file{"logedit.dist"};
print "Looks like everything's here, ok.\n";

# 1. Modify logedit.dist
open(IN,"logedit.dist");
open(OUT,">logedit");
while (<IN>) {
	$_ = "#!$ARGV[0]\n" if /^#!/;
	$_ = "\$unix = 1;\n" if /^#\$unix/;
	print OUT;
}
close(IN);
close(OUT);

# 2. Install!
do {
  print "Where would you like logedit installed? [$BINDIR] ";
  chop($bin = <STDIN>);
  $bin = $BINDIR if $bin =~ /^$/;
  $bin =~ s/^~/$home/;
  print "I can't put it in that directory!\n" unless -w $bin;
} while (! -w _);
print "Time to figure out the permission settings for logedit.\n";
print "There are a couple of standard setups:\n";
print "1. Just you - owner gets read/execute permission, others nothing\n";
print "2. Others - everyone gets read/execute permission\n";
do {
  print "Who will be using logedit? [1] ";
  chop($who = <STDIN>);
  $who = '1' if $who =~ /^$/;
} while ($who != 1 && $who != 2);
print "Installing logedit program...\n";
if ($who == 1) {
$prog =<<'EOP';
system "$CP logedit $bin/logedit\n";
chmod 0700, "$bin/logedit";
EOP
} else {
$prog =<<'EOP';
system "$CP logedit $bin/logedit\n";
chmod 0755, "$bin/logedit";
EOP
}
eval $prog;
die "Oops! Something went wrong, and I can't install. Try again.\n(Error was: $@)\n" if $@;
unlink("logedit");
print "\n";
print "Logedit is its own manual page, and can be read by using\n";
print "nroff -man logedit. For convenience, a symbolic link can be\n";
print "made from logedit to the local man page directory, allowing\n";
print "users to simply type 'man logedit'. Or, if your system uses\n";
print "formatted manual pages, I can produce and install a formatted\n";
print "manual page for logedit.\n";
print "\n";

do {
print "Would you like to install the symbolic link? [y] ";
  chop($yn = <STDIN>);
  $yn = 'y' if $yn =~ /^$/;
} while ($yn !~ /^(y|n)/i);
if ($yn =~ /^y/i) {
  # Install unformatted man page
 do {
  print "Where do you store unformatted manual pages? [$MANDIR] ";
  chop($man = <STDIN>);
  $man = $MANDIR if $man =~ /^$/;
  $man =~ s/^~/$home/;
  print "I can't put files there!\n" unless -w $man;
 } while (! -w _);
 if ($man =~ /([0-9l])$/) {
   $man .= "/logedit.$1"; 
 } else {
   $man .= "/logedit.1";
 }
  $prog = <<'EOP';
    symlink("$bin/logedit",$man);
EOP
  eval $prog;
  if ($@) {
    print "It appears your system doesn't support symbolic links\n";
    print "What a bummer. I'll just make a copy instead.\n";
    $prog = <<'EOP';
        system("$CP $bin/logedit $man");
        chmod 0444, $man;
EOP
    eval $prog;
    warn "Ouch. Something's badly broken. No can do.\n" if $@;
  }
}

if (-x $NROFF) {
  do {
    print "Would you like to install the formatted man page? [n] ";
    chop($yn = <STDIN>);
    $yn = 'n' if $yn =~ /^$/;
  } while ($yn !~ /^(y|n)/i);
  if ($yn =~ /^y/i) {
    # Install formatted man page
    do {
     print "Where do you store formatted manual pages? [$CATDIR] ";
     chop($cat = <STDIN>);
     $cat = $CATDIR if $cat =~ /^$/;
     $cat =~ s/^~/$home/;
     print "I can't put files there!\n" unless -w $cat;
    } while (! -w _);
    if ($cat =~ /([0-9l])$/) {
      $cat .= "/logedit.$1"; 
    } else {
      $cat .= "/logedit.1";
    }
    $prog =<<'EOP';
       system "$NROFF -man $bin/logedit > $cat";
       chmod 0666, $cat;
EOP
    eval $prog;
    warn "Ouch. Something's badly broken. No can do.\n" if $@;
  }
}    

# Install logeditrc?
if (-r "./sample.logeditrc") {
 $has_logeditrc_char = "y"; # This indicates the default - it feels backward
 $has_logeditrc_char = "n" if (-r "$home/.logeditrc");
 do {
  print "Would you like to install the sample logeditrc file from this release? [$has_logeditrc_char] ";
  chop($yn = <STDIN>);
  $yn = $has_logeditrc_char if ($yn =~ /^$/);
 } while ($yn !~ /^(y|n)/i);
 if ($yn =~ /^y/i) {
  undef $prog;
  if ($has_logeditrc_char eq "n") {
   print "Since I found .logeditrc in $home, I'll rename it .logeditrc.old\n";
   $prog =<<'EOP';
system "$CP $home/.logeditrc $home/.logeditrc.old\n";
EOP
  }
  print "Installing as $home/.logeditrc\n";
  $prog .= <<'EOP';
system "$CP ./sample.logeditrc $home/.logeditrc\n";
EOP
  eval $prog;
  warn "Oops! Something went wrong doing that. You can install the file by hand\n(Error was: $@)\n" if $@;
 }
} 

print "Logedit installation complete!\n";
if ($shell =~ /csh/) {
print "You may have to type 'rehash' at your shell prompt before ";
print "using logedit.\n";
}

exit 0;
