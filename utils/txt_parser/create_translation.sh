#!/bin/sh
cd `dirname $0`
echo "processing $1"
perl txtparser.pl $1
[ -e data ] || mkdir data
cp -fR generated/$1 data/$1
./CompressFiles data/$1
rm -f data/$1/*.txt

cp tbl/$1/expansionstring.txt data/$1/expansionstring.dat
cp tbl/$1/string.txt 		  data/$1/string.dat
cp tbl/$1/patchstring.txt	  data/$1/patchstring.dat
