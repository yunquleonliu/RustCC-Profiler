# RCC End-to-End Guide
# RCC 端到端完整指南

This document gives one complete, practical path for understanding and using this
project from zero to production CI usage.
本文提供一条完整且可执行的路径，帮助你从零理解并使用本项目，直到在 CI 中落地。

---

## 1. What RCC Is

RCC (Rust C/C++) is a pre-compilation safety profiler for C/C++.
RCC（Rust C/C++）是一个面向 C/C++ 的预编译安全分析器。

It analyzes code with Clang AST and reports rule violations before compilation
is considered acceptable by your policy.
它基于 Clang AST 分析代码，并在策略层面于“可接受编译”之前报告规则违规。

Key point:

- RCC does not rewrite your code automatically.
- RCC gives diagnostics, fix hints, and an exit code contract.

---

## 2. Current Supported Path

Current validated support:
当前已验证支持：

- Linux only
- LLVM/Clang 14
- CMake + compile_commands workflow

Windows docs are kept as deferred notes for future re-enable.
Windows 文档当前保留为未来恢复支持时的参考。

---

## 3. Repository Mental Model

Main executable:

- `rcc-check`

Core flow:

1. CLI parses args
2. ClangTool loads translation units
3. RuleEngine runs categories/rules
4. DiagnosticEngine prints findings
5. Process exits with policy result code

High-level source map:

- `src/main.cpp`: CLI entrypoint
- `src/RuleEngine.cpp`: rule orchestration
- `src/*Rules.cpp`: category/rule logic
- `src/Diagnostic.cpp`: output formatting and severity handling
- `tests/`: pass/fail behavior contract

---

## 4. Build Once (Linux)

Use the verified Linux path:

```bash
mkdir -p build-linux
cmake -S . -B build-linux -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm \
  -DClang_DIR=/usr/lib/llvm-14/lib/cmake/clang \
  -DRCC_BUILD_TESTS=ON \
  -DRCC_BUILD_EXAMPLES=ON

cmake --build build-linux --config Release -j
ctest --test-dir build-linux --output-on-failure
```

Binary location:

```bash
./build-linux/src/rcc-check
```

---

## 5. Run RCC On Real Files

Typical command:

```bash
./build-linux/src/rcc-check -p build-linux path/to/file.cpp
```

When include resolution is incomplete, pass Clang args after `--`:

```bash
./build-linux/src/rcc-check -p build-linux path/to/file.cpp -- -std=c++17
```

Useful flags:

- `--verbose`
- `--rcc-version`
- `--no-ownership`
- `--no-lifetime`
- `--no-concurrency`

---

## 6. Read Results Correctly

RCC prints:

- source location
- severity
- message
- rule id (for example `TCC-OWN-001`)
- optional fix hints / escape options

Exit codes:

- `0`: no blocking errors
- `1`: rule violations found
- `2`: compile/frontend issue
- `3`: internal error
- `4`: bad CLI args
- `5`: input file missing

This exit-code contract is the foundation for CI/CD and Make integration.

---

## 7. Rule Families In Practice

You will commonly see:

- `TCC-OWN-*` ownership and move
- `TCC-LIFE-*` lifetime and reference safety
- `TCC-CONC-*` concurrency and shared-state safety
- `TCC-BORROW-*` borrow-checker-inspired rules
- `TCC-OPTION-*`, `TCC-RESULT-*`, `TCC-PANIC-*`, `TCC-SAFE-*` safety families

Important practical note:

- not all declared families are equally complete yet
- treat ownership/lifetime/concurrency and tested move checks as the most reliable base

---

## 8. File Conventions vs Runtime Behavior

Repository conventions include:

- `_t.cc` suffix
- `@tcc` annotation
- `@tcc-no-*` category annotations

Current CLI runtime behavior:

- RCC analyzes files you explicitly pass on command line
- file-detector conventions exist in code, but are not yet enforced as a hard CLI gate

Use conventions for team organization today, but do not depend on them as strict
runtime filtering until the wiring is completed.

---

## 9. Smooth Adoption Strategy For Real Projects

### Stage A: Local developer loop

1. build once on Linux
2. run RCC on touched files
3. fix blocking errors
4. run unit/integration tests

### Stage B: CI observer mode

1. run RCC in CI
2. publish diagnostics
3. do not fail PRs yet

### Stage C: CI changed-files gate

1. run RCC only on changed C/C++ files
2. fail pipeline on exit code `1`

### Stage D: Expand coverage

1. selected directories
2. broader modules
3. eventually most of the project

---

## 10. VS Code Workflow (No Extension Needed)

Recommended first step:

- add a workspace task that runs RCC on current file
- add a problem matcher for `file:line:column: error|warning|note`

Why this first:

- near-zero setup cost
- immediate diagnostics in Problems panel
- easy for teammates to adopt

---

## 11. Make Workflow (Simple And Effective)

Add a target such as:

```make
RCC := ./build-linux/src/rcc-check
RCC_DB := build-linux
RCC_FILES := src/foo.cpp src/bar.cpp

.PHONY: rcc-check
rcc-check:
	$(RCC) -p $(RCC_DB) $(RCC_FILES)
```

Then gate with:

```make
.PHONY: rcc-gate
rcc-gate: rcc-check
```

and wire `rcc-gate` into your preferred build/check path.

---

## 12. Daily Checklist

Before pushing:

1. `rcc-check` on touched files
2. fix error-level findings
3. run project tests
4. push

In CI:

1. configure/build Linux environment
2. run RCC gate
3. fail only on blocking RCC results

---

## 13. Known Limits (Be Explicit)

- Linux is the actively supported platform today
- some rule families are still partial
- file-detector conventions are not yet hard-gated in main CLI path

This transparency helps teams trust the tool and adopt it incrementally.

---

## 14. Reading Order For New Maintainers

1. `README.md`
2. `Docs/user_guide.md`
3. `Docs/keywords_and_profiles.md`
4. `Docs/ci_cd_quickstart.md`
5. `Docs/integration_plan.md`
6. this file (`Docs/end_to_end.md`)
