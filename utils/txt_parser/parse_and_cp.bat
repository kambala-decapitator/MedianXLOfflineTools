@echo off
call create_translation ru
call create_translation en

cd generated
..\compressfiles itemtypes.txt mo.txt exptable.txt basestats.txt sets.txt
move /Y *.dat ..\data\

cd ..
xcopy data\* ..\..\Resources\data\ /Y /S
rmdir data /S /Q
pause
