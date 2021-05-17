if (!(Get-Module -ListAvailable -Name 'VSSetup')) {
    Write-Output 'Installing VSSetup'
    Install-Module VSSetup -Scope CurrentUser
}

$requiredVCLibs = @(
    "msvcrt.lib";
    "legacy_stdio_definitions.lib";
    "legacy_stdio_wide_specifiers.lib";
    "vcruntime.lib";
)

$requiredWinSDKLibs = @(
    "kernel32.Lib";
    "ShLwApi.Lib";
)

$VCBase = (Get-ChildItem ((Get-VSSetupInstance).InstallationPath + "\VC\Tools\MSVC\*\lib")).FullName
$winSDKReg = "HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots"
$winSDKVersion = Split-Path (Get-ChildItem $winSDKReg).Name -Leaf
$winSDKLib = (Get-ItemProperty $winSDKReg).KitsRoot10 + "Lib\$winSDKVersion"

foreach ($arch in @("x86"; "x64";)) {
    $dst = "libstdc\lib\$arch\"
    New-Item "libstdc\lib\$arch" -ItemType "directory" -Force | Out-Null
    foreach ($file in $requiredVCLibs) {
        Copy-Item ($VCBase + "\$arch\$file") -Destination $dst
    }

    Copy-Item ($winSDKLib + "\ucrt\$arch\ucrt.lib") -Destination $dst
    foreach ($file in $requiredWinSDKLibs) {
        Copy-Item ($winSDKLib + "\um\$arch\$file") -Destination $dst
    }
}
