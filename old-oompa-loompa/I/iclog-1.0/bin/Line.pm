#!/usr/local/bin/perl5 -w
#
# Line: methods for working with a line of conversation
#
package Line;
require 5.003;
use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK);
use Carp;
require Exporter;
@ISA       = qw(Exporter);
@EXPORT    = qw();
@EXPORT_OK = qw();

# Create a new Line object from a string of the form:
# 
# 01/30 15:37:49  [7/45/854667469] Li Erh has arrived.
# The numbers in brackets are player, room, mudtime
#
# A line is a ref to a hash containing these keys:
#   speaker
#   time
#   text
# Usage: Line->new(<line>)
sub new {
    my $type = shift;
    my $string = shift;
    chomp($string);
    $string =~ m#^.*?\[(\d+)/(\d+)/(\d+)\] (.*)#;
    my ($speaker,$room,$time,$text) = ($1,$2,$3,$4);
    my $self = { 'speaker' => $speaker, 'time' => $time, 'text' => $text,
		'room' => $room };
    bless $self, $type;
}

# Return the speaker, time, or text
sub speaker { my $self = shift; return $self->{'speaker'}; }
sub time { my $self = shift; return $self->{'time'}; }
sub text { my $self = shift; return $self->{'text'}; }
sub room { my $self = shift; return $self->{'room'}; }

# Return the number of words in the text
sub length {
	my $self = shift;
	my $text = $self->text();
	my @words = split(" ",$text);
	return scalar(@words);
}

1;
