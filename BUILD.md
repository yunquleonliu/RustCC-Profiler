# Rust C/C++ Profiler - Building and Usage Guide
# Rust C/C++ 分析器 - 构建和使用指南

## Quick Start / 快速开始

Recommended for clean verification: Linux first.
推荐用于干净验证：优先在 Linux 上构建。

### Prerequisites / 前置要求

- **CMake** 3.20 or higher / 3.20 或更高版本
- **C++17** compatible compiler / 兼容 C++17 的编译器
- **LLVM/Clang** 15, 16, or 17 / 版本 15、16 或 17

### Building / 构建

```bash
# Create clean Linux build directory / 创建干净的 Linux 构建目录
mkdir -p build-linux && cd build-linux

# Configure / 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build / 构建
cmake --build . --config Release

# Run tests (optional) / 运行测试（可选）
ctest --output-on-failure
```

For Linux-specific package setup and troubleshooting, see:
Linux 依赖安装和故障排查见：

- [BUILDING_ON_LINUX.md](BUILDING_ON_LINUX.md)

For Windows-specific setup, see:
Windows 专用配置见：

- [BUILDING_ON_WINDOWS.md](BUILDING_ON_WINDOWS.md)

### Installation / 安装

```bash
# Install to system / 安装到系统
sudo cmake --install .

# Or specify install prefix / 或指定安装前缀
cmake --install . --prefix /usr/local
```

---

## Usage / 使用方法

### Basic Usage / 基本使用

```bash
# Check a single file / 检查单个文件
rcc-check myfile_t.cc

# Check multiple files / 检查多个文件
rcc-check file1_t.cc file2_t.cc

# With compilation database / 使用编译数据库
rcc-check -p build/ src/main_t.cc
```

### File Markers / 文件标记

RCC analyzes files that have either:
RCC 分析具有以下任一标记的文件：

1. **`_t.cc` extension** / **`_t.cc` 扩展名**
   ```cpp
   // myfile_t.cc
   #include <memory>
   // ... your code
   ```

2. **`@tcc` annotation** / **`@tcc` 注解**
   ```cpp
   // myfile.cpp
   // @tcc
   #include <memory>
   // ... your code
   ```

### Command Options / 命令选项

```bash
# Show version / 显示版本
rcc-check --version

# Verbose output / 详细输出
rcc-check --verbose myfile_t.cc

# Disable specific checks / 禁用特定检查
rcc-check --no-ownership myfile_t.cc   # Disable ownership / 禁用所有权
rcc-check --no-lifetime myfile_t.cc    # Disable lifetime / 禁用生命周期
rcc-check --no-concurrency myfile_t.cc # Disable concurrency / 禁用并发
rcc-check --no-safety myfile_t.cc      # Disable safety patterns / 禁用安全模式

# Combine options / 组合选项
rcc-check --verbose --no-concurrency myfile_t.cc
```

---

## Understanding Diagnostics / 理解诊断信息

### Error Format / 错误格式

```
filename_t.cc:10:5: error / 错误: Use of 'new' operator forbidden [TCC-OWN-001]
  Fix suggestions / 修复建议:
    → Use std::make_unique<T>() instead
  Opt-out options / 退出选项:
    ⚠ Remove @tcc annotation to use raw C++
    ⚠ Move file out of RCC directory
```

### Error Categories / 错误类别

- **`TCC-OWN-xxx`**: Ownership violations / 所有权违规
- **`TCC-LIFE-xxx`**: Lifetime violations / 生命周期违规
- **`TCC-CONC-xxx`**: Concurrency violations / 并发违规
- **`TCC-BORROW-xxx`**: Borrow checker violations / 借用检查违规
- **`TCC-OPTION-xxx`**: Option safety violations / Option 安全违规
- **`TCC-RESULT-xxx`**: Result handling violations / Result 处理违规
- **`TCC-PANIC-xxx`**: Panic/unwrap violations / panic/unwrap 违规
- **`TCC-SAFE-xxx`**: Bounds and general safety violations / 边界和通用安全违规

---

## Integration / 集成

### CMake Integration / CMake 集成

