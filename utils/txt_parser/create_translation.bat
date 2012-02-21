@echo off
perl txtparser.pl %1
xcopy generated\%1 data\%1\ /Y /S
compressfiles data\%1
del data\%1\*.txt
rem zip -r data data
rem rmdir data /S /Q