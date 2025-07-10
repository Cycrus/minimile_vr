Write-Output "----- Adding CustomTreadmillDriver to SteamVR Registry -----"

$DRIVER_PATH = Join-Path $PSScriptRoot "CustomTreadmillDriver"
$STEAM_PATH = (Get-ItemProperty -Path "HKCU:\Software\Valve\Steam" -Name "SteamPath").SteamPath
$STEAM_VRPATHREG = Join-Path $STEAM_PATH "steamapps\common\SteamVR\bin\win64\vrpathreg.exe"

& $STEAM_VRPATHREG removedriverswithname CustomTreadmill
& $STEAM_VRPATHREG removedriver $DRIVER_PATH
& $STEAM_VRPATHREG adddriver $DRIVER_PATH


Write-Output "          Success!"