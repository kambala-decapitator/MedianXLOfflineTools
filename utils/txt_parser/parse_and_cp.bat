@echo off
call create_translation ru
call create_translation en

cd generated
..\compressfiles itemtypes.tsv exptable.tsv basestats.tsv sets.tsv
move /Y *.dat ..\data\

cd ..
xcopy data\* ..\..\Resources\data\ /Y /S
rmdir data /S /Q
pause
