#!/usr/bin/env bash

extractDir=~/Documents/MXL/items

mkdir -p $extractDir
for mpq in "/Users/kambala/Library/Application Support/CrossOver/Bottles/D2/drive_c/Program Files/Diablo II"/medianxl-*.mpq ; do
  ~/dev/diablo/MpqSimpleIO-binary -r "$mpq" \
  'data\global\items\*.dc6' \
  $extractDir 2>/dev/null
done
