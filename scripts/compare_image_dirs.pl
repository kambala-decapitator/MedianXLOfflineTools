#!/usr/bin/env perl

use warnings;
use strict;
use 5.012; # so readdir assigns to $_ in a lone while test
use File::Copy;

die "invalid args" if @ARGV != 2;
(my $pngDir, my $dc6Dir) = @ARGV;
opendir(my $dh, $dc6Dir) || die "error opening dc6 dir: $!";

my $newDir = "$dc6Dir/new";
mkdir $newDir;

chdir $dc6Dir;
while (readdir $dh) {
	next unless /^(.+)\.dc6$/;
	copy($_, $newDir) unless -e "$pngDir/$1.png";
}
closedir $dh;
