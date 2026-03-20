# RCC User Guide
# RCC 用户指南

This guide is for people who want to run the RCC profiler and interpret results.
It does not focus on rebuilding the project itself.
本指南面向希望运行 RCC 分析器并理解结果的用户，不聚焦于项目自身构建过程。

---

## 1. What You Run

The executable is `rcc-check`.
可执行文件名为 `rcc-check`。

Current validated environment:
当前已验证环境：

- Linux only
- LLVM/Clang 14
- CMake-generated compilation database

Typical built binary location:
典型二进制位置：

```bash
./build-linux/src/rcc-check
```

---

## 2. Minimal Working Invocation

For files that belong to a CMake project with `compile_commands.json`:
对于属于带有 `compile_commands.json` 的 CMake 项目的文件：

```bash
./build-linux/src/rcc-check -p build-linux path/to/file.cpp
```

For standalone files that are not fully described by the compilation database,
pass Clang arguments after `--`.
对于编译数据库中没有完整描述的独立文件，需要在 `--` 后显式传入 Clang 参数。

```bash
./build-linux/src/rcc-check \
  -p build-linux \
  tests/data/pass/ownership_smart_pointers.cpp \
  -- \
  -std=c++17 \
  -I/usr/include/c++/11 \
  -I/usr/include/x86_64-linux-gnu/c++/11 \
  -I/usr/include/c++/11/backward \
  -I/usr/lib/gcc/x86_64-linux-gnu/11/include \
  -I/usr/local/include \
  -I/usr/include/x86_64-linux-gnu \
  -I/usr/include
```

This is the same pattern used by the automated test suite.
这与自动化测试所使用的调用方式一致。

---

## 3. Supported CLI Flags

The current binary supports these user-facing options:
当前二进制支持以下用户可见选项：

- `-p <dir>`: path to the build directory with `compile_commands.json`
- `--verbose`: print per-file / per-translation-unit progress
- `--rcc-version`: print version banner and exit
- `--no-ownership`: disable ownership-category rules
- `--no-lifetime`: disable lifetime-category rules
- `--no-concurrency`: disable concurrency-category rules

Important:
重要说明：

- There is currently no `--no-safety` CLI flag.
- 当前没有 `--no-safety` 这个 CLI 选项。

---

## 4. What The Exit Codes Mean

RCC uses process exit codes as the integration contract.
RCC 使用进程退出码作为集成契约。

- `0`: no blocking errors were found
- `1`: rule violations were found
- `2`: frontend / compilation problem
- `3`: internal tool error
- `4`: invalid command-line arguments
- `5`: input file not found

That means RCC fits naturally into CI, `ctest`, shell scripts, and Make rules.
这意味着 RCC 可以自然集成到 CI、`ctest`、shell 脚本和 Make 规则中。

---

## 5. Output Shape

On success:
成功时：

```text
✓ All checks passed! Code is RCC-compliant.
✓ 所有检查通过！代码符合 RCC 规范。
```

On failure:
失败时：

```text
✗ RCC rule violations found:
✗ 发现 RCC 规则违规:
```

Each diagnostic includes:
每条诊断通常包含：

- source location
- severity
- message
- rule id such as `TCC-OWN-001`
- fix hints when available
- escape-hatch suggestions when available

---

## 6. File Naming And Annotation Conventions

The repository defines two RCC file markers in `FileDetector`:
仓库在 `FileDetector` 中定义了两种 RCC 文件标记：

- `_t.cc` suffix
- `@tcc` annotation within the first 100 lines

Examples:
示例：

```cpp
// foo_t.cc
```

```cpp
// @tcc
```

There are also per-file annotation switches in `FileDetector`:
`FileDetector` 中还定义了按文件控制的注解：

- `@tcc-no-ownership`
- `@tcc-no-lifetime`
- `@tcc-no-concurrency`

Important current limitation:
当前的重要限制：

- These detection/config hooks exist in code, but the current CLI path does not
  enforce them before analysis.
- 这些检测/配置逻辑在代码中存在，但当前 CLI 执行路径并不会在分析前真正使用它们进行过滤。

Today, RCC analyzes the files you explicitly pass on the command line.
目前，RCC 会分析你在命令行中显式传入的文件。

So treat `_t.cc` and `@tcc` as project conventions today, not as a hard CLI gate.
因此，目前应将 `_t.cc` 和 `@tcc` 视为项目约定，而不是 CLI 的硬性分析入口门槛。

---

## 7. Rule Families You Will See

Rule ids use this format:
规则 ID 使用以下格式：

```text
TCC-<CATEGORY>-<NUMBER>
```

Common families:
常见规则家族：

- `TCC-OWN-*`: ownership and move rules
- `TCC-LIFE-*`: lifetime and reference safety
- `TCC-CONC-*`: concurrency and shared-state safety
- `TCC-BORROW-*`: borrow-checker-inspired rules
- `TCC-OPTION-*`: optional / null-handling guidance
- `TCC-RESULT-*`: result / error-handling guidance
- `TCC-PANIC-*`: panic / unwrap-style hazards
- `TCC-SAFE-*`: general safety checks such as bounds guidance

Not all declared rule families are equally complete yet.
并非所有已声明的规则家族都已经同等完善。

The currently reliable path is:
当前较为可靠的路径是：

- ownership core rules
- lifetime core rules
- concurrency core rules already exercised by tests
- use-after-move detection

Some borrow and safety-pattern rules are still partial or placeholder logic.
部分 borrow 和 safety-pattern 规则仍然是部分实现或占位逻辑。

---

## 8. Practical Usage Patterns

### Check one file in a project

```bash
./build-linux/src/rcc-check -p build-linux src/my_module.cpp
```

### Check one standalone file with explicit include paths

```bash
./build-linux/src/rcc-check -p build-linux examples/01_smart_pointers_t.cc -- -std=c++17
```

### Temporarily disable one rule family

```bash
./build-linux/src/rcc-check -p build-linux src/my_module.cpp --no-concurrency
```

### Verbose mode for debugging invocation issues

```bash
./build-linux/src/rcc-check -p build-linux src/my_module.cpp --verbose
```

---

## 9. How RCC Fits Into A Normal Project

RCC works best when:
RCC 最适合以下使用方式：

- your project already generates `compile_commands.json`
- you run RCC on selected source files in CI or local validation
- you treat exit code `1` as a blocking policy failure

If your file is outside the compilation database, you must supply enough Clang
arguments after `--` for parsing to succeed.
如果文件不在编译数据库中，就必须在 `--` 后补充足够的 Clang 参数，确保解析成功。

---

## 10. Current Reality Checklist

What is true today:
今天真实有效的情况：

- Linux-only build/test path is the currently supported one.
- GitHub Actions is aligned to the Linux validation path.
- `rcc-check` is the actual executable.
- `-p build-linux` is the normal entrypoint for project analysis.
- tests are green on Linux.

What is not yet true end-to-end:
当前尚未完全成立的部分：

- `_t.cc` / `@tcc` are not yet enforced by the CLI runtime path
- per-file `@tcc-no-*` config is not yet wired into rule-engine execution
- some declared rule families are still incomplete

---

## 11. Where To Go Next

- For Linux build setup: `BUILDING_ON_LINUX.md`
- For project philosophy: `README.md`
- For rule terminology: `Docs/keywords_and_profiles.md`
