# Building Rust C/C++ on Windows / 在 Windows 上构建 Rust C/C++

> Recommendation / 建议:
> For first clean verification, use Linux build first.
> 第一次干净验证建议优先走 Linux 构建。
> See: `BUILDING_ON_LINUX.md`

## 🔧 Prerequisites / 先决条件

### Required / 必需
1. **CMake 3.20+** ✅ (You have this! / 你已经有了！)
2. **LLVM/Clang Development Libraries** ❌ (Need to install / 需要安装)
3. **MSVC or MinGW** (For compiling / 用于编译)

### Why LLVM/Clang is Required / 为什么需要 LLVM/Clang

Rust C/C++ is a **static analyzer** that parses C++ code using Clang's AST (Abstract Syntax Tree) libraries. This is the same technology used by:
Rust C/C++ 是一个**静态分析器**，使用 Clang 的 AST（抽象语法树）库解析 C++ 代码。这与以下工具使用的技术相同：

- clang-tidy (C++ linter)
- clangd (Language server)
- Clang Static Analyzer

**You cannot build Rust C/C++ without LLVM/Clang libraries**, just like you cannot build clang-tidy without them.
**没有 LLVM/Clang 库就无法构建 Rust C/C++**，就像无法在没有它们的情况下构建 clang-tidy 一样。

---

## 📥 Installing LLVM/Clang / 安装 LLVM/Clang

### Option 1: Official Pre-built Binaries (Recommended / 推荐)

1. **Download LLVM 17.0.6 for Windows**
   ```
   https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/LLVM-17.0.6-win64.exe
   ```

2. **Run the installer**
   - ✅ Check "Add LLVM to system PATH"
   - ✅ Install location: `C:\Program Files\LLVM`

3. **Restart PowerShell**

4. **Verify installation**
   ```powershell
   clang --version
   # Should show: clang version 17.0.6
   ```

### Option 2: Using vcpkg (Alternative / 备选)

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install LLVM (this takes 1-2 hours!)
.\vcpkg install llvm:x64-windows

# Set environment variable
$env:LLVM_DIR = "C:\path\to\vcpkg\installed\x64-windows"
```

---

## 🏗️ Building Rust C/C++ / 构建 Rust C/C++

Once LLVM is installed / 安装 LLVM 后：

```powershell
# Clone or navigate to repository
cd "D:\SyncWork\Rust C/C++ Profiler"

# Run quick build script
.\quick-build.ps1
```

The script will:
脚本将：
- ✅ Detect MSVC environment
- ✅ Find LLVM installation
- ✅ Configure with CMake
- ✅ Build with MSVC
- ✅ Run tests

---

## 🎯 Quick Validation (Without Full Build) / 快速验证（无需完整构建）

If you want to validate the project structure without building:
如果您想在不构建的情况下验证项目结构：

```powershell
# Check all files are present
Get-ChildItem -Recurse -File | Measure-Object | Select-Object Count

# Verify test files
Get-ChildItem tests\data -Recurse -Filter *.cpp

# Check documentation
Get-ChildItem -Filter *.md
```

---

## 📊 Build Outputs / 构建输出

After successful build / 成功构建后：

```
build/
  src/
    Release/
      rcc-check.exe  ← Main executable / 主可执行文件
```

---

## 🧪 Testing / 测试

```powershell
# Run all tests
cd build
ctest -C Release --output-on-failure

# Test specific file
.\src\Release\rcc-check.exe ..\examples\01_smart_pointers_t.cc
```

---

## ❓ Troubleshooting / 故障排查

### "LLVM_DIR not found"
**Solution / 解决方案**:
```powershell
# Set manually
$env:LLVM_DIR = "C:\Program Files\LLVM\lib\cmake\llvm"
$env:Clang_DIR = "C:\Program Files\LLVM\lib\cmake\clang"
.\quick-build.ps1
```

### "Cannot find cl.exe"
**Solution / 解决方案**:
```powershell
# Initialize Visual Studio environment
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
# Or use the Developer Command Prompt
```

### Build takes too long
**Expected / 预期**: First build with LLVM can take 5-15 minutes.
Subsequent builds are much faster (1-2 minutes).

---

## 🚀 After Building / 构建完成后

You can use Rust C/C++:
您可以使用 Rust C/C++：

```powershell
# Check any _t.cc file
.\build\src\Release\rcc-check.exe myfile_t.cc

# Or annotated C++ file
.\build\src\Release\rcc-check.exe myfile.cpp  # with @tcc annotation
```

---

## 💡 Alternative: Use CI/CD / 备选方案：使用 CI/CD

If local building is challenging, you can:
如果本地构建困难，您可以：

1. **Push to GitHub**
2. **GitHub Actions will build automatically** (see `.github/workflows/ci.yml`)
3. **Download artifacts** from Actions tab

This is often easier for quick validation!
这通常更容易进行快速验证！

---

## 📞 Need Help? / 需要帮助？

The project structure is complete and correct. LLVM is only needed for **building**, not for understanding the code.
项目结构完整且正确。LLVM 仅用于**构建**，而不是理解代码。

All source code is readable and well-documented without building.
所有源代码无需构建即可阅读并理解。

Current local blocker reminder / 当前本机阻塞提醒：

- If `LLVM_DIR-NOTFOUND` appears in `build/CMakeCache.txt`, install LLVM/Clang first.
- 如果 `build/CMakeCache.txt` 出现 `LLVM_DIR-NOTFOUND`，请先安装 LLVM/Clang。

