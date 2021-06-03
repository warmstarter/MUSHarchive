#!/usr/bin/perl --

use strict;

## log_cleanup ("micro").  2007/07/04 by Azundris.
## minimal log clean-up. you probably want to use log_process instead.
##
## invocation:  ./log_cleanup < logfile > outfile
##

## read in paragraph mode -- this is necessary if the client word-wraps the
## logs; if it doesn't and one pose is one line in the log (regardless of
## length), put a # in front of the following line to disable it:
$/='';

## no user-configurables after this point.



## ----------------------------------------------------------------------------

## no previous paragraph yet, set to empty string
my $p="";

## line counter
my $c=0;

while(<>) {
  ## removing trailing newlines
  chomp;
  chomp;

  ## remove line-breaks in the middle of poses/paragraphs
  s/\n/ /sg;

  ## remove extra spaces
  s/\s{2,}/ /g;
  s/^(.*?)[\t ]+$/$1/g;



  ## if this paragraph is a duplicate of the previous one, discard it
  if ( $p eq $_ ) {
    next; }



  ## technical feedback from the game server
  if ( $_ =~ m/^GAME: .*$/ ) {
    next; }

  if ( $_ =~ m/^Huh\?  \(Type "help" for help\.\)/ ) {
    next; }



  ## output multi-descer ("You changed your description")
  if ( $_ =~ m/^Excellent choice!/ ) {
    next; }



  ## remember this paragraph so we can catch duplicates
  $p=$_;

  print $_."\n\n";

  $c++; }
