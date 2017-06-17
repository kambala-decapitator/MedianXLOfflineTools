#!/usr/bin/env fish

rm *.pcx

for f in * #.dc6
    ~/Documents/MXL/dc6con.zip\ Folder/dc6con "$f"
end

mogrify -format png -fuzz 2% -transparent black *.pcx
# mogrify -format png *.pcx
mv *.png ~/Development/medianxlofflinetools/Resources/data/images/items
