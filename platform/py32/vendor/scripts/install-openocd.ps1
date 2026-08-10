[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"

$OpenOcdUrl = "https://gitee.com/puya-semiconductor/tools-and-software/raw/" +
    "d503385bb4f9f5a51d7d5a5913d566fde9b66652/" +
    "PY32_GCC/openocd-0.12.0.zip"
$OpenOcdSha256 = "F2CB432E5C6AC65FA3B26F6A8441F3F54E53242335FBE5DA3B075CDF06152058"

$CommonRoot = Split-Path -Parent $PSScriptRoot
$ToolsRoot = Join-Path $CommonRoot "tools"
$OpenOcdRoot = Join-Path $ToolsRoot "openocd-windows"
$OpenOcd = Join-Path $OpenOcdRoot "bin\openocd.exe"

if (Test-Path -LiteralPath $OpenOcd) {
    Write-Host "OpenOCD is ready: $OpenOcd"
    exit 0
}

$InstallRoot = Join-Path $ToolsRoot ".openocd-windows-install"
$Archive = Join-Path $InstallRoot "openocd-0.12.0.zip"
$Expanded = Join-Path $InstallRoot "openocd-0.12.0"

if (Test-Path -LiteralPath $InstallRoot) {
    Remove-Item -Recurse -Force -LiteralPath $InstallRoot
}
New-Item -ItemType Directory -Force -Path $InstallRoot | Out-Null

Write-Host "OpenOCD was not found. Downloading Puya's official OpenOCD 0.12.0 package..."
try {
    Invoke-WebRequest -Uri $OpenOcdUrl -OutFile $Archive -UseBasicParsing
    $ActualHash = (Get-FileHash -Algorithm SHA256 $Archive).Hash
    if ($ActualHash -ne $OpenOcdSha256) {
        throw "OpenOCD SHA-256 mismatch: expected $OpenOcdSha256, got $ActualHash"
    }

    Expand-Archive -LiteralPath $Archive -DestinationPath $InstallRoot
    if (-not (Test-Path -LiteralPath (Join-Path $Expanded "bin\openocd.exe"))) {
        throw "Downloaded OpenOCD package has an unexpected directory layout."
    }

    if (Test-Path -LiteralPath $OpenOcdRoot) {
        Remove-Item -Recurse -Force -LiteralPath $OpenOcdRoot
    }
    Move-Item -LiteralPath $Expanded -Destination $OpenOcdRoot
} finally {
    if (Test-Path -LiteralPath $InstallRoot) {
        Remove-Item -Recurse -Force -LiteralPath $InstallRoot
    }
}

Write-Host "OpenOCD installed: $OpenOcd"
