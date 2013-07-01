#!/usr/bin/perl -w

use strict;
use File::Slurp qw(edit_file);

die "usage: perl set_new_app_version.pl <project name> <version>" if scalar(@ARGV) < 2;
chdir '..';

my $projName = shift;

my $versionComponents = 4;
my $newVersion = shift;
my @versionNumbers = split /\./, $newVersion;
push @versionNumbers, (0) x ($versionComponents - scalar(@versionNumbers));

# MSVS's .vcxproj
my $newDefinesMsvs = '';
for my $i (1..$versionComponents) { $newDefinesMsvs = $newDefinesMsvs."NVER$i=$versionNumbers[$i-1];" }
$newDefinesMsvs = $newDefinesMsvs."NVER_STRING=\"$newVersion\";";
edit_file { s/NVER1=.+";/$newDefinesMsvs/g } "$projName.vcxproj";

# QtCreator's .pro
edit_file { for my $i (1..$versionComponents) { s/(?<=NVER$i = )\d/$versionNumbers[$i-1]/ } } "$projName.pro";
