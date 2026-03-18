#!/bin/bash
# Tough C Quick Start Script
# Tough C 快速启动脚本
# Linux/macOS Bash version / Linux/macOS Bash 版本

set -e

echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Tough C Profiler - Quick Build / 快速构建                 ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Check prerequisites / 检查先决条件
echo "Checking prerequisites... / 检查先决条件..."

# Check CMake
if ! command -v cmake &> /dev/null; then
    echo "✗ CMake not found. Please install CMake 3.20+."
    echo "✗ 未找到 CMake。请安装 CMake 3.20+。"
    exit 1
fi
echo "✓ CMake found: $(cmake --version | head -n1)"

# Check for LLVM/Clang
if ! command -v clang &> /dev/null; then
    echo "⚠ Clang not in PATH. Make sure LLVM/Clang is installed."
    echo "⚠ Clang 不在 PATH 中。请确保已安装 LLVM/Clang。"
else
    echo "✓ Clang found: $(clang --version | head -n1)"
fi

echo ""

# Detect LLVM/Clang CMake dirs / 检测 LLVM/Clang 的 CMake 目录
LLVM_CMAKE_DIR=""
CLANG_CMAKE_DIR=""

if command -v llvm-config &> /dev/null; then
    LLVM_CMAKE_DIR="$(llvm-config --cmakedir 2>/dev/null || true)"
    if [ -n "$LLVM_CMAKE_DIR" ] && [ -d "$LLVM_CMAKE_DIR" ]; then
        echo "✓ LLVM CMake dir: $LLVM_CMAKE_DIR"
        CANDIDATE_CLANG_DIR="$(dirname "$LLVM_CMAKE_DIR")/clang"
        if [ -d "$CANDIDATE_CLANG_DIR" ]; then
            CLANG_CMAKE_DIR="$CANDIDATE_CLANG_DIR"
            echo "✓ Clang CMake dir: $CLANG_CMAKE_DIR"
        else
            echo "⚠ Clang CMake dir auto-detect failed. Will rely on CMake search."
            echo "⚠ Clang CMake 目录自动检测失败，将依赖 CMake 默认搜索。"
        fi
    fi
fi

# Create Linux-specific build directory / 创建 Linux 专用构建目录
echo "Creating build directory... / 创建构建目录..."
BUILD_DIR="build-linux"
if [ -d "$BUILD_DIR" ]; then
    echo "Build directory exists, cleaning... / 构建目录已存在，清理中..."
    rm -rf "$BUILD_DIR"
fi
mkdir "$BUILD_DIR"
echo "✓ Build directory created: $BUILD_DIR / 构建目录已创建"
echo ""

# Configure / 配置
echo "Configuring with CMake... / 使用 CMake 配置..."
cd "$BUILD_DIR"

CMAKE_ARGS=(
  ..
  -DCMAKE_BUILD_TYPE=Release
  -DTCC_BUILD_TESTS=ON
  -DTCC_BUILD_EXAMPLES=ON
)

if [ -n "$LLVM_CMAKE_DIR" ]; then
    CMAKE_ARGS+=("-DLLVM_DIR=$LLVM_CMAKE_DIR")
fi

if [ -n "$CLANG_CMAKE_DIR" ]; then
    CMAKE_ARGS+=("-DClang_DIR=$CLANG_CMAKE_DIR")
fi

cmake "${CMAKE_ARGS[@]}"

echo "✓ Configuration complete / 配置完成"
echo ""

# Build / 构建
echo "Building... / 构建中..."
cmake --build . --config Release

echo "✓ Build complete / 构建完成"
echo ""

# Test / 测试
echo "Running tests... / 运行测试..."
if ctest -C Release --output-on-failure; then
    echo "✓ All tests passed / 所有测试通过"
else
    echo "⚠ Some tests failed / 部分测试失败"
fi
echo ""

cd ..

# Summary / 总结
echo "╔════════════════════════════════════════════════════════════╗"
echo "║  Build Complete! / 构建完成！                               ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Executable location / 可执行文件位置:"
echo "  build-linux/src/tcc-check"
echo ""
echo "To install / 安装:"
echo "  sudo cmake --install build-linux --prefix /usr/local"
echo ""
echo "To test examples / 测试示例:"
echo "  ./build-linux/src/tcc-check examples/01_smart_pointers.tcc"
echo ""
echo "For more information / 更多信息:"
echo "  See BUILD.md and PROJECT_STRUCTURE.md"
echo ""
