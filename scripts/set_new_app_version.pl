#!/usr/bin/perl -w

use strict;
use File::Slurp qw(edit_file);

die "new version wasn't passed" if scalar(@ARGV) == 0;
chdir '..';

my $digits = 4;
my $newVersion = shift;
my @versionNumbers = split /\./, $newVersion;
push @versionNumbers, (0) x ($digits - scalar(@versionNumbers));

# MSVS's .vcxproj
my $newDefinesMsvs = '';
for my $i (1..$digits) { $newDefinesMsvs = $newDefinesMsvs."NVER$i=$versionNumbers[$i-1];" }
$newDefinesMsvs = $newDefinesMsvs."NVER_STRING=\"$newVersion\";";
edit_file { s/NVER1=.+";/$newDefinesMsvs/g } 'MedianXLOfflineTools.vcxproj';

# QtCreator's .pro
edit_file { for my $i (1..$digits) { s/(?<=NVER$i = )\d/$versionNumbers[$i-1]/ } } 'MedianXLOfflineTools.pro';
