#!/usr/bin/perl

# original script copyright (C) grig 2011
# improvements and modifications for Median XL Offline Tools copyright (C) kambala 2011-2012

use strict;
use warnings;
use File::Path qw/make_path/;
# use Data::Dumper;

my $locale = $ARGV[0] // 'en', my $tbl;
&readTbl($_) for ('string', 'expansionstring', 'patchstring');

sub readTbl
{
    my $filename = shift;
    open my $tmpfile, "<", "tbl/$locale/${filename}.txt";
    for (<$tmpfile>) { $tbl->{$1} = $2 if /\"([^\"]+)\"\t\"([^\"]*)\"$/ }
    close $tmpfile;
}

my $realNameField = 'realName';
sub tblExpandArray
{
    (my $itemProperties, my $field, my $newFieldName) = @_;
    $newFieldName = $realNameField unless defined $newFieldName;
    $field = "tbl" unless defined $field;
    for (@$itemProperties) { $_->{$newFieldName} = $tbl->{$_->{$field}} // $_->{$field} if defined $_->{$field} }
}

my ($nameStr, $spellDescStr) = qw/namestr spelldescstr/;
sub tblExpandHash
{
    (my $itemsHash, my $field) = @_;
    for (keys %$itemsHash)
    {
        my ($nameKey, $spellDescKey) = ($itemsHash->{$_}->{$nameStr}, $itemsHash->{$_}->{$spellDescStr});
        $nameKey = $_ unless defined $nameKey; # nameKey is defined only for items
        $itemsHash->{$_}->{$field} = $tbl->{$nameKey} // 'NO NAME';
        $itemsHash->{$_}->{$spellDescStr} = $tbl->{$spellDescKey} if defined $spellDescKey and defined $tbl->{$spellDescKey};
    }
}

my $zeroRe = qr/^0$/;
my %propertiesStatsHash; $propertiesStatsHash{"stat".($_ + 1)} = 6 + $_ * 4 for (0..6);
my $properties = parsetxt("properties.txt", "#code"=>"0", %propertiesStatsHash, '!_enabled' => {col => 1, val => $zeroRe});

my $descArrayRef = [
{key => "descstrpos", col => 43, expanded => "descPositive"},
{key => "descstrneg", col => 44, expanded => "descNegative"},
{key => "descstr2",   col => 45, expanded => "descStringAdd"},
{key => "dgrpstrpos", col => 49, expanded => "descGroupPositive"},
{key => "dgrpstrneg", col => 50, expanded => "descGroupNegative"},
{key => "dgrpstr2",   col => 51, expanded => "descGroupStringAdd"}
];
my %propertiesHash = ("stat"=>"0", bitsSave=>10, "bits"=>"22", "add"=>"23", "saveParamBits"=>"24", descpriority=>40, descfunc=>41, descval=>42, dgrp=>46, dgrpfunc=>47, dgrpval=>48); $propertiesHash{$_->{key}} = $_->{col} for (@$descArrayRef);
my $itemProperties = parsetxt("itemstatcost.txt", _index=>"1", %propertiesHash);
&tblExpandArray($itemProperties, $_->{key}, $_->{expanded}) for (@$descArrayRef);

# add new field for group properies
for (0..scalar @$itemProperties)
{
    my $desc = $itemProperties->[$_]->{descGroupPositive};
    next unless defined $desc;
    my @sameGroupIDs;
    for my $otherID (0..scalar @$itemProperties)
    {
        my $otherDesc = $itemProperties->[$otherID]->{descGroupPositive};
        push(@sameGroupIDs, $otherID) if defined $otherDesc and $otherDesc eq $desc and $otherID != $_;
    }
    $itemProperties->[$_]->{descGroupIDs} = join(",", @sameGroupIDs);
}

sub statIdsFromPropertyStat
{
    (my $property, my $getAllStats) = @_;
    $getAllStats = 1 unless defined $getAllStats;
    
    my $propsStat = undef;
    # handle special cases
    if ($property =~ /^dmg\-(max|min)$/) { $propsStat = $1."damage" }
    elsif ($property eq "dmg%") { $propsStat = "item_maxdamage_percent" }
    if (defined $propsStat)
    {
        for (0..scalar @$itemProperties) { return $_ if $itemProperties->[$_]->{stat} eq $propsStat };
        return undef;
    }
    
    my @ids;
    for my $statKey (sort keys %propertiesStatsHash)
    {
        my $prop = $properties->{$property}->{$statKey};
        next unless defined $prop;
        for (0..scalar @$itemProperties)
        {
            if ($itemProperties->[$_]->{stat} eq $prop)
            {
                if ($getAllStats) { push @ids, $_ }
                else { return $_ }
                last;
            }
        }
    }
    return join(',', @ids);
}
sub statIdFromPropertyStat { &statIdsFromPropertyStat($_[0], 0) }

my $uniques = parsetxt("uniqueitems.txt", _autoindex=>"0", iName=>"0", rlvl=>7);
&tblExpandArray($uniques, "iName");

my $sets = parsetxt("setitems.txt",_autoindex=>"0", iIName=>"0", iSName=>"1", '!_lodSet' => {col => 6, val => qr/^old LoD$/});
&tblExpandArray($sets, "iIName", "IName");
&tblExpandArray($sets, "iSName", "SName");

my $mxlSets, my $setIndex = 0, my $oldIndex = -3;
for my $setElement (@$sets)
{
    $oldIndex++;
    next unless (defined $setElement->{IName} and defined $setElement->{SName});
    $mxlSets->[$setIndex]->{index} = $oldIndex;
    $mxlSets->[$setIndex]->{IName} = $setElement->{IName};
    $mxlSets->[$setIndex]->{SName} = $setElement->{SName};
    $setIndex++;
}

my $itemName = 'name';
my $armorTypes = parsetxt("armor.txt", $itemName=>"0", "#code"=>"18", $nameStr=>19, w=>"29", h=>"30", type=>49, rlvl=>15, image => 35);
my $weaponTypes = parsetxt("weapons.txt", $itemName=>"0", "#code"=>"3", $nameStr=>5, w=>"41", h=>"42", type=>1, stackable=>43, rlvl=>28, image => 48);
my $miscTypes = parsetxt("misc.txt", $itemName=>"0", "#code"=>"18", $nameStr=>20, $spellDescStr=>68, w=>"22", h=>"23", type=>37, stackable=>48, rlvl=>8, image => 28);
&tblExpandHash($_, $itemName) for ($armorTypes, $weaponTypes, $miscTypes);

my $skills = parsetxt("skills.txt", _index=>"1", "dbgname"=>"0", "internalName"=>"3", "class"=>"2", "srvmissile"=>"15", "srvmissilea"=>"18", "srvmissileb"=>"19", "srvmissilec"=>"20", "SrcDam"=>"220");
my $skillsDsc = parsetxt("skilldesc.txt", "#code"=>"0", tab => 1, row => 2, col => 3, "dscname"=>"8");

my $processedSkills;
my $index = -1;
my %classes = (ama => 0, sor => 1, nec => 2, pal => 3, bar => 4, dru => 5, ass => 6);
foreach my $elem (@$skills)
{
    $index++;
    
    $processedSkills->[$index]->{iname1} = $elem->{dbgname};
    $processedSkills->[$index]->{class} = defined $elem->{class} ? $classes{$elem->{class}} : -1;
    next unless defined $elem->{internalName};
    my $desc = $skillsDsc->{$elem->{internalName}};
    $processedSkills->[$index]->{iname} = $desc->{dscname} if defined $desc->{dscname};
    $processedSkills->[$index]->{tab} = $desc->{tab};
    $processedSkills->[$index]->{row} = $desc->{row};
    $processedSkills->[$index]->{col} = $desc->{col};
}
&tblExpandArray($processedSkills, "iname", "name");

my $monstats = parsetxt("monstats.txt", _autoindex=>"0", $nameStr=>5);
&tblExpandArray($monstats, $nameStr);

# RW
my %rwKeysHash = ($itemName => undef, rune1 => 16, rune2 => 17); # this dummy key ($itemName) is used when writing to file
$rwKeysHash{"itype$_"} = $_ + 5 for (1..6);
my $rw = parsetxt("runes.txt", "#_tbl" => 0, '!_enabled' => {col => 2, val => $zeroRe}, '!_rune' => {col => 16, val => qr/^jew$/}, %rwKeysHash);
$rw->{'09This'} = {itype1 => 'weap', itype2 => 'armo', rune1 => 'jew'}; # yeah, it's a hack (jewelword)
&tblExpandHash($rw, $itemName);

# skip lines that don't start with 'xsignet' at col 33 (AH in Excel) and are not for honorific/crafted recipe
# 'value' field will contain either property value (including oskill level) or chance in ctc
my $moStat = "xsignet";
my $cubemain = parsetxt("cubemain.txt", '#_code' => 11, '!_enabled' => {col => 1, val => $zeroRe}, '!_mo' => {col => 33, val => qr/^(?!$moStat)/}, '!_honorific' => {col => 0, val => qr/^honorific/}, prop => 21, value => 25, moStat => 33);

# MO names and properties
my $mos; # hashref with keys from ID column of itemstatcost
for my $miscItem (keys %$miscTypes)
{
    my $hashRef = $miscTypes->{$miscItem};
    # no UMO support because there's no simple way to determine if it has been cubed with item (it doesn't have 'x_signet*' stat)
    next unless defined $hashRef->{type} and $hashRef->{type} eq 'asaa';
    
    my $cubeMo = $cubemain->{$miscItem};
    for (0..scalar @$itemProperties)
    {
        next unless defined $itemProperties->[$_]->{stat} and defined $cubeMo->{moStat};
        if ($itemProperties->[$_]->{stat} =~ /x_signet(\d+)/ and $cubeMo->{moStat} eq $moStat.$1)
        {
            $mos->{$_}->{statId} = &statIdFromPropertyStat($cubeMo->{prop});
            $mos->{$_}->{value} = $cubeMo->{value};
            $mos->{$_}->{code} = $miscItem;
            last;
        }
    }
}

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
        $firstColumn++; # skip 'max'
    }
}
my $gems = parsetxt("gems.txt", $itemName => 0, 'letter' => 1, "#code" => 3, %gemStatsHash);
&tblExpandHash($gems, $itemName);
if ($locale ne 'en')
{
    # translate rune letters
    if (open my $runeLetters, "<", "tbl/$locale/runes.txt")
    {
        my %letters;
        while(<$runeLetters>) { $letters{$1} = $2 if /^(\w+)\t(.+)$/ }
        for (keys %$gems)
        {
            my $letter = $gems->{$_}->{letter};
            $gems->{$_}->{letter} = $letters{$letter} if defined $letter and defined $letters{$letter};
        }
    }
    else { print "inserted runes will stay in English - failed to open tbl/$locale/runes.txt: $!\n" }
}

