#!/usr/bin/perl -w

use strict;

my @names = qw/gema gems geme gemr gemd gemt gemz gemx gemq gemp gemo geml/;
my $grade = 0;
my $i = 0;

open my $f, '<', 'd:\Games\Installed\Diablo II\MODS\Median XL - Ultimative\save\necrofaggot.d2x' or die "can't open file: $!";
binmode $f;
seek $f, 3398, 0;
while (!eof $f)
{
    seek $f, 7, 1; # set cursor to beginning of item data by skipping page header
    my $itemData;
    read $f, $itemData, 14;

    open my $out, '>', "$names[$i]${grade}.d2i";
    binmode $out;
    print $out $itemData;

    $grade++;
    if ($grade == 5)
    {
        $grade = 0;
        $i++;
        last if $i == scalar @names;
    }
}
