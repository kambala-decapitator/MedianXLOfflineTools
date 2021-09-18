#!/usr/bin/env zsh

rm -f *.pcx

for f in $(find . -iname '*.dc6')
    ~/Diablo\ II\ save\ files/dc6con.zip\ Folder/dc6con "$f"

mogrify -format png -fuzz 2% -transparent black *.pcx
# mogrify -format png *.pcx

scriptDir=$(cd "$(dirname "$0")" && pwd)
mv *.png "$scriptDir/../resources/data/images/items"
