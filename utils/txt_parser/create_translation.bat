@echo off
perl txtparser.pl %1
xcopy generated\%1 data\%1\ /Y /S
compressfiles data\%1
del data\%1\*.txt

copy /B tbl\%1\expansionstring.txt data\%1\expansionstring.dat
copy /B tbl\%1\string.txt 		   data\%1\string.dat
copy /B tbl\%1\patchstring.txt	   data\%1\patchstring.dat