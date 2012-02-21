@echo off
call create_translation en
call create_translation ru

cd generated
..\compressfiles itemtypes.txt mo.txt exptable.txt
move /Y *.dat ..\data\

cd ..
xcopy data\* ..\..\Resources\data\ /Y /S
rmdir data /S /Q
pause