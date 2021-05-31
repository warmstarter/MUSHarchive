#!/usr/local/bin/perl5
#
# Take a file of processed ic logs (on stdin) and split it
# into separate logs, mailing each to an RP admin.
#

# How many logs does each admin get?
my $logs_per_admin = 1;

# Where are the admin addresses? This should be a file of email addresses,
# 1 per line
my $address_file = '/home/mush/admin-addresses';

# Get the addresses
open(IN,$address_file) or die "Can't open $address_file\n";
while (<IN>) {
  next if /^\//;
  next if /^\|/;
  next if /conte/;
  chop;
  push(@admin,$_);
}
close(IN);
my @temp;
push(@temp,splice(@admin,rand(@admin),1)) while @admin;
@admin = @temp;

$/ = "---- Start:";
@logs = <STDIN>; 	# Slurp!
shift(@logs);		# The first is empty, ditch it.
print "Got ",scalar(@logs)," logs\n";

my $adm = 0;
my $per = 0;
foreach (@logs) {
  s#$/##;
  &sendlog($admin[$adm],\$_);
  $adm++;
  if ($adm > $#admin) {
    $adm = 0; 
    $per++;
    last if $per == $logs_per_admin;
  }
}

exit 0;


# Send a log, given an email address and a ref to the log
sub sendlog {
  my($address,$logref) = @_;
  open(OUT,"|-") || exec "/usr/lib/sendmail", "-t";
  print OUT <<EOF;
To: $address
Subject: IC log to review
X-Mailer: split-n-mail.pl

This is an automatically-generated email message. Below you will
find one of the logs which the script has decided is among
the most interesting logs this period. Please read it. :)

What should you do with this log once you read it? Well, here are some 
suggestions:
 * If the logs fits into one of the ongoing plots, post a note about
   it on that plot's rpboard, so we can integrate those actions into
   the plot.
 * If the log suggests a new plot, post a proposal for it on the
   Plots in Planning rpboard
 * \@mail a player a dream which relates to their actions in the
   log. It can be a warped version of the events, a replay,
   stuff to think about, memories, etc. This isn't a mystical vision,
   just a dream (or nightmare!)
 * If players did well, +vote for them and tell them so. If they did really 
   well, grant them an XP and/or nominate them for an RP award.
 * If the log isn't all that great, don't be afraid to do nothing with it.

---- Start: $$logref

EOF
  close(OUT);
}
