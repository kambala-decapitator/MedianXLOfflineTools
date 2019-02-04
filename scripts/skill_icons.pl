#!/usr/bin/env perl

use warnings;
use strict;
use 5.012; # so readdir assigns to $_ in a lone while test
use File::Basename;
use File::Copy;

my $skillIconsDir = $ARGV[0];
opendir(my $dh, $skillIconsDir) || die "error opening dir: $!";
chdir $skillIconsDir;

my %characterIndexes = (ama => 0, sor => 1, nec => 2, pal => 3, bar => 4, dru => 5, ass => 6);
while (readdir $dh) {
    next unless /^([a-z]{3})_icons_/;
    my $idx = $characterIndexes{$1};
    my $outDir = "$skillIconsDir/$idx";
    mkdir $outDir;
    
    while (my $icon = <$skillIconsDir/$_/00/*>) {
        my $newName = basename($icon);
        next if $newName =~ /[13579]\./;
        $newName =~ s/^0{0,3}//;
        copy($icon, "$outDir/$newName");
    }
}
closedir $dh;