# generate
my $prefix = "generated/$locale";
make_path $prefix;

my $itemTypes = parsetxt("itemtypes.txt", "#code"=>"0", "code0"=>4, "equiv1"=>"5", equiv2 => 6, bodyLoc => 9, "class"=>30);
open my $out, ">", "generated/itemtypes.txt";
print $out "#code\tequiv\n";
for my $name (keys %$itemTypes)
{
    my $hashRef = $itemTypes->{$name};
    next unless defined $hashRef->{equiv1} and defined $hashRef->{code0};
    print $out $hashRef->{code0}, "\t", $hashRef->{equiv1};
    print $out ",", $hashRef->{equiv2} if defined $hashRef->{equiv2};
    print $out "\n";
}
close $out;

open $out, ">", "$prefix/items.txt";
print $out "#code\tname\t$spellDescStr\twidth\theight\tgentype\tstackable\trlvl\timage\ttype\tsockettype\tclass\n";
my $itemType = 0;
for my $ref ($armorTypes, $weaponTypes, $miscTypes)
{
    for my $itemCode (keys %$ref)
    {
        my $hashRef = $ref->{$itemCode};
        next unless defined $hashRef->{w};
        
        my $type = $hashRef->{type};
        printf $out "%s\t%s\t%s\t%d\t%d\t%s\t%d\t%d\t%s\t%s\t", $itemCode, $hashRef->{$itemName}, ($hashRef->{$spellDescStr} // ''), $hashRef->{w}, $hashRef->{h}, $itemType, ($hashRef->{stackable} // 0), ($hashRef->{rlvl} // 0), $hashRef->{image}, $type;
        
        for my $itemName (keys %$itemTypes) # find which class it belongs to
        {
            $hashRef = $itemTypes->{$itemName};
            if ($hashRef->{code0} eq $type)
            {
                # to determine item type when parsing socketables (see @subtypes): 0 - shield, 1 - weapon, nothing - armor
                print $out $itemType if defined $hashRef->{bodyLoc} and $hashRef->{bodyLoc} eq 'rarm';
                print $out "\t".(defined $hashRef->{class} ? $classes{$hashRef->{class}} : -1)."\n";
                last;
            }
        }
    }
    $itemType++;
}
close $out;

open $out, ">", "$prefix/uniques.txt";
print $out "#index\titem\trlvl\n";
my $count = -1;
for my $hashRef (@$uniques)
{
    print $out "$count\t$hashRef->{$realNameField}\t".($hashRef->{rlvl} // 0)."\n" if defined $hashRef->{$realNameField};
    $count++;
}
close $out;

open $out, ">", "$prefix/sets.txt";
print $out "#index\titem\tset\n";
print $out "$_->{index}\t$_->{IName}\t$_->{SName}\n" for (@$mxlSets);
close $out;

$count = -1;
my @skillKeys = qw/name class tab row col/;
open $out, ">", "$prefix/skills.txt";
print $out "#code\t".join("\t", @skillKeys)."\n";
for my $hashRef (@$processedSkills)
{
    $count++;
    print $out $count;
    print $out "\t".($hashRef->{$_} // ($_ eq 'name' ? '' : 0)) for (@skillKeys);
    print $out "\n";
}
close $out;

# don't need the keys any more, but need tbl strings
for (@$descArrayRef)
{
    delete $propertiesHash{$_->{key}};
    $propertiesHash{$_->{expanded}} = undef;
    $propertiesHash{descGroupIDs} = undef;
}

my @propKeys = sort keys %propertiesHash;
open $out, ">", "$prefix/props.txt";
print $out "#code\t".join("\t", @propKeys)."\n";
$count = -1;
for my $hashRef (@$itemProperties)
{
    $count++;
    next unless defined $hashRef->{bits} or defined $hashRef->{bitsSave};
    # my $s;
    # $s .= "\t".($hashRef->{$_} // '') for (@propKeys);
    # print $out "${count}$s\n";
    print $out $count;
    print $out "\t".($hashRef->{$_} // '') for (@propKeys);
    print $out "\n";
}
close $out;

open $out, ">", "$prefix/monsters.txt";
print $out "#index\tname\n";
$count = -2;
for (@$monstats)
{
    print $out "$count\t$_->{$realNameField}\n" if defined $_->{$realNameField};
    $count++;
}
close $out;

my @rwKeys = sort keys %rwKeysHash;
open $out, ">", "$prefix/rw.txt";
print $out "#".join("\t", @rwKeys)."\n";
for my $key (keys %$rw)
{
    print $out ($rw->{$key}->{$_} // '')."\t" for (@rwKeys);
    print $out "\n";
}
close $out;

my @moIds = sort keys %$mos;
my @moKeys = sort keys %{$mos->{$moIds[0]}};
open $out, ">", "generated/mo.txt";
print $out "#id\t".join("\t", @moKeys)."\n";
for my $id (@moIds)
{
    print $out $id;
    print $out "\t".($mos->{$id}->{$_} // '') for (@moKeys);
    print $out "\n";
}
close $out;

my @gemKeys = sort keys %gemStatsHash;
open $out, ">", "$prefix/socketables.txt";
print $out "#code\t$itemName\tletter\t".join("\t", @gemKeys)."\n";
for my $gemCode (keys %$gems)
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
                next;
            }
            
            $value = &statIdsFromPropertyStat($value);
        }
        print $out "\t".($value // '');
    }
    print $out "\n";
}
close $out;

my @baseStatsKeys = qw/strength dexterity energy vitality stamina lifePerLevel staminaPerLevel manaPerLevel lifePerPoint staminaPerPoint manaPerPoint/;
my %baseStatsParams = (_autoindex => 0, stamina => 6, '!_class' => {col => 0, val => 'Expansion'});
$baseStatsParams{$baseStatsKeys[$_]} = $_ + 1  for (0..3);
$baseStatsParams{$baseStatsKeys[$_]} = $_ + 13 for (5..10);
my $baseStats = parsetxt("charstats.txt", %baseStatsParams);

open $out, ">", "generated/basestats.txt";
print $out "#classcode\t".join("\t", @baseStatsKeys)."\n";
my $classCode = 0;
for my $i (0..scalar @$baseStats)
{
    next unless defined $baseStats->[$i];
    print $out $classCode;
    print $out "\t", $baseStats->[$i]->{$_} for (@baseStatsKeys);
    print $out "\n";
    $classCode++;
}
close $out;

# open my $out, ">", "d2items.pm";
# print $out saveStructure($miscTypes, "itemMisc", 80);
# print $out saveStructure($weaponTypes, "itemWeapons", 80);
# print $out saveStructure($armorTypes, "itemArmors", 80);
# print $out saveStructure($itemProperties, "itemProps", 80);
# print $out saveStructure($uniques, "uniques", 80);
# print $out saveStructure($sets, "sets", 80);
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
                    $ok = 0 if (!defined $s) or (length $s == 0) or ($s =~ /$hashRef->{val}/);
                    _log ("BAD - !$key");
                }
                elsif ($key =~ /^\!/)
                {
                    $ok = 0 if ((!defined($cols[$structure{$key}])) or (length($cols[$structure{$key}]) == 0));
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
                        _log("-> $fieldName<$structure{$key}>: '$cols[$structure{$key}]'\n");
                        if (defined ($cols[$structure{$key}]) and (length($cols[$structure{$key}])>0))
                        {
                            if (defined $indexValue)
                            {
                                $parsed->[$indexValue]->{$fieldName} = $cols[$structure{$key}];
                                _log("\tat $indexValue:1: $fieldName = $cols[$structure{$key}]\n");
                            }
                            else
                            {
                                if (defined $subHash)
                                {
                                    $parsed->{$subHash}->{$fieldName} = $cols[$structure{$key}] if ($addOnce == 0 or ($addOnce > 0 and !defined $parsed->{$subHash}->{$fieldName}));
                                    _log("\tat $subHash:2: $fieldName = $cols[$structure{$key}]\n");
                                }
                                else
                                {
                                    $parsed->{$fieldName} = $cols[$structure{$key}];
                                    _log("\tat $fieldName:3: $cols[$structure{$key}]\n");
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
