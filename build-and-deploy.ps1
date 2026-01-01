# Build and Deploy OBS Holyrics Finder Plugin
# This script builds the plugin and copies it to OBS installation directory

param(
    [string]$ObsInstallPath = "C:\Program Files\obs-studio"
)

$ErrorActionPreference = "Stop"

Write-Host "=== OBS Holyrics Finder - Build and Deploy ===" -ForegroundColor Cyan
Write-Host ""

# Get the workspace root
$WorkspaceRoot = $PSScriptRoot
if ($PSScriptRoot -eq "") {
    $WorkspaceRoot = Get-Location
}

Set-Location $WorkspaceRoot

# Step 1: Delete build_x64_ninja if it exists
$BuildDir = Join-Path $WorkspaceRoot "build_x64_ninja"
if (Test-Path $BuildDir) {
    Write-Host "[1/4] Cleaning existing build directory..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
    Write-Host "  ✓ Removed $BuildDir" -ForegroundColor Green
} else {
    Write-Host "[1/4] Build directory doesn't exist, skipping clean..." -ForegroundColor Yellow
}

# Step 2: Configure with CMake
Write-Host ""
Write-Host "[2/4] Configuring project with CMake..." -ForegroundColor Yellow
try {
    & cmake --preset windows-x64
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE"
    }
    Write-Host "  ✓ Configuration completed" -ForegroundColor Green
} catch {
    Write-Host "  ✗ Configuration failed: $_" -ForegroundColor Red
    exit 1
}

# Step 3: Build the project
Write-Host ""
Write-Host "[3/4] Building project (Release)..." -ForegroundColor Yellow
try {
    & cmake --build build_x64_ninja --config Release
    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed with exit code $LASTEXITCODE"
    }
    Write-Host "  ✓ Build completed" -ForegroundColor Green
} catch {
    Write-Host "  ✗ Build failed: $_" -ForegroundColor Red
    exit 1
}

# Step 4: Deploy to OBS
Write-Host ""
Write-Host "[4/4] Deploying to OBS installation..." -ForegroundColor Yellow

$SourceDir = Join-Path $BuildDir "Release"
$ObsPluginDir = Join-Path $ObsInstallPath "obs-plugins\64bit"

# Check if source directory exists
if (-not (Test-Path $SourceDir)) {
    Write-Host "  ✗ Build output directory not found: $SourceDir" -ForegroundColor Red
    exit 1
}

# Find DLL and PDB files
$DllFiles = Get-ChildItem -Path $SourceDir -Filter "*.dll" -ErrorAction SilentlyContinue
$PdbFiles = Get-ChildItem -Path $SourceDir -Filter "*.pdb" -ErrorAction SilentlyContinue

if ($DllFiles.Count -eq 0) {
    Write-Host "  ✗ No DLL files found in $SourceDir" -ForegroundColor Red
    exit 1
}

if ($DllFiles.Count -gt 1) {
    Write-Host "  ✗ Multiple DLL files found in $SourceDir. Expected only one plugin DLL." -ForegroundColor Red
    exit 1
}

if ($PdbFiles.Count -eq 0) {
    Write-Host "  ✗ No PDB files found in $SourceDir" -ForegroundColor Red
    exit 1
}

if ($PdbFiles.Count -gt 1) {
    Write-Host "  ✗ Multiple PDB files found in $SourceDir. Expected only one plugin PDB." -ForegroundColor Red
    exit 1
}

# Check if we need admin rights
$NeedAdmin = $false
try {
    $testFile = Join-Path $ObsPluginDir "test_write_permission.tmp"
    New-Item -Path $testFile -ItemType File -Force -ErrorAction Stop | Out-Null
    Remove-Item -Path $testFile -Force -ErrorAction SilentlyContinue
} catch {
    $NeedAdmin = $true
}

# Function to copy files with admin rights
function Copy-WithAdmin {
    param(
        [string]$Source,
        [string]$Destination
    )
    
    if ($NeedAdmin) {
        # Use Start-Process with -Verb RunAs to get admin rights
        $copyScript = @"
`$ErrorActionPreference = 'Stop'
try {
    Copy-Item -Path '$Source' -Destination '$Destination' -Force
    Write-Host '  ✓ Copied $(Split-Path -Leaf $Source)' -ForegroundColor Green
} catch {
    Write-Host '  ✗ Failed to copy $(Split-Path -Leaf $Source): `$_' -ForegroundColor Red
    exit 1
}
"@
        $encodedCommand = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($copyScript))
        Start-Process powershell.exe -ArgumentList "-NoProfile", "-EncodedCommand", $encodedCommand -Verb RunAs -Wait
    } else {
        Copy-Item -Path $Source -Destination $Destination -Force
        Write-Host "  ✓ Copied $(Split-Path -Leaf $Source)" -ForegroundColor Green
    }
}

# Copy DLL files
foreach ($dll in $DllFiles) {
    Write-Host "  Copying $($dll.Name)..."
    try {
        Copy-WithAdmin -Source $dll.FullName -Destination (Join-Path $ObsPluginDir $dll.Name)
    } catch {
        Write-Host "  ✗ Failed to copy $($dll.Name): $_" -ForegroundColor Red
        exit 1
    }
}

# Copy PDB files
foreach ($pdb in $PdbFiles) {
    Write-Host "  Copying $($pdb.Name)..."
    try {
        Copy-WithAdmin -Source $pdb.FullName -Destination (Join-Path $ObsPluginDir $pdb.Name)
    } catch {
        Write-Host "  ✗ Failed to copy $($pdb.Name): $_" -ForegroundColor Red
        exit 1
    }
}

Write-Host ""
Write-Host "=== Build and Deploy Completed Successfully ===" -ForegroundColor Green
Write-Host ""
Write-Host "Plugin files copied to: $ObsPluginDir" -ForegroundColor Cyan
Write-Host "You can now start OBS Studio to test the plugin." -ForegroundColor Cyan
Write-Host ""
