# Build Release Version for GitHub
# This script creates a release build and packages it for distribution

param(
    [string]$Version = "1.0.0"
)

$ErrorActionPreference = "Stop"

Write-Host "=== OBS Holyrics Finder - Release Build ===" -ForegroundColor Cyan
Write-Host "Version: $Version" -ForegroundColor Cyan
Write-Host ""

# Get the workspace root
$WorkspaceRoot = $PSScriptRoot
Set-Location $WorkspaceRoot

# Step 1: Clean build directory
$BuildDir = Join-Path $WorkspaceRoot "build_x64_ninja"
if (Test-Path $BuildDir) {
    Write-Host "[1/5] Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
    Write-Host "  ? Cleaned" -ForegroundColor Green
}

# Step 2: Configure with CMake
Write-Host ""
Write-Host "[2/5] Configuring with CMake..." -ForegroundColor Yellow
& cmake --preset windows-x64
if ($LASTEXITCODE -ne 0) {
    Write-Host "  ? Configuration failed" -ForegroundColor Red
    exit 1
}
Write-Host "  ? Configuration completed" -ForegroundColor Green

# Step 3: Build Release
Write-Host ""
Write-Host "[3/5] Building Release version..." -ForegroundColor Yellow
& cmake --build build_x64_ninja --config Release
if ($LASTEXITCODE -ne 0) {
    Write-Host "  ? Build failed" -ForegroundColor Red
    exit 1
}
Write-Host "  ? Build completed" -ForegroundColor Green

# Step 4: Create release package
Write-Host ""
Write-Host "[4/5] Creating release package..." -ForegroundColor Yellow

$ReleaseDir = Join-Path $WorkspaceRoot "release"
$PackageDir = Join-Path $ReleaseDir "obs-holyrics-finder-$Version"
$PluginDir = Join-Path $PackageDir "obs-plugins\64bit"

# Create directories
if (Test-Path $ReleaseDir) {
    Remove-Item -Path $ReleaseDir -Recurse -Force
}
New-Item -Path $PluginDir -ItemType Directory -Force | Out-Null

# Copy DLL and PDB
$SourceDir = Join-Path $BuildDir "Release"
$DllFile = Get-ChildItem -Path $SourceDir -Filter "*.dll" | Select-Object -First 1
$PdbFile = Get-ChildItem -Path $SourceDir -Filter "*.pdb" | Select-Object -First 1

if ($DllFile) {
    Copy-Item -Path $DllFile.FullName -Destination $PluginDir
    Write-Host "  ? Copied $($DllFile.Name)" -ForegroundColor Green
} else {
    Write-Host "  ? DLL file not found!" -ForegroundColor Red
    exit 1
}

if ($PdbFile) {
    Copy-Item -Path $PdbFile.FullName -Destination $PluginDir
    Write-Host "  ? Copied $($PdbFile.Name)" -ForegroundColor Green
}

# Copy README and LICENSE
if (Test-Path "README.md") {
    Copy-Item -Path "README.md" -Destination $PackageDir
    Write-Host "  ? Copied README.md" -ForegroundColor Green
}

if (Test-Path "LICENSE") {
    Copy-Item -Path "LICENSE" -Destination $PackageDir
    Write-Host "  ? Copied LICENSE" -ForegroundColor Green
}

# Create installation instructions
$InstallInstructions = @"
# OBS Holyrics Finder - Installation Instructions

## Automatic Installation (Recommended)

1. Extract this ZIP file
2. Copy the 'obs-plugins' folder to your OBS installation directory
   - Default location: C:\Program Files\obs-studio\
3. Restart OBS Studio
4. The plugin will appear in Tools > Holyrics Finder (or "Localizador Holyrics" in Portuguese)

## Manual Installation

1. Navigate to: obs-plugins\64bit\
2. Copy the .dll file to: C:\Program Files\obs-studio\obs-plugins\64bit\
3. (Optional) Copy the .pdb file for debugging support
4. Restart OBS Studio

## Usage

1. Open OBS Studio
2. Go to Tools > Holyrics Finder
3. Click "Test Connection" or "Scan Network" to find your Holyrics instance
4. Select browser sources to update and click "Update Selected Sources"

## Supported Languages

- English
- Português (Brasil)

The plugin automatically detects OBS's language setting.

## Troubleshooting

If the plugin doesn't appear:
- Make sure you copied files to the correct directory
- Check OBS logs (Help > Log Files) for errors
- Verify OBS version compatibility (requires OBS 28.0 or later)

## Support

Report issues at: https://github.com/Pelezi/obs-holyrics-plugin-finder/issues
"@

Set-Content -Path (Join-Path $PackageDir "INSTALL.txt") -Value $InstallInstructions -Encoding UTF8
Write-Host "  ? Created INSTALL.txt" -ForegroundColor Green

# Step 5: Create ZIP archive
Write-Host ""
Write-Host "[5/5] Creating ZIP archive..." -ForegroundColor Yellow

$ZipPath = Join-Path $ReleaseDir "obs-holyrics-finder-$Version-windows-x64.zip"
Compress-Archive -Path $PackageDir -DestinationPath $ZipPath -Force

Write-Host "  ? Created ZIP archive" -ForegroundColor Green

# Display results
Write-Host ""
Write-Host "=== Release Build Complete ===" -ForegroundColor Green
Write-Host ""
Write-Host "Release package: $ZipPath" -ForegroundColor Cyan
Write-Host "Package size: $([math]::Round((Get-Item $ZipPath).Length / 1KB, 2)) KB" -ForegroundColor Cyan
Write-Host ""
Write-Host "Package contents:" -ForegroundColor Yellow
Get-ChildItem -Path $PackageDir -Recurse | Where-Object { -not $_.PSIsContainer } | ForEach-Object {
    $relativePath = $_.FullName.Substring($PackageDir.Length + 1)
    Write-Host "  - $relativePath" -ForegroundColor Gray
}
Write-Host ""
Write-Host "Ready to upload to GitHub Releases!" -ForegroundColor Green
Write-Host ""
