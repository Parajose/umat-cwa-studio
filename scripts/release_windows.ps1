# Local packaging only: no GitHub, git push, tag or publish operations.
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    & "$PSScriptRoot/build_windows.ps1"
    New-Item -ItemType Directory -Force build, dist | Out-Null
    $compiler = Get-Command g++ -ErrorAction Stop
    & $compiler.Source -dumpmachine | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "Cannot determine compiler target." }
    $target = (& $compiler.Source -dumpmachine).Trim()
    if ($target -notmatch 'mingw') { throw "Use a MinGW Windows compiler. Target was $target" }
    $architecture = if ($target -match '^x86_64') { 'x64' } elseif ($target -match '^i[3-6]86') { 'x86' } else { throw "Unsupported release architecture: $target" }
    & $compiler.Source -std=c++11 -Wall -Wextra -pedantic -static tests/core_tests.cpp -o build/core_tests.exe
    if ($LASTEXITCODE -ne 0) { throw "Test compilation failed." }
    Push-Location build
    try {
        & .\core_tests.exe
        if ($LASTEXITCODE -ne 0) { throw "Core tests failed." }
    } finally { Pop-Location }
    $version = (Get-Content VERSION -Raw).Trim()
    if ($version -notmatch '^\d+\.\d+\.\d+$') { throw "Invalid VERSION." }
    $name = "UMaT-CWA-Studio-$version-windows-$architecture"
    $stage = Join-Path $root "build/portable/$name"
    if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
    New-Item -ItemType Directory -Force $stage | Out-Null
    Copy-Item bin/Release/UMaT_CWA_Studio.exe $stage
    Copy-Item docs/START_HERE.txt $stage
    Copy-Item docs/USER_GUIDE.md $stage
    Copy-Item CHANGELOG.md $stage
    "Version: $version`r`nCompiler target: $target`r`nBuilt: $([DateTime]::UtcNow.ToString('o'))`r`nManual Windows verification: required before sharing" | Set-Content (Join-Path $stage BUILD_INFO.txt)
    $zip = Join-Path $root "dist/$name.zip"
    Compress-Archive -Path $stage -DestinationPath $zip -Force
    $hash = (Get-FileHash -Algorithm SHA256 $zip).Hash.ToLowerInvariant()
    "$hash  $name.zip" | Set-Content -Encoding ASCII "$zip.sha256"
    Write-Host "Prepared $zip"
    Write-Host "Complete docs/WINDOWS_CHECKLIST.md before sharing. Nothing was published."
} finally { Pop-Location }
