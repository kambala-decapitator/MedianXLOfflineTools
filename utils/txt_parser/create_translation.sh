#!/bin/sh
cd `dirname $0`
perl txtparser.pl $1
[ -e data ] || mkdir data
cp -fR generated/$1 data/$1
./CompressFiles data/$1
rm -f data/$1/*.txt
