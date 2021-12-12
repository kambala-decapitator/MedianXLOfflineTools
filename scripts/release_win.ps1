msbuild /property:Configuration=Release /maxCpuCount MedianXLOfflineTools.sln

$appVersionMatch = Select-String -Pattern 'NVER_STRING="(\d+(?:\.\d+)+)"' -CaseSensitive -List -Path MedianXLOfflineTools.vcxproj
$appVersion = $appVersionMatch.Matches.Groups[1].Value

cd ..\Release
del -Force *.7z
7z a -r -mx -myx -mmt=on "MedianXLOfflineTools_${appVersion}.7z" *
