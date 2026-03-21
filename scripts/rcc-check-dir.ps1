param(
    [Parameter(Mandatory = $true)]
    [string]$TargetPath,

    [string]$BuildDir = "",

    [switch]$TccOnly
)

$ErrorActionPreference = 'Stop'

function Resolve-RccBinary {
    param([string]$RepoRoot)

    $candidates = @(
        (Join-Path $RepoRoot 'build\src\Release\rcc-check.exe'),
        (Join-Path $RepoRoot 'build\src\rcc-check.exe')
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "rcc-check.exe not found. Build first (quick-build.ps1) or pass a valid build output."
}

function Test-LikelyBinaryFile {
    param([string]$FilePath)

    $bytes = [System.IO.File]::ReadAllBytes($FilePath)
    if ($bytes.Length -eq 0) {
        return $false
    }

    if ($bytes.Length -ge 2 -and $bytes[0] -eq 0x4D -and $bytes[1] -eq 0x5A) {
        return $true
    }

    $sampleSize = [Math]::Min(512, $bytes.Length)
    $nullCount = 0
    for ($i = 0; $i -lt $sampleSize; $i++) {
        if ($bytes[$i] -eq 0) {
            $nullCount++
        }
    }

    return (($nullCount / $sampleSize) -gt 0.10)
}

$repoRoot = Split-Path -Parent $PSScriptRoot
$target = Resolve-Path $TargetPath

if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot 'build'
}

$rcc = Resolve-RccBinary -RepoRoot $repoRoot

$excludePattern = '\\(bin|build|out|dist|\.git|node_modules|\.cache)\\'
$extensions = @('*.c', '*.cc', '*.cpp', '*.cxx')

$files = Get-ChildItem -Path $target -Recurse -File -Include $extensions |
    Where-Object { $_.FullName -notmatch $excludePattern }

if ($TccOnly) {
    $files = $files | Where-Object {
        $_.Name -like '*_t.cc' -or (Get-Content -Path $_.FullName -TotalCount 100 -ErrorAction SilentlyContinue | Select-String -Pattern '@tcc' -SimpleMatch)
    }
}

if (-not $files -or $files.Count -eq 0) {
    Write-Host "No C/C++ source files found under $target after filters."
    exit 0
}

$failed = 0
$checked = 0
$skippedBinary = 0

foreach ($file in $files) {
    if (Test-LikelyBinaryFile -FilePath $file.FullName) {
        Write-Host "SKIP(binary): $($file.FullName)"
        $skippedBinary++
        continue
    }

    Write-Host "RCC checking: $($file.FullName)"
    & $rcc --auto-stdcpp-includes -p $BuildDir $file.FullName
    if ($LASTEXITCODE -ne 0) {
        $failed++
    }
    $checked++
}

Write-Host ""
Write-Host "Summary: checked=$checked failed=$failed skipped_binary=$skippedBinary"

if ($failed -ne 0) {
    exit 1
}

exit 0
