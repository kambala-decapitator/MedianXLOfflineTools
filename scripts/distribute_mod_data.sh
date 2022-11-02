#!/usr/bin/env bash

scriptDir=$(cd "$(dirname "$0")" && pwd)
zipName="$1.zip"
extraRsyncOptions="$2"

mkdir resources
rsync "$scriptDir/../resources/data" -qr --exclude=data/items --exclude=data/sorting $extraRsyncOptions resources
zip -qr "$zipName" resources
rm -rf resources
