#!/bin/sh
cd `dirname $0`
./create_translation.sh ru
./create_translation.sh en

cd generated
../CompressFiles itemtypes.txt mo.txt exptable.txt basestats.txt sets.txt
mv *.dat ../data

cd ..
cp -fR data/* ../../Resources/data
rm -r data
