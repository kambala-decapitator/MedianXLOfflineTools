#!/usr/bin/env bash

scriptDir=$(cd "$(dirname "$0")" && pwd)
zipName="$1.zip"
extraExclude=$2

mkdir resources
rsync "$scriptDir/../resources/data" -qr --exclude=data/items --exclude=data/sorting $extraExclude resources
zip -qr "$zipName" resources
rm -rf resources
