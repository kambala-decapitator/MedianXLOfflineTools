#!/usr/bin/env zsh

# call from the directory containing dc6 files

scriptDir=$(cd "$(dirname "$0")" && pwd)
imagesPath=../resources/data/images/items
find . -iname '*.dc6' -exec ~/dev/diablo/build-qdc6-qt4/qdc6 {} +
mv *.png "$scriptDir"/$imagesPath

cd "$scriptDir"
git add -A $imagesPath/*
git commit -m "update item images to Sigma $1"
