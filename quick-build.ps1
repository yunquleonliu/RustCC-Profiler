# Rust C/C++ Quick Start Script
# Rust C/C++ 快速启动脚本
# Windows PowerShell version / Windows PowerShell 版本

Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║  Rust C/C++ Profiler - Quick Build / 快速构建                 ║" -ForegroundColor Cyan
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Initialize MSVC environment / 初始化 MSVC 环境
Write-Host "Initializing MSVC environment... / 初始化 MSVC 环境..." -ForegroundColor Yellow
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vsWhere) {
    $vsPath = & $vsWhere -latest -property installationPath
    if ($vsPath) {
        $vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $vcvarsPath) {
            Write-Host "✓ Found Visual Studio at: $vsPath" -ForegroundColor Green
            # Import MSVC environment into PowerShell
            $tempFile = [System.IO.Path]::GetTempFileName()
            cmd /c "`"$vcvarsPath`" > nul && set" | Out-File -FilePath $tempFile
            Get-Content $tempFile | ForEach-Object {
                if ($_ -match "^([^=]+)=(.*)$") {
                    [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], "Process")
                }
            }
            Remove-Item $tempFile
            Write-Host "✓ MSVC environment loaded / MSVC 环境已加载" -ForegroundColor Green
        }
    }
}

# Check prerequisites / 检查先决条件
Write-Host "Checking prerequisites... / 检查先决条件..." -ForegroundColor Yellow

# Check CMake
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) {
    Write-Host "✗ CMake not found. Please install CMake 3.20+." -ForegroundColor Red
    Write-Host "✗ 未找到 CMake。请安装 CMake 3.20+。" -ForegroundColor Red
    exit 1
}
Write-Host "✓ CMake found: $($cmake.Version)" -ForegroundColor Green

# Check for MSVC
$cl = Get-Command cl -ErrorAction SilentlyContinue
if ($cl) {
    Write-Host "✓ MSVC compiler found: $($cl.Source)" -ForegroundColor Green
} else {
    Write-Host "⚠ MSVC compiler (cl.exe) not found in PATH" -ForegroundColor Yellow
}

# Check for LLVM/Clang - REQUIRED for Rust C/C++
Write-Host ""
Write-Host "Checking LLVM/Clang installation... / 检查 LLVM/Clang 安装..." -ForegroundColor Yellow
$llvmPaths = @(
    "C:\Program Files\LLVM",
    "C:\Program Files (x86)\LLVM",
    "$env:LLVM_HOME"
)

$llvmFound = $false
$llvmDir = $null
foreach ($path in $llvmPaths) {
    if ($path -and (Test-Path $path)) {
        $llvmDir = $path
        $llvmFound = $true
        Write-Host "✓ LLVM found at: $llvmDir" -ForegroundColor Green
        break
    }
}

if (-not $llvmFound) {
    Write-Host "✗ LLVM/Clang NOT FOUND! / 未找到 LLVM/Clang！" -ForegroundColor Red
    Write-Host ""
    Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Yellow
    Write-Host "  Rust C/C++ requires LLVM/Clang libraries to function." -ForegroundColor White
    Write-Host "  Rust C/C++ 需要 LLVM/Clang 库才能运行。" -ForegroundColor White
    Write-Host ""
    Write-Host "  Download from: https://github.com/llvm/llvm-project/releases" -ForegroundColor Cyan
    Write-Host "  Recommended: LLVM 17.0.x (includes Clang)" -ForegroundColor Cyan
    Write-Host "  推荐：LLVM 17.0.x（包含 Clang）" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "  Installation steps / 安装步骤:" -ForegroundColor White
    Write-Host "  1. Download LLVM-17.x.x-win64.exe" -ForegroundColor Gray
    Write-Host "  2. Run installer, select 'Add to PATH'" -ForegroundColor Gray
    Write-Host "  3. Restart PowerShell" -ForegroundColor Gray
    Write-Host "  4. Run this script again / 重新运行此脚本" -ForegroundColor Gray
    Write-Host "═══════════════════════════════════════════════════════════" -ForegroundColor Yellow
    Write-Host ""
    exit 1
}

Write-Host ""

# Create build directory / 创建构建目录
Write-Host "Creating build directory... / 创建构建目录..." -ForegroundColor Yellow
if (Test-Path "build") {
    Write-Host "Build directory exists, cleaning... / 构建目录已存在，清理中..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force "build"
}
New-Item -ItemType Directory -Path "build" | Out-Null
Write-Host "✓ Build directory created / 构建目录已创建" -ForegroundColor Green
Write-Host ""

# Configure / 配置
Write-Host "Configuring with CMake... / 使用 CMake 配置..." -ForegroundColor Yellow
Set-Location "build"

# Set LLVM_DIR if we found LLVM / 如果找到 LLVM 则设置 LLVM_DIR
$cmakeArgs = @(
    "..",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DRCC_BUILD_TESTS=ON",
    "-DRCC_BUILD_EXAMPLES=ON"
)

if ($llvmDir) {
    $cmakeArgs += "-DLLVM_DIR=$llvmDir\lib\cmake\llvm"
    $cmakeArgs += "-DClang_DIR=$llvmDir\lib\cmake\clang"
}

# Use Visual Studio generator if available
if ($cl) {
    Write-Host "Using Visual Studio generator with MSVC / 使用 Visual Studio 生成器和 MSVC" -ForegroundColor Cyan
    cmake @cmakeArgs -G "Visual Studio 17 2022" -A x64
} else {
    cmake @cmakeArgs
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ CMake configuration failed / CMake 配置失败" -ForegroundColor Red
    Write-Host ""
    Write-Host "Troubleshooting / 故障排查:" -ForegroundColor Yellow
    Write-Host "1. Ensure LLVM is installed: https://github.com/llvm/llvm-project/releases" -ForegroundColor Gray
    Write-Host "2. Set LLVM_DIR environment variable to LLVM installation path" -ForegroundColor Gray
    Write-Host "3. Restart PowerShell after LLVM installation" -ForegroundColor Gray
    Set-Location ..
    exit 1
}
Write-Host "✓ Configuration complete / 配置完成" -ForegroundColor Green
Write-Host ""

# Build / 构建
Write-Host "Building... / 构建中..." -ForegroundColor Yellow
cmake --build . --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "✗ Build failed / 构建失败" -ForegroundColor Red
    Set-Location ..
    exit 1
}
Write-Host "✓ Build complete / 构建完成" -ForegroundColor Green
Write-Host ""

# Test / 测试
Write-Host "Running tests... / 运行测试..." -ForegroundColor Yellow
ctest -C Release --output-on-failure

if ($LASTEXITCODE -ne 0) {
    Write-Host "⚠ Some tests failed / 部分测试失败" -ForegroundColor Yellow
}
else {
    Write-Host "✓ All tests passed / 所有测试通过" -ForegroundColor Green
}
Write-Host ""

Set-Location ..

# Summary / 总结
Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║  Build Complete! / 构建完成！                               ║" -ForegroundColor Green
Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "Executable location / 可执行文件位置:" -ForegroundColor Cyan
Write-Host "  build\src\Release\rcc-check.exe" -ForegroundColor White
Write-Host ""
Write-Host "To install / 安装:" -ForegroundColor Cyan
Write-Host "  cmake --install build --prefix <install_path>" -ForegroundColor White
Write-Host ""
Write-Host "To test examples / 测试示例:" -ForegroundColor Cyan
Write-Host "  .\build\src\Release\rcc-check.exe examples\01_smart_pointers_t.cc" -ForegroundColor White
Write-Host ""
Write-Host "For more information / 更多信息:" -ForegroundColor Cyan
Write-Host "  See BUILD.md and PROJECT_STRUCTURE.md" -ForegroundColor White
Write-Host ""

