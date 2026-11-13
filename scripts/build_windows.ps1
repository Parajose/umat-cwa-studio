$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Push-Location $root
try {
    $compiler = Get-Command g++ -ErrorAction Stop
    New-Item -ItemType Directory -Force "bin/Release" | Out-Null
    & $compiler.Source -std=c++11 -O2 -Wall -Wextra -Iinclude src/main.cpp -o bin/Release/UMaT_CWA_Studio.exe -mwindows -static -lcomctl32 -lshell32 -lole32 -lcomdlg32 -lgdi32 -luser32
    if ($LASTEXITCODE -ne 0) { throw "C++ build failed (exit $LASTEXITCODE)." }
    Write-Host "Built bin/Release/UMaT_CWA_Studio.exe"
} finally { Pop-Location }
