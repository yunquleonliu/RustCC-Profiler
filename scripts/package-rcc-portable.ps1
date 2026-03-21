param(
    [string]$OutputDir = "",
    [string]$RccExe = "",
    [string]$LlvmBinDir = "",
    [switch]$NoZip
)

$ErrorActionPreference = 'Stop'

function Resolve-RccExe {
    param([string]$RepoRoot, [string]$Explicit)

    if (-not [string]::IsNullOrWhiteSpace($Explicit)) {
        $resolved = Resolve-Path $Explicit -ErrorAction Stop
        return $resolved.Path
    }

    $candidates = @(
        (Join-Path $RepoRoot 'build\src\Release\rcc-check.exe'),
        (Join-Path $RepoRoot 'build\src\rcc-check.exe')
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Cannot find rcc-check.exe. Build first or pass -RccExe."
}

function Resolve-LlvmBinDir {
    param([string]$Explicit)

    if (-not [string]::IsNullOrWhiteSpace($Explicit)) {
        $resolved = Resolve-Path $Explicit -ErrorAction Stop
        return $resolved.Path
    }

    $candidates = @()
    if ($env:LLVM_HOME) {
        $candidates += (Join-Path $env:LLVM_HOME 'bin')
    }

    $candidates += @(
        'C:\Program Files\LLVM\bin',
        'C:\Program Files (x86)\LLVM\bin'
    )

    if ($env:USERPROFILE -and (Test-Path "$env:USERPROFILE\llvm-dev")) {
        $archives = Get-ChildItem "$env:USERPROFILE\llvm-dev" -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -like 'clang+llvm-*-windows-msvc' }
        foreach ($archive in $archives) {
            $candidates += (Join-Path $archive.FullName 'bin')
        }
    }

    foreach ($candidate in $candidates) {
        if ((Test-Path $candidate) -and (Test-Path (Join-Path $candidate 'libclang.dll'))) {
            return (Resolve-Path $candidate).Path
        }
    }

    throw "Cannot find LLVM bin directory with libclang.dll. Pass -LlvmBinDir."
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$rccExePath = Resolve-RccExe -RepoRoot $repoRoot -Explicit $RccExe
$llvmBinPath = Resolve-LlvmBinDir -Explicit $LlvmBinDir

if ([string]::IsNullOrWhiteSpace($OutputDir)) {
    $OutputDir = Join-Path $repoRoot 'dist'
}

if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$bundleRoot = Join-Path $OutputDir "rcc-portable-$stamp"
$binDir = Join-Path $bundleRoot 'bin'
$scriptsDir = Join-Path $bundleRoot 'scripts'
$docsDir = Join-Path $bundleRoot 'docs'

New-Item -ItemType Directory -Path $bundleRoot, $binDir, $scriptsDir, $docsDir | Out-Null

Copy-Item $rccExePath (Join-Path $binDir 'rcc-check.exe') -Force
Copy-Item (Join-Path $repoRoot 'scripts\rcc-check-dir.ps1') (Join-Path $scriptsDir 'rcc-check-dir.ps1') -Force
Copy-Item (Join-Path $repoRoot 'Docs\rcc_portable_quickstart.md') (Join-Path $docsDir 'rcc_portable_quickstart.md') -Force

# Copy LLVM runtime DLLs to maximize portability.
Get-ChildItem -Path $llvmBinPath -File -Filter '*.dll' | ForEach-Object {
    Copy-Item $_.FullName (Join-Path $binDir $_.Name) -Force
}

$launcher = @'
param(
    [Parameter(Mandatory = $true)]
    [string]$TargetPath,
    [switch]$TccOnly
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$env:PATH = (Join-Path $root 'bin') + ';' + $env:PATH
$scanner = Join-Path $root 'scripts\rcc-check-dir.ps1'

if ($TccOnly) {
    & $scanner -TargetPath $TargetPath -TccOnly
} else {
    & $scanner -TargetPath $TargetPath
}
'@

Set-Content -Path (Join-Path $bundleRoot 'run-rcc-dir.ps1') -Value $launcher -Encoding UTF8

$readme = @"
# RCC Portable Bundle

This bundle runs RCC on Windows machines without local RCC build setup.

## Quick start

1. Open PowerShell in this folder.
2. Run:

   .\run-rcc-dir.ps1 -TargetPath "C:\path\to\project"

Optional TCC-only scan:

   .\run-rcc-dir.ps1 -TargetPath "C:\path\to\project" -TccOnly

See docs\rcc_portable_quickstart.md for details.
"@

Set-Content -Path (Join-Path $bundleRoot 'README.md') -Value $readme -Encoding UTF8

if (-not $NoZip) {
    $zipPath = "$bundleRoot.zip"
    if (Test-Path $zipPath) {
        Remove-Item $zipPath -Force
    }
    Compress-Archive -Path (Join-Path $bundleRoot '*') -DestinationPath $zipPath -CompressionLevel Optimal
    Write-Host "Portable bundle created: $zipPath"
} else {
    Write-Host "Portable bundle created: $bundleRoot"
}
