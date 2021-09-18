#!/usr/bin/env perl

# original script copyright (C) grig 2011
# improvements and modifications for Median XL Offline Tools copyright (C) kambala 2011-2021

use strict;
use warnings;
use File::Path qw/make_path/;
use Data::Dumper;

my $locale = $ARGV[0] // 'en', my $tbl;
&readTbl($_) for ('string', 'expansionstring', 'patchstring'); # do not change order of the files!

sub readTbl
{
    my $filename = shift;
    open my $tmpfile, "<", "tbl/$locale/${filename}.txt";
    for (<$tmpfile>) { $tbl->{$1} = $2 if /\"(.+?)\"\t\"(.*)\"$/ }
    close $tmpfile
}

my $realNameField = 'realName';
sub tblExpandArray
{
    (my $itemProperties, my $field, my $newFieldName) = @_;
    $newFieldName = $realNameField unless defined $newFieldName;
    $field = "tbl" unless defined $field;
    for (@$itemProperties)
    {
        $_->{$newFieldName} = $tbl->{$_->{$field}} // $_->{$field} if defined $_->{$field};
        next unless defined $_->{$newFieldName};
        $_->{$newFieldName} =~ s/\</&lt;/g;
        $_->{$newFieldName} =~ s/\>/&gt;/g
    }
}

my ($nameStr, $spellDescStr) = qw/namestr spelldescstr/;
my $itemWithoutName = $tbl->{dummy};
sub tblExpandHash
{
    (my $itemsHash, my $field) = @_;
    for (keys %$itemsHash)
    {
        my ($nameKey, $spellDescKey) = ($itemsHash->{$_}->{$nameStr}, $itemsHash->{$_}->{$spellDescStr});
        $nameKey = $_ unless defined $nameKey; # nameKey is defined only for items
        $itemsHash->{$_}->{$field} = $tbl->{$nameKey} // $itemWithoutName;
        print "'$nameKey' not found in tbl\n" if $itemsHash->{$_}->{$field} eq $itemWithoutName;
        $itemsHash->{$_}->{$spellDescStr} = $tbl->{$spellDescKey} if defined $spellDescKey and defined $tbl->{$spellDescKey}
    }
}

my $zeroRe = qr/^0$/;
my %propertiesStatsHash; $propertiesStatsHash{"stat".($_ + 1)} = 6 + $_ * 4 for (0..6);
my $properties = parsetxt("properties.csv", "#code"=>"0", param1 => 4, %propertiesStatsHash);

my $descArrayRef = [
    {key => "descstrpos", col => 18, expanded => "descPositive"},
    {key => "descstrneg", col => 19, expanded => "descNegative"},
    {key => "descstr2",   col => 20, expanded => "descStringAdd"},
    {key => "dgrpstrpos", col => 50, expanded => "descGroupPositive"},
    {key => "dgrpstrneg", col => 51, expanded => "descGroupNegative"},
    {key => "dgrpstr2",   col => 52, expanded => "descGroupStringAdd"},
];
my %propertiesHash = ("stat"=>"0", bitsSave=>32, bitsParamSave=>33, "bits"=>"5", "add"=>"6", "saveParamBits"=>"7",
                      descpriority=>15, descfunc=>16, descval=>17, dgrp=>47, dgrpfunc=>48, dgrpval=>49);
$propertiesHash{$_->{key}} = $_->{col} for (@$descArrayRef);
my $itemProperties = parsetxt("itemstatcost.csv", _index=>"1", %propertiesHash);
&tblExpandArray($itemProperties, $_->{key}, $_->{expanded}) for (@$descArrayRef);

# add new field for group properties
for (0..scalar @$itemProperties)
{
    my $desc = $itemProperties->[$_]->{descGroupPositive};
    next unless defined $desc;
    my @sameGroupIDs;
    for my $otherID (0..scalar @$itemProperties)
    {
        my $otherDesc = $itemProperties->[$otherID]->{descGroupPositive};
        push(@sameGroupIDs, $otherID) if defined $otherDesc and $otherDesc eq $desc and $otherID != $_
    }
    $itemProperties->[$_]->{descGroupIDs} = join(",", @sameGroupIDs)
}

sub statIdsFromPropertyStat
{
    (my $property, my $classSkillsParam) = @_;
    return undef unless defined $property;

    my $propsStat = undef;
    # handle special cases
    if    ($property =~ /^dmg\-(max|min)$/) { $propsStat = $1."damage" }
    elsif ($property eq "dmg%")             { $propsStat = "item_maxdamage_percent" }
    if (defined $propsStat)
    {
        for (0..scalar @$itemProperties) { return $_ if $itemProperties->[$_]->{stat} eq $propsStat };
        return undef
    }

    my @ids;
    for my $statKey (sort keys %propertiesStatsHash)
    {
        my $stat = $properties->{$property}->{$statKey};
        next unless defined $stat;
        for (0..scalar @$itemProperties)
        {
            my $propStat = $itemProperties->[$_]->{stat};
            if (defined $propStat and $propStat eq $stat)
            {
                push @ids, $_;
                $$classSkillsParam = $properties->{$property}->{param1} if defined $classSkillsParam and $stat eq 'item_addclassskills';
                last
            }
        }
    }
    return join(',', @ids)
}

my $uniques = parsetxt("uniqueitems.csv", _autoindex=>0, iName=>0, rlvl=>5, image=>64);
&tblExpandArray($uniques, "iName");

# any fixed property in .txt is defined with the following set of columns
my @fixedPropertyKeys = qw/prop param min max/;
my $fixedPropertyKeysSize = scalar @fixedPropertyKeys;

sub getSetFixedPropertiesHash
{
    (my $firstColumn, my $propertiesSize, my $hashRef, my $keysRef, my $prependToKey) = @_;
    $prependToKey = '' unless defined $prependToKey;

    for (my $i = 1; $i <= $propertiesSize; $i++)
    {
        my $colStart = $firstColumn + $fixedPropertyKeysSize * ($i - 1);
        for (0 .. $fixedPropertyKeysSize-1)
        {
            my $key = $prependToKey.$fixedPropertyKeys[$_].$i;
            push @$keysRef, $key;
            $hashRef->{$key} = $colStart + $_
        }
    }
}

sub expandSetProperties
{
    (my $keysRef, my $dst, my $src) = @_;
    for (my $i = 0; $i < scalar @$keysRef; $i++)
    {
        my $key = $keysRef->[$i];
        my $convertStatSub = sub {
            my $param;
            $dst->{$key} = &statIdsFromPropertyStat(($src // $dst)->{$key}, \$param);
            $dst->{$keysRef->[$i + 1]} = $param if defined $param # class skills
        };

        # convert property name to property id(s) in each first column
        if (defined $src)
        {
            if ($i % $fixedPropertyKeysSize)
            {
                $dst->{$key} = $src->{$key} unless defined $dst->{$key}
            }
            else
            {
                &$convertStatSub()
            }
        }
        elsif ($i % $fixedPropertyKeysSize == 0) { &$convertStatSub() }
    }
}

my %setsPropertiesHash, my @setsPropertiesKeys;
&getSetFixedPropertiesHash(8,  8, \%setsPropertiesHash, \@setsPropertiesKeys, 'part_'); # partial bonuses
&getSetFixedPropertiesHash(40, 8, \%setsPropertiesHash, \@setsPropertiesKeys, 'full_'); # full ones

my $oldSetCond = qr/^old LoD$/;
my $sets = parsetxt("sets.csv", "#key" => 0, tbl => 1, '!_lodSet' => {col => 2, val => $oldSetCond}, %setsPropertiesHash);
&expandSetProperties(\@setsPropertiesKeys, $sets->{$_}) for (keys %$sets);

my %greenPropertiesHash, my @greenPropertiesKeys;
&getSetFixedPropertiesHash(57, 10, \%greenPropertiesHash, \@greenPropertiesKeys);
my $setItems = parsetxt("setitems.csv", _autoindex=>0, iIName=>0, setKey=>1, rlvl=>8, image=>87, addfunc=>94,
                        %greenPropertiesHash, '!_lodSet' => {col => 6, val => $oldSetCond});
&tblExpandArray($setItems, "iIName", "IName");

my $setTblKey = 'setTblKey';
for (@$setItems)
{
    $_->{$setTblKey} = $sets->{$_->{setKey}}->{tbl} if defined $_->{setKey};
}
&tblExpandArray($setItems, $setTblKey, "SName");

my $mxlSets, my $setIndex = 0, my $oldIndex = -3;
my @setFields = qw/IName SName setKey rlvl image/;
for my $setItem (@$setItems)
{
    $oldIndex++;
    next unless (defined $setItem->{IName} and defined $setItem->{SName});

    $mxlSets->[$setIndex]->{index} = $oldIndex;
    my $newSetItem = $mxlSets->[$setIndex];
    $newSetItem->{$_} = $setItem->{$_} for (@setFields);

    # if addfunc == 0, then properties are embedded in the item
    my $addfunc = $setItem->{addfunc};
    &expandSetProperties(\@greenPropertiesKeys, $newSetItem, $setItem) if defined $addfunc and $addfunc > 0;

    $setIndex++
}

my $itemName = 'name';
my $armorTypes = parsetxt("armor.csv", $itemName=>1, "#code"=>0, $nameStr=>21, w=>31, h=>32, type=>2,
                          type2=>3, rlvl=>18, image=>37, rstr=>12, rdex=>13);
my $weaponTypes = parsetxt("weapons.csv", $itemName=>1, "#code"=>0, $nameStr=>5, w=>43, h=>44, type=>2,
                           type2=>3, stackable=>45, rlvl=>30, rstr=>25, rdex=>26, image=>50, quest=>67,
                           '1hMinDmg'=>12, '1hMaxDmg'=>13, '2hMinDmg'=>16, '2hMaxDmg'=>17, throwMinDmg=>18, throwMaxDmg=>19,
                           '1h2h'=>14, '2h'=>15, strBonus=>23, dexBonus=>24);
my $miscTypes = parsetxt("misc.csv", $itemName=>0, "#code"=>5, $nameStr=>7, $spellDescStr=>69,
                         w=>24, h=>25, type=>8, type2=>9, stackable=>48, rlvl=>13, image=>30, quest=>52);
&tblExpandHash($_, $itemName) for ($armorTypes, $weaponTypes, $miscTypes);

my $skills = parsetxt("skills.csv", _index=>"1", "dbgname"=>"0", "internalName"=>"3", "class"=>"2",
                      "srvmissile"=>"9", "srvmissilea"=>"12", "srvmissileb"=>"13", "srvmissilec"=>"14");
my $skillsDsc = parsetxt("skilldesc.csv", "#code"=>0, tab => 1, row => 2, col => 3, image => 8, dscname=>9);

my $processedSkills;
my $index = -1;
my %classes = (ama => 0, sor => 1, nec => 2, pal => 3, bar => 4, dru => 5, ass => 6);
foreach my $elem (@$skills)
{
    $index++;

    $processedSkills->[$index]->{iname1} = $elem->{dbgname};
    $processedSkills->[$index]->{class} = (defined $elem->{class} and defined $classes{$elem->{class}}) ? $classes{$elem->{class}} : -1;
    next unless defined $elem->{internalName};

    my $desc = $skillsDsc->{$elem->{internalName}};
    $processedSkills->[$index]->{iname} = $desc->{dscname} if defined $desc->{dscname};
    $processedSkills->[$index]->{tab}   = $desc->{tab};
    $processedSkills->[$index]->{row}   = $desc->{row};
    $processedSkills->[$index]->{col}   = $desc->{col};
    $processedSkills->[$index]->{image} = $desc->{image}
}
&tblExpandArray($processedSkills, "iname", "name");

my $monstats = parsetxt("monstats.csv", _autoindex=>"0", $nameStr=>6);
&tblExpandArray($monstats, $nameStr);

# RW
# dummy key $itemName is used when writing to file
my %rwKeysHash = ($itemName => undef);
for (1..6) {
    $rwKeysHash{"allowedType$_"} = $_ + 2;
    $rwKeysHash{"rune$_"} = $_ + 11
}
# $rwKeysHash{"disallowedType$_"} = $_ + 8 for (1..3);
my $rw = parsetxt("runes.csv", _autoindex => 0, tbl => 0, '!_enabled' => {col => 1, val => $zeroRe},
                  '!_rune' => {col => 12, val => qr/^jew$/}, %rwKeysHash);
push(@$rw, {tbl => '09This', allowedType1 => 'weap', allowedType2 => 'armo', rune1 => 'jew'}); # yeah, it's a hack (jewelword)
# fake hash to collect names from tbl
my $fakeRwHash;
foreach my $elem (@$rw)
{
    $fakeRwHash->{$elem->{tbl}} = {} if defined $elem
}
&tblExpandHash($fakeRwHash, $itemName);

# gems & runes
my @subtypes = ('weapon', 'armor', 'shield');
my @subtypeKeys = ('code', 'param', 'value');
my $firstColumn = 5; # weaponMod1Code
my %gemStatsHash;
for my $subtype (@subtypes)
{
    for my $i (1..3)
    {
        for my $key (@subtypeKeys) { $gemStatsHash{$subtype.$i.$key} = $firstColumn++ }
        $firstColumn++ # skip 'max'
    }
}
my $gems = parsetxt("gems.csv", $itemName => 0, 'letter' => 1, "#code" => 3, %gemStatsHash);
&tblExpandHash($gems, $itemName);
if ($locale ne 'en')
{
    # translate rune letters
    my $runesFile = "tbl/$locale/runes.txt";
    if (open my $runeLetters, "<", $runesFile)
    {
        my %letters;
        while(<$runeLetters>) { $letters{$1} = $2 if /^(\w+)\t(.+)$/ }
        for (keys %$gems)
        {
            my $letter = $gems->{$_}->{letter};
            $gems->{$_}->{letter} = $letters{$letter} if defined $letter and defined $letters{$letter}
        }
    }
    else { print "inserted runes will stay in English - failed to open $runesFile: $!\n" }
}

# generate
my $prefix = "generated/$locale";
make_path $prefix;

my %invgfx; $invgfx{"invgfx$_"} = 34 + $_ for (1..6);
my @invgfxKeys = sort keys %invgfx;
my $itemTypes = parsetxt("itemtypes.csv", "#code"=>0, code0=>1, equiv1=>8, equiv2=>9, bodyLoc=>12, class=>33, %invgfx);
open my $out, ">", "generated/itemtypes.csv";
print $out "#code\tequiv\tvarImages\tname\n";
for my $name (sort keys %$itemTypes)
{
    my $hashRef = $itemTypes->{$name};
    next unless defined $hashRef->{equiv1} and defined $hashRef->{code0};
    print $out $hashRef->{code0}, "\t", $hashRef->{equiv1};
    print $out ",", $hashRef->{equiv2} if defined $hashRef->{equiv2};

    my @varImages;
    for (@invgfxKeys)
    {
        my $varImage = $hashRef->{$_};
        last unless defined $varImage;
        push @varImages, $varImage;
    }
    print $out "\t", join(",", @varImages), "\t$name\n"
}
close $out;

open $out, ">", "$prefix/items.csv";
print $out "#code\tname\t$spellDescStr\twidth\theight\tgentype\tstackable\trlvl\trstr\trdex\t1h2h\t2h\t";
print $out "1hMinDmg\t1hMaxDmg\t2hMinDmg\t2hMaxDmg\tthrowMinDmg\tthrowMaxDmg\timage\tquest\tstrBonus\tdexBonus\t";
print $out "type\tsockettype\tclass\n"; # these columns are treated specially
my $itemType = 0;
for my $ref ($armorTypes, $weaponTypes, $miscTypes)
{
    for my $itemCode (sort keys %$ref)
    {
        my $hashRef = $ref->{$itemCode};
        next unless defined $hashRef->{w};

        my $type = $hashRef->{type};
        printf $out "%s\t%s\t%s\t%d\t%d\t%s\t%s\t%d\t%d\t%d\t%s\t%s\t", $itemCode, $hashRef->{$itemName},
            ($hashRef->{$spellDescStr} // ''), $hashRef->{w}, $hashRef->{h}, $itemType,
            (defined $hashRef->{stackable} and $hashRef->{stackable} > 0 ? $hashRef->{stackable} : ''),
            ($hashRef->{rlvl} // 0), ($hashRef->{rstr} // 0), ($hashRef->{rdex} // 0),
            ($hashRef->{'1h2h'} // ''), ($hashRef->{'2h'} // '');
        printf $out "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t", ($hashRef->{'1hMinDmg'} // ''), ($hashRef->{'1hMaxDmg'} // ''),
            ($hashRef->{'2hMinDmg'} // ''), ($hashRef->{'2hMaxDmg'} // ''), ($hashRef->{throwMinDmg} // ''),
            ($hashRef->{throwMaxDmg} // ''), $hashRef->{image}, ($hashRef->{quest} // ''),
            ($hashRef->{strBonus} // ''), ($hashRef->{dexBonus} // '');

        print $out $type;
        print $out ",", $hashRef->{type2} if defined $hashRef->{type2};
        print $out "\t";

        for my $itemName (keys %$itemTypes) # find which class it belongs to
        {
            my $hashRef = $itemTypes->{$itemName};
            if ($hashRef->{code0} eq $type)
            {
                # to determine item type when parsing socketables (see @subtypes): 0 - shield, 1 - weapon, nothing - armor
                print $out $itemType if defined $hashRef->{bodyLoc} and $hashRef->{bodyLoc} eq 'rarm';
                print $out "\t".(defined $hashRef->{class} ? $classes{$hashRef->{class}} : -1)."\n";
                last
            }
        }
    }
    $itemType++
}
close $out;

open $out, ">", "$prefix/uniques.csv";
print $out "#index\titem\trlvl\timage\n";
my $count = -1;
for my $hashRef (@$uniques)
{
    my $realName = $hashRef->{$realNameField};
    printf $out "%d\t%s\t%d\t%s\n", $count, ($realName // ''), ($hashRef->{rlvl} // 0),
        ($hashRef->{image} // '') if defined $realName;
    $count++
}
close $out;

open $out, ">", "generated/sets.csv";
print $out "#index\t".join("\t", @setsPropertiesKeys)."\n";
for my $setKey (sort keys %$sets)
{
    print $out $setKey;
    print $out "\t".($sets->{$setKey}->{$_} // '') for (@setsPropertiesKeys);
    print $out "\n"
}
close $out;

open $out, ">", "$prefix/setitems.csv";
print $out "#index\titem\tset\tkey\trlvl\timage\t".join("\t", @greenPropertiesKeys)."\n";
for my $set (@$mxlSets)
{
    printf $out "%d\t%s\t%s\t%s\t%d\t%s", $set->{index}, $set->{IName}, $set->{SName},
        $set->{setKey}, ($set->{rlvl} // 0), ($set->{image} // '');
    print $out "\t".($set->{$_} // '') for (@greenPropertiesKeys);
    print $out "\n"
}
close $out;

$count = -1;
my @skillKeys = qw/name class tab row col image/;
open $out, ">", "$prefix/skills.csv";
print $out "#code\t".join("\t", @skillKeys)."\n";
for my $hashRef (@$processedSkills)
{
    $count++;
    print $out $count;
    print $out "\t".($hashRef->{$_} // '') for (@skillKeys);
    print $out "\n"
}
close $out;

# don't need the keys any more, but need tbl strings
for (@$descArrayRef)
{
    delete $propertiesHash{$_->{key}};
    $propertiesHash{$_->{expanded}} = undef;
    $propertiesHash{descGroupIDs} = undef
}

my @propKeys = sort keys %propertiesHash;
open $out, ">", "$prefix/props.csv";
print $out "#code\t".join("\t", @propKeys)."\n";
$count = -1;
for my $hashRef (@$itemProperties)
{
    $count++;
    next unless defined $hashRef->{bits} or defined $hashRef->{bitsSave};
    print $out $count;
    print $out "\t".($hashRef->{$_} // '') for (@propKeys);
    print $out "\n"
}
close $out;

open $out, ">", "$prefix/monsters.csv";
print $out "#index\tname\n";
$count = -2;
for (@$monstats)
{
    print $out "$count\t$_->{$realNameField}\n" if defined $_->{$realNameField};
    $count++
}
close $out;

my @rwKeys = sort keys %rwKeysHash;
open $out, ">", "$prefix/rw.csv";
print $out "#".join("\t", @rwKeys)."\n";
foreach my $elem (@$rw)
{
    next unless defined $elem;
    for (@rwKeys)
    {
        my $s = $_ eq $itemName ? $fakeRwHash->{$elem->{tbl}}->{$itemName} : $elem->{$_};
        print $out ($s // '')."\t"
    }
    print $out "\n"
}
close $out;

my @gemKeys = sort keys %gemStatsHash;
open $out, ">", "$prefix/socketables.csv";
print $out "#code\t$itemName\tletter\t".join("\t", @gemKeys)."\n";
for my $gemCode (sort keys %$gems)
{
    next if length $gemCode == 0; # skip 'Expansion'
    my $hashRef = $gems->{$gemCode};
    print $out "$gemCode\t$hashRef->{$itemName}\t".($hashRef->{letter} // '');
    for (my $i = 0; $i < scalar @gemKeys; $i++)
    {
        my $value = $hashRef->{$gemKeys[$i]};
        if ($gemKeys[$i] =~ /code/)
        {
            if (!defined $value or $value eq 'hp/paragon' or $value eq 'ac/runemaster')
            {
                print $out "\t\t\t";
                $i += 2; # skip param and value
                next
            }

            $value = &statIdsFromPropertyStat($value)
        }
        print $out "\t".($value // '')
    }
    print $out "\n"
}
close $out;

my @baseStatsKeys = qw/strength dexterity energy vitality stamina lifePerLevel staminaPerLevel manaPerLevel lifePerPoint staminaPerPoint manaPerPoint/;
my %baseStatsParams = (_autoindex => 0, stamina => 6, '!_class' => {col => 0, val => 'Expansion'});
$baseStatsParams{$baseStatsKeys[$_]} = $_ + 1  for (0..3);
$baseStatsParams{$baseStatsKeys[$_]} = $_ + 13 for (5..10);
my $baseStats = parsetxt("charstats.csv", %baseStatsParams);

open $out, ">", "generated/basestats.csv";
print $out "#classcode\t".join("\t", @baseStatsKeys)."\n";
my $classCode = 0;
for my $i (0..scalar @$baseStats)
{
    next unless defined $baseStats->[$i];
    print $out $classCode;
    print $out "\t", $baseStats->[$i]->{$_} for (@baseStatsKeys);
    print $out "\n";
    $classCode++
}
close $out;

# open my $out, ">", "d2items.pm";
# print $out saveStructure($miscTypes, "itemMisc", 80);
# print $out saveStructure($weaponTypes, "itemWeapons", 80);
# print $out saveStructure($armorTypes, "itemArmors", 80);
# print $out saveStructure($itemProperties, "itemProps", 80);
# print $out saveStructure($uniques, "uniques", 80);
# print $out saveStructure($setItems, "sets", 80);
# print $out saveStructure($processedSkills, "skills", 80);
# print $out saveStructure($properties, "properties", 80);
# print $out saveStructure($itemTypes, "itemTypes", 80);
# close $out;

sub saveStructure
{
    my $parsed = shift;
    my $name = shift;
    my $lineLen = shift;
    $lineLen = 80 unless defined $lineLen;

    my @tmp = split(/\n/, Dumper($parsed));
    my $output = "";
    my $line = "";
    my $firstLine = 1;
    foreach my $elem(@tmp)
    {
        $elem = trim($elem);
        if ($firstLine)
        {
            if ($elem =~ /(\$VAR[\d]+)\s*=(.*)/)
            {
                #print "MATCH!:$elem";
                $elem = 'our $'.$name."=".$2 if defined($name);
            }
            else
            {
                #print "NO MATCH!:$elem";
            }
        }
        $firstLine=0;
        if (length($line.$elem) > $lineLen)
        {
            $output.=$line."\n";
            $line = $elem;
        }
        else
        {
            $line.=$elem;
        }
    }
    if (length($line)>0)
    {
        $output.=$line."\n";
    }
    return "$output\n";
}
sub _log
{
    #print @_;
}
sub parsetxt
{
    #keys interpretation: _id !nonull hash !_{skip if matches} #_{add to hash only if !defined $key}
    my ($fname, %structure) = @_;
    $fname = "txt/$fname";
    open my $fh, "<", $fname or return undef;
    my $parsed;

    my $count = -1;
    my $firstLine = 1;
    foreach my $line (<$fh>)
    {
        chomp $line;
        $count++;
        my @cols = split(/\t/, $line);
        _log ("$fname: $count: ".(join",", @cols));
        if (!$firstLine) # skip the table header
        {
            # check if !fields are satisfied (!field says NOT NULL)
            my $ok = 1;
            foreach my $key(keys %structure)
            {
                if ($key =~ /^\!\_/)
                {
                    my $hashRef = $structure{$key};
                    my $s = $cols[$hashRef->{col}];
                    $ok = 0 if (defined $s) and ($s =~ /$hashRef->{val}/);
                    _log ("BAD - !$key");
                }
                elsif ($key =~ /^\!/)
                {
                    my $s = $cols[$structure{$key}];
                    $ok = 0 if (!defined $s) or (length $s == 0);
                    _log ("BAD - !$key");
                }
            }

            #print "line[$count]:ok=$ok -> ";
            if ($ok == 1)
            {
                my $indexValue = undef;
                my $subHash = undef;
                # search for a unique key if required
                foreach my $key(keys %structure)
                {
                    if ($key =~ /^\_auto/)
                    {
                        $indexValue = $count;
                        $ok = 0 unless defined $indexValue;
                        _log ("BAD1 - indexValue for $key") unless $ok;
                    }
                    elsif ($key =~ /^\_/)
                    {
                        $indexValue = $cols[$structure{$key}];
                        $ok = 0 unless defined $indexValue;
                        _log ("BAD2 - indexValue for $key") unless $ok;
                    }
                }
                my $addOnce = 0;
                foreach my $key(keys %structure)
                {
                    $addOnce = 1 if $key =~ /^\#\_/;
                    if ($key =~ /^\#/)
                    {
                        $subHash = $cols[$structure{$key}];
                        $ok = 0 unless defined $subHash;
                        _log ("BAD3 - subhash for $key=$structure{$key}") unless $ok;
                    }
                }

                # add the record
                if ($ok == 1)
                {
                    foreach my $key(keys %structure)
                    {
                        next if $key =~ /^\_/ or $key =~ /^\#/ or $key =~ /^\!\_/ or !defined $structure{$key};
                        my $fieldName = $key;
                        if ($fieldName =~ /^\!(.*)/)
                        {
                            $fieldName = $1;
                        }
                        my $s = $cols[$structure{$key}];
                        # _log("-> $fieldName<$structure{$key}>: '$s'\n");
                        if (defined $s and length $s > 0)
                        {
                            if (defined $indexValue)
                            {
                                $parsed->[$indexValue]->{$fieldName} = $s;
                                _log("\tat $indexValue:1: $fieldName = $s\n");
                            }
                            else
                            {
                                if (defined $subHash)
                                {
                                    $subHash = $1 if $subHash =~ /^\"(.+)\"$/; # stupid Excel may add double-quotes
                                    $parsed->{$subHash}->{$fieldName} = $s if ($addOnce == 0 or ($addOnce > 0 and !defined $parsed->{$subHash}->{$fieldName}));
                                    _log("\tat $subHash:2: $fieldName = $s\n");
                                }
                                else
                                {
                                    $parsed->{$fieldName} = $s;
                                    _log("\tat $fieldName:3: $s\n");
                                }
                            }
                        }
                    }
                }
            }
            _log ("\n");
        }
        $firstLine = 0;
    }
    close $fh;
    return $parsed;
}

sub trim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    $string =~ s/\s+$//;
    return $string;
}
# Left trim function to remove leading whitespace
sub ltrim($)
{
    my $string = shift;
    $string =~ s/^\s+//;
    return $string;
}
# Right trim function to remove trailing whitespace
sub rtrim($)
{
    my $string = shift;
    $string =~ s/\s+$//;
    return $string;
}
