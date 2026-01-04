# Build Release Version for GitHub
# This script creates a release build and packages it for distribution

param(
    [string]$Version = "1.1.0"
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
if (Test-Path "README.pt-BR.md") {
    Copy-Item -Path "README.pt-BR.md" -Destination $PackageDir
    Write-Host "  ? Copied README.md" -ForegroundColor Green
}

if (Test-Path "LICENSE") {
    Copy-Item -Path "LICENSE" -Destination $PackageDir
    Write-Host "  ? Copied LICENSE" -ForegroundColor Green
}

# Create installation instructions
$InstallInstructions = @"
# OBS Holyrics Finder - Instruções de Instalação

1. Extraia esse arquivo ZIP para uma pasta temporária
2. Copie a pasta 'obs-plugins' para o diretório de instalação do OBS
   - Local padrão: C:\Program Files\obs-studio\
3. Reinicie o OBS Studio
4. O plugin vai aparecer em Ferramentas > Localizador Holyrics

## Uso

1. Abra OBS Studio
2. Vá para Ferramentas > Localizador Holyrics
3. Clique em "Testar Conexão" ou "Escanear Rede" para encontrar sua instância do Holyrics
4. Selecione as fontes do navegador para atualizar e clique em "Atualizar Fontes Selecionadas"

## Idiomas Suportados

- English
- Português (Brasil)

O plugin detecta automaticamente o idioma do OBS Studio.

## Suporte

Reporte erros em: https://github.com/Pelezi/obs-holyrics-plugin-finder/issues
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
