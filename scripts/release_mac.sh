#!/usr/bin/env bash

# example:
# QTDIR=~/dev/Qt-libs/4.8.7/distrib scripts/release_mac.sh ../build-MXLOT-release

if [[ -z "$QTDIR" ]]; then
  echo 'QTDIR not set. Call me from the repo root.'
  exit 1
fi

buildDir="$1"
if [[ -z "$buildDir" ]]; then
  echo 'No build dir specified. Call me from the repo root.'
  exit 1
fi

touch medianxlofflinetools.cpp # make sure it's rebuilt to generate current time

repoDir=$(pwd)
bundleName=MedianXLOfflineTools.app

mkdir -p "$buildDir"
cd "$buildDir"

"$QTDIR"/bin/qmake "$repoDir/MedianXLOfflineTools.pro" -nocache CONFIG+=release
make -j$(getconf _NPROCESSORS_ONLN) --silent
"$QTDIR"/bin/macdeployqt "$bundleName"

appVersion=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$bundleName"/Contents/Info.plist)
enclosingDirName="MedianXLOfflineTools_mac_$appVersion"
mkdir "$enclosingDirName"
mv "$bundleName" "$enclosingDirName"
cp "$repoDir/scripts/disable_auto_load.command" "$enclosingDirName"

archiveName="${enclosingDirName}.txz"
tar -cf "$archiveName" --xz "$enclosingDirName"

echo -e "\n$(pwd)/$archiveName"
