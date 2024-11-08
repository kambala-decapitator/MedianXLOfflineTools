#!/usr/bin/env bash

extractDir=~/Documents/MXL/skills
mxlotDir=~/dev/diablo/medianxlofflinetools

mkdir -p $extractDir
for mpq in "/Users/kambala/Library/Application Support/CrossOver/Bottles/D2/drive_c/Program Files/Diablo II"/medianxl-*.mpq ; do
  ~/dev/diablo/MpqSimpleIO-binary -r "$mpq" \
  'data\global\themes\classic_sigma\game\skills\icons-*.dc6' \
  $extractDir 2>/dev/null
done
rm $extractDir/icons-shared.dc6
# exit 0

~/dev/diablo/build-qdc6-qt4/qdc6 -f jpg -d $extractDir
$mxlotDir/scripts/skill_icons.pl $extractDir

skillsDir=$mxlotDir/resources/data/images/skills
rm -rf $skillsDir/{0,1,2,3,4,5,6}
mv $extractDir/{0,1,2,3,4,5,6} $skillsDir

rm -rf $extractDir