```cmake
# Add RCC check as custom target / 添加 RCC 检查作为自定义目标
add_custom_target(rcc_check
    COMMAND rcc-check ${CMAKE_SOURCE_DIR}/src/*_t.cc
    COMMENT "Running Rust C/C++ checks / 运行 Rust C/C++ 检查"
)

# Run RCC before build / 在构建前运行 RCC
add_dependencies(my_target rcc_check)
```

### CI Integration / CI 集成

```yaml
# GitHub Actions example / GitHub Actions 示例
- name: Run RCC Checks
  run: |
    rcc-check src/**/*_t.cc
    
# Fail build on RCC errors / RCC 错误时失败构建
- name: RCC Check
  run: rcc-check src/**/*_t.cc || exit 1
```

---

## Examples / 示例

See the `examples/` directory for:
参见 `examples/` 目录以获取：

- ✓ `01_smart_pointers_t.cc` - Correct ownership / 正确的所有权
- ✗ `02_raw_pointer_violations_t.cc` - Ownership errors / 所有权错误
- ✓ `03_lifetime_safety_t.cc` - Safe lifetime / 安全的生命周期
- ✗ `04_lifetime_violations_t.cc` - Lifetime errors / 生命周期错误
- ✓ `05_thread_safety_t.cc` - Thread-safe patterns / 线程安全模式
- ✗ `06_thread_violations_t.cc` - Concurrency errors / 并发错误
- ✓ `07_move_semantics_t.cc` - Move-safe patterns / 移动安全模式
- ✗ `08_move_violations_t.cc` - Use-after-move and double-move / 移动后使用与双重移动
- ✓ `09_borrow_checker_t.cc` - Borrow-safe patterns / 借用安全模式
- ✗ `10_borrow_violations_t.cc` - Borrow conflicts / 借用冲突
- ✓ `11_option_result_patterns_t.cc` - Option/Result best practices / Option/Result 最佳实践
- ✗ `12_safety_violations_t.cc` - Safety violations / 安全违规

---

## Escape Hatches / 逃生通道

If RCC is too restrictive:
如果 RCC 限制过多：

### Option 1: Remove RCC marker / 选项1：移除 RCC 标记
```cpp
// myfile.cpp (no @tcc annotation)
// Now this is regular C++ / 现在这是普通 C++
int* ptr = new int(42);  // OK now / 现在可以了
```

### Option 2: Disable specific checks / 选项2：禁用特定检查
```cpp
// @tcc-no-ownership
// Still RCC, but ownership checks disabled
// 仍然是 RCC，但所有权检查已禁用
```

### Option 3: Mix RCC and non-RCC files / 选项3：混合 RCC 和非 RCC 文件
```
src/
  safe_module_t.cc     ← RCC enforced / RCC 强制
  legacy_module.cpp   ← Regular C++ / 普通 C++
```

---

## FAQ / 常见问题

**Q: Does RCC change my code?**
**Q: RCC 会改变我的代码吗？**

A: No. RCC only analyzes and reports. It never modifies your source files.
A: 不会。RCC 只分析和报告。它从不修改您的源文件。

**Q: Can I use RCC with existing projects?**
**Q: 我可以在现有项目中使用 RCC 吗？**

A: Yes. Add RCC markers incrementally to files you want to check.
A: 可以。逐步向您想检查的文件添加 RCC 标记。

**Q: What if I need unsafe operations?**
**Q: 如果我需要不安全的操作怎么办？**

A: Remove the RCC marker from that specific file. RCC is opt-in per file.
A: 从该特定文件移除 RCC 标记。RCC 是按文件选择加入的。

---

## Next Steps / 后续步骤

- Read [Docs/INDEX.md](Docs/INDEX.md) for the full documentation map
  阅读 [Docs/INDEX.md](Docs/INDEX.md) 获取完整文档索引

- Check [Docs/vision.md](Docs/vision.md) for philosophy
  查看 [Docs/vision.md](Docs/vision.md) 了解理念

- Track implementation reality in [Working Track .md](Working%20Track%20.md)
  在 [Working Track .md](Working%20Track%20.md) 跟踪真实实现进度

