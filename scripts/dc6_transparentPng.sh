#!/usr/bin/env fish

rm -f *.pcx

for f in * #.dc6
    ~/Documents/MXL/dc6con.zip\ Folder/dc6con "$f"
end

mogrify -format png -fuzz 2% -transparent black *.pcx
# mogrify -format png *.pcx

set scriptDir (dirname (status --current-filename))
mv *.png "$scriptDir/../resources/data/images/items"
