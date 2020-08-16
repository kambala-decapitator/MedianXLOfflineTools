#!/usr/bin/env bash

if [[ -z $1 ]]; then
  echo 'No version specified. Call me from the repo root.'
  exit 1
fi

qtProject=app.pri
msvsProject=MedianXLOfflineTools.vcxproj
infoPlist=resources/mac/Info.plist

scriptDir=$(cd "$(dirname "$0")" && pwd)
"$scriptDir"/set_new_app_version.pl "$qtProject" "$msvsProject" $1 "$infoPlist"
git commit -m "bump version to $1" "$qtProject" "$msvsProject" "$infoPlist"
