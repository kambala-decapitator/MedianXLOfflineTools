#!/bin/sh
cd `dirname $0`
./create_translation.sh ru
echo
./create_translation.sh en

cd generated
../CompressFiles itemtypes.tsv exptable.tsv basestats.tsv sets.tsv
mv *.dat ../data

cd ..
cp -fR data/* ../../Resources/data
rm -r data
