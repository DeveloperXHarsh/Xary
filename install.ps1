# ============================================================================
# Xary Engine - Windows Automated Installer Script
# ============================================================================

$ErrorActionPreference = "Stop"

$InstallDir = "$env:LOCALAPPDATA\Programs\xary"
$ZipPath = "$env:TEMP\xary-latest.zip"
$ReleaseUrl = "https://github.com/DeveloperXHarsh/Xary/releases/latest/download/xary-v1.0.0-windows-x64.zip"

Write-Host "🚀 Installing Xary Engine..." -ForegroundColor Cyan

# Create Installation Directory
if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
}

# Download Archive
Write-Host "📥 Downloading latest binary..." -ForegroundColor Yellow
Invoke-WebRequest -Uri $ReleaseUrl -OutFile $ZipPath

# Extract Archive
Write-Host "📦 Extracting files..." -ForegroundColor Yellow
Expand-Archive -Path $ZipPath -DestinationPath $InstallDir -Force
Remove-Item -Path $ZipPath -Force

# Update User PATH Variable
$UserPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($UserPath -notlike "*$InstallDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$UserPath;$InstallDir", "User")
    Write-Host "✔ Added $InstallDir to User PATH." -ForegroundColor Green
}

Write-Host "`n🎉 Xary Engine successfully installed!" -ForegroundColor Green
Write-Host "💡 Restart your terminal and run 'xary --help' to verify installation." -ForegroundColor Cyan