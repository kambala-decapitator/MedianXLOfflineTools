#!/usr/bin/perl -w

use strict;
use File::Slurp qw(edit_file edit_file_lines);

die "usage: perl set_new_app_version.pl <Qt project name> <MSVS project name> <version> [info.plist path]" if scalar(@ARGV) < 2;

my $qtProjName = shift;
my $msvsProjName = shift;

my $versionComponents = 4;
my $newVersion = shift;
my @versionNumbers = split /\./, $newVersion;
push @versionNumbers, (0) x ($versionComponents - scalar(@versionNumbers));

# MSVS's .vcxproj
my $newDefinesMsvs = '';
for my $i (1..$versionComponents) { $newDefinesMsvs = $newDefinesMsvs."NVER$i=$versionNumbers[$i-1];" }
$newDefinesMsvs = $newDefinesMsvs."NVER_STRING=\"$newVersion\";";
edit_file { s/NVER1=.+";/$newDefinesMsvs/g } "$msvsProjName";

# QtCreator's .pro
edit_file { for my $i (1..$versionComponents) { s/(?<=NVER$i = ).+/$versionNumbers[$i-1]/ } } "$qtProjName";

# increment info.plist's bundle version
my $infoPlistPath = shift;
if (defined $infoPlistPath)
{
    my $keyFound = 0;
    edit_file_lines
    {
        if ($keyFound)
        {
            $keyFound = 0;

            /\d+/;
            my $oldVersion = $&;
            my $newVersion = $oldVersion + 1;
            s/$oldVersion/$newVersion/
        }
        $keyFound = 1 if /CFBundleVersion/
    } $infoPlistPath
}
