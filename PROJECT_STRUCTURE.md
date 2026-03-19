# Rust C/C++ Profiler - Project Structure
# Rust C/C++ 分析器 - 项目结构

## Directory Layout / 目录布局

```text
Rust C/C++ Profiler/
|-- CMakeLists.txt
|-- README.md
|-- BUILD.md
|-- BUILDING_ON_LINUX.md
|-- BUILDING_ON_WINDOWS.md
|-- COMPLETE_MVP.md
|-- PROJECT_STRUCTURE.md
|-- Rust C/C++ Manifesto.md
|-- Working Track .md
|-- quick-build.sh
|-- quick-build.ps1
|-- Docs/
|   |-- INDEX.md
|   |-- rust_c_cpp_spec.md
|   |-- rust_abstractions.md
|   `-- vision.md
|-- include/
|   `-- tcc/
|       |-- ASTVisitor.h
|       |-- BorrowRules.h
|       |-- ConcurrencyRules.h
|       |-- Core.h
|       |-- Diagnostic.h
|       |-- DiagnosticHelper.h
|       |-- FileDetector.h
|       |-- LifetimeRules.h
|       |-- MoveSemanticRules.h
|       |-- OwnershipRules.h
|       |-- Rule.h
|       |-- RuleEngine.h
|       `-- SafetyPatternRules.h
|-- src/
|   |-- ASTVisitor.cpp
|   |-- BorrowRules.cpp
|   |-- ConcurrencyRules.cpp
|   |-- Diagnostic.cpp
|   |-- FileDetector.cpp
|   |-- LifetimeRules.cpp
|   |-- main.cpp
|   |-- MoveSemanticRules.cpp
|   |-- OwnershipRules.cpp
|   |-- Rule.cpp
|   |-- RuleEngine.cpp
|   `-- SafetyPatternRules.cpp
|-- tests/
|   |-- CMakeLists.txt
|   `-- data/
|       |-- pass/
|       `-- fail/
`-- examples/
    |-- 01_smart_pointers_t.cc
    |-- 02_raw_pointer_violations_t.cc
    |-- 03_lifetime_safety_t.cc
    |-- 04_lifetime_violations_t.cc
    |-- 05_thread_safety_t.cc
    |-- 06_thread_violations_t.cc
    |-- 07_move_semantics_t.cc
    |-- 08_move_violations_t.cc
    |-- 09_borrow_checker_t.cc
    |-- 10_borrow_violations_t.cc
    |-- 11_option_result_patterns_t.cc
    `-- 12_safety_violations_t.cc
```

## Module Status / 模块状态

### Core / 核心模块

- `Core.h`, `Diagnostic.*`, `Rule.*`, `RuleEngine.*`, `ASTVisitor.*`: implemented.
- `Core.h`、`Diagnostic.*`、`Rule.*`、`RuleEngine.*`、`ASTVisitor.*`：已实现。

### Rule Families / 规则族

- Ownership (`TCC-OWN-*`): implemented baseline + move extensions.
- Lifetime (`TCC-LIFE-*`): implemented baseline.
- Concurrency (`TCC-CONC-*`): implemented baseline.
- Borrow (`TCC-BORROW-*`): integrated, partially complete.
- Safety patterns (`TCC-OPTION/RESULT/PANIC/SAFE-*`): integrated, partially complete.

### Build / 构建

- Recommended clean verification target: Linux (`build-linux/`).
- 推荐的干净验证目标：Linux（`build-linux/`）。
- Windows build cache should be treated as disposable.
- Windows 构建缓存应视为一次性产物。

## Notes / 备注

- Current progress and blockers are tracked in `Working Track .md`.
- 当前进度与阻塞见 `Working Track .md`。
- Historical "MVP complete" wording should be interpreted as milestone-level, not full implementation completeness.
- 历史上的 “MVP 完成” 表述应视作里程碑语义，不等于当前实现已全部完工。

