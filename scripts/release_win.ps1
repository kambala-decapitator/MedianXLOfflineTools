msbuild /property:Configuration=Release /p:Platform=Win32 /terminalLogger /maxCpuCount MedianXLOfflineTools.sln
if ($LASTEXITCODE -ne 0) { exit 1 }

$appVersionMatch = Select-String -Pattern 'NVER_STRING="(\d+(?:\.\d+)+)"' -CaseSensitive -List -Path MedianXLOfflineTools.vcxproj
$appVersion = $appVersionMatch.Matches.Groups[1].Value

cd ..\Release
del -Force *.7z
7z a -r -mx -myx -mmt=on "MedianXLOfflineTools_${appVersion}.7z" *
