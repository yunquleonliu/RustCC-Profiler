# Building Rust C/C++ on Linux / 在 Linux 上构建 Rust C/C++

## Scope / 适用范围

This guide is the recommended path for clean verification builds.
本指南是进行干净验证构建的推荐路径。

---

## 1. Prerequisites / 前置依赖

Required tools / 必需工具:

- `cmake` >= 3.20
- C++17 compiler (`clang++` or `g++`)
- LLVM + Clang development packages with CMake config files
- `ninja` (recommended, optional)

Typical Ubuntu/Debian packages / 常见 Ubuntu/Debian 包:

```bash
sudo apt update
sudo apt install -y \
  cmake ninja-build build-essential \
  clang llvm-dev libclang-dev
```

Typical Fedora packages / 常见 Fedora 包:

```bash
sudo dnf install -y \
  cmake ninja-build gcc-c++ \
  clang llvm-devel clang-devel
```

---

## 2. Clean Build Directory / 干净构建目录

Use a Linux-only build directory to avoid cross-platform cache confusion.
使用 Linux 专用构建目录，避免跨平台缓存混淆。

```bash
cd /path/to/Tough\ C\ Profiler
rm -rf build build-linux
mkdir -p build-linux
```

---

## 3. Configure / 配置

### Option A: quick script (recommended) / 快速脚本（推荐）

```bash
chmod +x quick-build.sh
./quick-build.sh
```

### Option B: manual CMake / 手动 CMake

If `llvm-config` exists, use it to locate LLVM CMake directory:
如果存在 `llvm-config`，用它定位 LLVM CMake 目录：

```bash
LLVM_CMAKE_DIR="$(llvm-config --cmakedir)"
CLANG_CMAKE_DIR="$(dirname "$LLVM_CMAKE_DIR")/clang"

cmake -S . -B build-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_DIR="$LLVM_CMAKE_DIR" \
  -DClang_DIR="$CLANG_CMAKE_DIR" \
  -DRCC_BUILD_TESTS=ON \
  -DRCC_BUILD_EXAMPLES=ON
```

If `Clang_DIR` does not exist, omit it and set it explicitly later according to distro layout.
如果 `Clang_DIR` 不存在，可先省略并按发行版目录后续显式指定。

---

## 4. Build and Test / 构建与测试

```bash
cmake --build build-linux --config Release -j
ctest --test-dir build-linux --output-on-failure
```

---

## 5. Validate Failure Examples / 验证失败示例

After `rcc-check` is built:
构建出 `rcc-check` 后：

```bash
./build-linux/src/rcc-check tests/data/fail/move_use_after_move.cpp
./build-linux/src/rcc-check tests/data/fail/safety_violations.cpp
```

---

## 6. Troubleshooting / 故障排查

### `LLVM_DIR-NOTFOUND`

```bash
llvm-config --cmakedir
```

Set `-DLLVM_DIR=` to that output.
将 `-DLLVM_DIR=` 设置为该输出路径。

### `Could not find ClangConfig.cmake`

Try one of these typical paths:
尝试以下常见路径之一：

- `/usr/lib/llvm-17/lib/cmake/clang`
- `/usr/lib64/cmake/clang`

Then pass:

```bash
-DClang_DIR=/your/clang/cmake/path
```

### Build cache contamination from Windows / Windows 缓存污染

Delete and reconfigure from scratch:
删除并重新配置：

```bash
rm -rf build build-linux
cmake -S . -B build-linux -G Ninja ...
```

---

## 7. Recommended Workflow / 推荐工作流

1. Build in Linux with fresh directory (`build-linux`).
2. Run all tests.
3. Run selected fail examples and capture diagnostics.
4. Commit results or fixes.

1. 在 Linux 使用全新目录（`build-linux`）构建。
2. 跑全量测试。
3. 跑失败示例并采集诊断输出。
4. 提交结果或修复。

