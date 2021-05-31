#!/usr/bin/perl

use warnings;
use strict;
$|++;


my @directories = ();

LINE:
while (my $line = <>) {
	chomp $line;
	$line =~ s/^\s+//;
	$line =~ s/\s+$//;
	next LINE if ($line eq '');
	next LINE if ($line =~ m/^#/);
	my ($path, $count, $age) = split(/:/,$line);

	die  "Record missing path '$line'.\n"   unless ($path);
	die  "Record missing count '$line'.\n"  unless ($count);
	die  "Record missing age '$line'.\n"    unless ($age);
	die  "Not a directory: '$path'.\n"      unless (-d $path);
	die  "Invalid count '$count'.\n"        unless ($count > 0);
	die  "Invalid age '$age'.\n"            unless ($age > 0);

	my $dir_h;
	opendir($dir_h, $path) or die "Failed to read directory '$path': $!\n";
	my @files = read_dir($path);

	push @directories,  {
		'path'		=>	$path,
		'count'		=>	$count,
		'age'		=> 	$age,
		'contents'	=>  \@files,
		};
	
}

@directories = sort { $a->{'age'} <=> $b->{'age'} } @directories;

my $now = time();

for (my $i = 0; $i < @directories; $i++) {
	if ($i + 1 < @directories) {
		# We have another directory to potentially
		# move files into
		my $dest_time = (@{$directories[$i + 1]->{'contents'}})
			? $directories[$i + 1]->{'contents'}->[-1]->{'mtime'} 
			: 0;
		my $cutoff = $dest_time + $directories[$i + 1]->{'age'};
		if ($directories[$i]->{'contents'}->[-1]->{'mtime'} > $cutoff) {
			# The newest file in this directory is newer than the newest file
			# in the target dir + the age cutoff
			my $src_file  = $directories[$i    ]->{'path'}
				. '/'
				. $directories[$i]->{'contents'}->[-1]->{'file'};
			my $dest_file = $directories[$i + 1]->{'path'}
				. '/'
				. $directories[$i]->{'contents'}->[-1]->{'file'};
			# Destination changed, reread it
			link $src_file,$dest_file;
			my @new_dest = read_dir($directories[$i + 1]->{'path'});
			$directories[$i + 1]->{'contents'} = \@new_dest;
		}
	}

	if (@{$directories[$i]->{'contents'}} > $directories[$i]->{'count'}) {
		my $difference = @{$directories[$i]->{'contents'}} - $directories[$i]->{'count'};
		print "I have " . @{$directories[$i]->{'contents'}} . " when I want  $directories[$i]->{'count'}\n";
		unlink map {
				$directories[$i]->{'path'}
				. '/'
				. $_->{'file'}
			} @{$directories[$i]->{'contents'}}[0..($difference - 1)];
	}
}

sub read_dir {
	my ($path) = @_;
	my $dir_h;
	opendir($dir_h, $path) or die "Failed to read directory '$path': $!\n";
	return 
		sort {
			$a->{'mtime'} <=> $b->{'mtime'}
		}
		map {
			my @fstat = stat("$path/$_");
			{
				'file'	=>	$_,
				'mtime'	=>  $fstat[9],
			}
		}
		grep {
			$_ ne '.' && $_ ne '..'
		} readdir($dir_h);
}
