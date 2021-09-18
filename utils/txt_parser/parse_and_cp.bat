@echo off
call create_translation ru
call create_translation en

cd generated
..\compressfiles itemtypes.csv exptable.csv basestats.csv sets.csv
move /Y *.dat ..\data\

cd ..
xcopy data\* ..\..\Resources\data\ /Y /S
rmdir data /S /Q
pause
