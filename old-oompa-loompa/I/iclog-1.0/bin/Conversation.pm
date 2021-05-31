#!/usr/local/bin/perl5 -w
#
# Conversation: methods for working with Conversation objects
#
package Conversation;
require 5.003;
use strict;
use vars qw(@ISA @EXPORT @EXPORT_OK);
use Carp;
use Line;
require Exporter;
@ISA       = qw(Exporter);
@EXPORT    = qw();
@EXPORT_OK = qw();

sub log2;

# Create a new Conversation object from a list of Lines
# A Conversation is a ref to a hash:
#   lines -> ref to a list of Lines
#   score -> an interestingness score
# Usage: Conversation->new(@lines)
sub new {
    my $type = shift;
    my @lines = @_;
    my $self = { 'lines' => \@lines };
    bless $self, $type;
    $self->{'start_time'} = $self->{'lines'}->[0]->time;
    $self->{'players'} = [ $self->count_players ];
    $self->{'score'} = $self->compute_interestingness;
    return $self;
}

# Return the conversation as an array of Lines
sub raw {
   my $self = shift;
   return @{ $self->{'lines'} };
}

sub interestingness {
   my $self = shift;
   return $self->{'score'};
}

sub start_time {
   my $self = shift;
   return $self->{'start_time'};
}

sub players {
   my $self = shift;
   return @{ $self->{'players'} };
}

# Print the text of the conversation
sub print {
  my $self = shift;
  my $fh = shift;
  my @lines = $self->raw;
  foreach (@lines) {
    print $fh $_->text,"\n";
  }
}


# The length of the conversation
sub length {
   my $self = shift;
   return scalar($self->raw());
}

# Return a list of players in the conversation
sub count_players {
   my $self = shift;
   my @lines = $self->raw;
   my $line;
   my %speakers;
   foreach $line (@lines) {
	$speakers{$line->speaker()}++;
   }
   return keys %speakers;
}

# The number of players in the conversation
sub num_speakers {
   my $self = shift;
   return scalar @{ $self->{'players'} };
}

# The length, in words, of the longest utterance in the conversation
sub max_words {
   my $self = shift;
   my @lines = $self->raw;
   my $line;
   my $max = 0;
   foreach $line (@lines) {
	$max = $line->length() if $line->length() > $max;
   }
   return $max;
}

# How interesting is the conversation? This is the tricky part
# We do it by tossing some 10-sided dice. The number of dice
# we toss is:
#   (# of speakers) + (log2 length in lines) + (log2 longest utterance)
sub compute_interestingness {
  my $self = shift;
  my $dice = 
    $self->num_speakers + log2($self->length) + log2($self->max_words);
  $dice = int($dice);
  my $sum = 0;
  foreach (1..$dice) {
	$sum += int(rand(10));
  }
  return $sum;
}


# Log base 2
sub log2 {
  my $x = shift;
  return $x ? log($x)/log(2) : 0;
}

1;
