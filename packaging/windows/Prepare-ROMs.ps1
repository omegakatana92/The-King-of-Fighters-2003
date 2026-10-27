# Prepare-ROMs.ps1 - build a runtime-ready KOF 2003 ROM set from your own dumps.
#
# This script extracts the MAME "kof2003" and "neogeo" sets and runs the
# bundled decrypt tool.  It downloads nothing and contains no ROM data.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File .\Prepare-ROMs.ps1
#   powershell -ExecutionPolicy Bypass -File .\Prepare-ROMs.ps1 -KofZip C:\path\kof2003.zip -NeoGeoZip C:\path\neogeo.zip

param(
    [string]$KofZip = "",
    [string]$NeoGeoZip = "",
    [string]$OutDir = ""
)

$ErrorActionPreference = "Stop"
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $OutDir) { $OutDir = Join-Path $here "roms" }
$src = Join-Path $here "roms-src"

function Ask-Zip($label, $current) {
    if ($current -and (Test-Path $current)) { return (Resolve-Path $current).Path }
    while ($true) {
        $p = Read-Host "Path to $label"
        $p = $p.Trim('"').Trim()
        if (Test-Path $p) { return (Resolve-Path $p).Path }
        Write-Host "Not found: $p" -ForegroundColor Yellow
    }
}

$KofZip = Ask-Zip "kof2003.zip" $KofZip
$NeoGeoZip = Ask-Zip "neogeo.zip" $NeoGeoZip

if (-not (Test-Path (Join-Path $here "kof2003_decrypt.exe"))) {
    throw "kof2003_decrypt.exe is missing from $here"
}

if (Test-Path $src) { Remove-Item -Recurse -Force $src }
New-Item -ItemType Directory -Force $src | Out-Null
New-Item -ItemType Directory -Force $OutDir | Out-Null

# Merged MAME sets put the parent files in a kof2003/ subdirectory.  Extract
# to a scratch folder, find the parent set, and flatten only the files the
# tool needs.  This also works for a non-merged zip with the files at the root.
$gameScratch = Join-Path $src "game"
New-Item -ItemType Directory -Force $gameScratch | Out-Null
Write-Host "Extracting $KofZip"
tar -xf $KofZip -C $gameScratch
if ($LASTEXITCODE -ne 0) { throw "Could not extract kof2003.zip with tar" }
$p1 = Get-ChildItem $gameScratch -Recurse -Filter "271-p1c.p1" | Select-Object -First 1
if (-not $p1) { throw "271-p1c.p1 was not found in $KofZip" }
$gameDir = $p1.DirectoryName
$needed = @("271-p1c.p1", "271-p2c.p2",
            "271-c1c.c1", "271-c2c.c2", "271-c3c.c3", "271-c4c.c4",
            "271-c5c.c5", "271-c6c.c6", "271-c7c.c7", "271-c8c.c8",
            "271-m1c.m1", "271-v1c.v1", "271-v2c.v2", "271-p3c.p3")
foreach ($name in $needed) {
    $f = Join-Path $gameDir $name
    if (-not (Test-Path $f)) { throw "$name is missing from the kof2003 set" }
    Copy-Item $f (Join-Path $src $name)
}
Remove-Item -Recurse -Force $gameScratch

Write-Host "Extracting $NeoGeoZip"
tar -xf $NeoGeoZip -C $src
if ($LASTEXITCODE -ne 0) { throw "Could not extract neogeo.zip with tar" }

Write-Host "Preparing KOF 2003 ROMs"
& (Join-Path $here "kof2003_decrypt.exe") --rom-path $src --out $OutDir
if ($LASTEXITCODE -ne 0) { throw "kof2003_decrypt failed" }

Write-Host ""
Write-Host "Done. Run:" -ForegroundColor Green
Write-Host "  .\`"KOF 2003.exe`" --rom-path `"$OutDir`""

