# Rust C/C++ Documentation Index
# Rust C/C++ 文档索引

这是 Rust C/C++ (Rust C) Profiler 项目的完整文档索引。
This is the complete documentation index for the Rust C/C++ (Rust C) Profiler project.

---

## 快速导航 / Quick Navigation

### 新用户入口 / New User Entry

1. [User Guide](user_guide.md) - How to actually run rcc-check
2. [Build on Linux](../BUILDING_ON_LINUX.md) - Current supported build path
3. [Integration Plan](integration_plan.md) - VS Code, CI, and Make adoption path
4. [Keywords and Profiles](keywords_and_profiles.md) - RCC/TCC naming and rule prefixes

---

## 核心文档 / Core Documentation

### 1. [README.md](../README.md)
**概述 / Overview**
- 项目介绍和核心哲学 / Project introduction and core philosophy
- Safety is opt-in, power is never removed
- 设计为 AI 时代准备 / Designed for the AI era

### 2. [rust_c_cpp_spec.md](rust_c_cpp_spec.md)
**核心规范 / Core Specification**
- 所有权与生命周期增强规范 / Ownership and lifetime enhancement specification
- 语义升级而非新语法 / Semantic upgrade without new keywords
- Profiler 三位一体流水线 / Three-phase profiler pipeline
- 实现路线图 / Implementation roadmap

### 3. [Rust Abstractions](rust_abstractions.md) ⭐ 新增 / New
**Rust 抽象集成 / Rust Abstractions Integration**
- 7 大核心 Rust 抽象 / 7 core Rust abstractions:
  - 所有权系统 / Ownership system
  - 借用检查器 / Borrow checker
  - Option<T> 模式 / Option pattern
  - Result<T, E> 模式 / Result pattern
  - 生命周期标注 / Lifetime annotations
  - 内部可变性 / Interior mutability
  - 线程安全类型 / Send/Sync traits
- 实现策略和规则清单 / Implementation strategy and rule list
- 集成示例代码 / Integration examples

### 4. [Rust C/C++ Manifesto.md](../Rust_Cpp_Manifesto.md)
**设计理念 / Design Rationale**
- C++ Federation 承诺 / C++ Federation promise
- 为什么早期限制会失败 / Why early restrictions fail
- RCC 的真正价值 / The real value of RCC
- 可逆安全性 / Reversible safety

### 5. [vision.md](vision.md)
**项目愿景 / Project Vision**
- Long-term goals and philosophy
- 长期目标和理念

### 6. [user_guide.md](user_guide.md)
**用户使用指南 / User Guide**
- How to run `rcc-check` in real projects
- Compile database usage and `--` clang-args pattern
- Exit codes, diagnostics, and current limitations

### 7. [keywords_and_profiles.md](keywords_and_profiles.md)
**关键词与配置画像 / Keywords and Profiles**
- RCC/TCC terminology reference
- Rule prefixes, file markers, escape hatches
- Real profiles users can run today

### 8. [integration_plan.md](integration_plan.md)
**集成方案 / Integration Plan**
- Linux-first VS Code workflow
- Real-project onboarding path
- Make integration strategy

### 9. [ci_cd_quickstart.md](ci_cd_quickstart.md)
**CI/CD 快速落地 / CI/CD Quickstart**
- Most smooth low-friction adoption path
- Changed-files-first gate strategy
- Linux runner reference workflow

### 10. [end_to_end.md](end_to_end.md)
**端到端完整指南 / End-to-End Guide**
- One complete path from setup to CI gate
- Practical daily workflow and adoption stages
- Known limits and realistic usage boundaries

---

## 技术文档 / Technical Documentation

### 11. [BUILD.md](../BUILD.md)
**构建说明 / Build Instructions**
- Prerequisites and dependencies
- Build steps for different platforms
- 构建前提和依赖项
- 不同平台的构建步骤

### 12. [BUILDING_ON_WINDOWS.md](../BUILDING_ON_WINDOWS.md)
**Windows 构建 / Windows Build**
- Windows-specific build instructions
- Visual Studio and MSVC setup
- Windows 特定的构建说明

### 13. [BUILDING_ON_LINUX.md](../BUILDING_ON_LINUX.md)
**Linux 构建 / Linux Build**
- Clean Linux validation workflow
- Distro package prerequisites
- LLVM/Clang path troubleshooting
- 干净 Linux 验证流程
- 发行版依赖安装与 LLVM/Clang 路径排查

### 14. [PROJECT_STRUCTURE.md](../PROJECT_STRUCTURE.md)
**项目结构 / Project Structure**
- Directory layout / 目录布局
- Module organization / 模块组织
- File descriptions / 文件说明

### 15. [COMPLETE_MVP.md](../COMPLETE_MVP.md)
**MVP 里程碑记录 / MVP Milestone Record**
- Historical milestone summary
- Refer to Working Track for current status
- 历史里程碑总结
- 当前状态请以 Working Track 为准

### 16. [Working Track .md](../Working%20Track%20.md)
**工作追踪 / Work Tracking**
- Development progress
- Task tracking
- 开发进度
- 任务追踪

---

## 代码示例 / Code Examples

### 示例概览 / Examples Overview

所有示例都位于 `examples/` 目录，分为 **通过** 和 **失败** 两类：
All examples are in the `examples/` directory, categorized as **Pass** and **Fail**:

---

### ✅ 正确示例 (应该通过检查) / Correct Examples (Should Pass)

#### [01_smart_pointers_t.cc](../examples/01_smart_pointers_t.cc)
**智能指针使用 / Smart Pointer Usage**
- std::unique_ptr<T> 唯一所有权 / Unique ownership
- std::shared_ptr<T> 共享所有权 / Shared ownership
- RAII 自动清理 / RAII automatic cleanup
- **Rules**: TCC-OWN-001~004

#### [03_lifetime_safety_t.cc](../examples/03_lifetime_safety_t.cc)
**生命周期安全 / Lifetime Safety**
- 按值返回 / Return by value (RVO)
- const 引用安全 / Safe const references
- 容器值语义 / Container value semantics
- **Rules**: TCC-LIFE-001~004

#### [05_thread_safety_t.cc](../examples/05_thread_safety_t.cc)
**线程安全 / Thread Safety**
- std::mutex 保护共享状态 / Mutex-protected shared state
- std::atomic 原子操作 / Atomic operations
- 线程安全模式 / Thread-safe patterns
- **Rules**: TCC-CONC-001~004

#### [07_move_semantics_t.cc](../examples/07_move_semantics_t.cc) ⭐ 新增 / New
**移动语义 (Rust 风格) / Move Semantics (Rust-style)**
- 移动构造和移动赋值 / Move constructor and assignment
- 所有权转移 / Ownership transfer
- 只移动类型 / Move-only types
- **Rules**: TCC-OWN-005~007

#### [09_borrow_checker_t.cc](../examples/09_borrow_checker_t.cc) ⭐ 新增 / New
**借用检查器 / Borrow Checker**
- 多个不可变借用 / Multiple immutable borrows
- 单个可变借用 / Single mutable borrow
- 顺序借用模式 / Sequential borrow patterns
- **Rules**: TCC-BORROW-001~004

#### [11_option_result_patterns_t.cc](../examples/11_option_result_patterns_t.cc) ⭐ 新增 / New
**Option 和 Result 模式 / Option and Result Patterns**
- std::optional<T> 显式空值处理 / Explicit null handling
- std::variant<T, E> 错误处理 / Error handling
- 安全的值展开 / Safe value unwrapping
- 错误传播 / Error propagation
- **Rules**: TCC-OPTION-001~002, TCC-RESULT-001~002

---

### ❌ 违规示例 (应该失败检查) / Violation Examples (Should Fail)

#### [02_raw_pointer_violations_t.cc](../examples/02_raw_pointer_violations_t.cc)
**原始指针违规 / Raw Pointer Violations**
- ✗ 使用 new/delete / Using new/delete
- ✗ 使用 malloc/free / Using malloc/free
- ✗ 原始所有权指针 / Raw owning pointers
- **Violations**: TCC-OWN-001~004

#### [04_lifetime_violations_t.cc](../examples/04_lifetime_violations_t.cc)
**生命周期违规 / Lifetime Violations**
- ✗ 返回局部变量引用 / Returning reference to local
- ✗ 悬空指针 / Dangling pointers
- ✗ 容器存储原始指针 / Containers with raw pointers
- **Violations**: TCC-LIFE-001~004

#### [06_thread_violations_t.cc](../examples/06_thread_violations_t.cc)
**并发违规 / Concurrency Violations**
- ✗ 无保护的共享状态 / Unprotected shared state
- ✗ 数据竞争 / Data races
- ✗ 非线程安全类型跨线程使用 / Non-thread-safe types across threads
- **Violations**: TCC-CONC-001~004

#### [08_move_violations_t.cc](../examples/08_move_violations_t.cc) ⭐ 新增 / New
**移动语义违规 / Move Semantic Violations**
- ✗ 移动后使用 / Use after move
- ✗ 双重移动 / Double move
- ✗ 传递给函数后使用 / Use after passing to function
- **Violations**: TCC-OWN-005~006

#### [10_borrow_violations_t.cc](../examples/10_borrow_violations_t.cc) ⭐ 新增 / New
**借用违规 / Borrow Violations**
- ✗ 可变和不可变借用冲突 / Mutable and immutable borrow conflict
- ✗ 多个可变借用 / Multiple mutable borrows
- ✗ 借用超出所有者生命周期 / Borrow outlives owner
- ✗ 借用期间修改 / Modification during borrow
- **Violations**: TCC-BORROW-001~004

#### [12_safety_violations_t.cc](../examples/12_safety_violations_t.cc) ⭐ 新增 / New
**安全模式违规 / Safety Pattern Violations**
- ✗ 未检查的 .value() 调用 / Unchecked .value() call
- ✗ 忽略 Result 返回值 / Ignoring Result return
- ✗ 未经边界检查的数组访问 / Unchecked array access
- ✗ 直接调用 abort/exit / Direct abort/exit calls
- **Violations**: TCC-OPTION-001, TCC-RESULT-001, TCC-PANIC-001~002, TCC-SAFE-001

---

## 规则参考 / Rule Reference

### 规则类别 / Rule Categories

#### 所有权规则 / Ownership Rules (TCC-OWN-xxx)
- TCC-OWN-001: 禁止 new / Forbid new operator
- TCC-OWN-002: 禁止 delete / Forbid delete operator
- TCC-OWN-003: 禁止 malloc/free / Forbid malloc/free
- TCC-OWN-004: 检测原始所有权指针 / Detect raw owning pointers
- TCC-OWN-005: 移动后使用检测 / Use-after-move detection ⭐
- TCC-OWN-006: 双重移动检测 / Double move detection ⭐
- TCC-OWN-007: RAII 类型强制移动语义 / Enforce move semantics for RAII ⭐

#### 生命周期规则 / Lifetime Rules (TCC-LIFE-xxx)
- TCC-LIFE-001: 禁止返回局部变量引用 / Forbid dangling references
- TCC-LIFE-002: 禁止返回局部变量指针 / Forbid dangling pointers
- TCC-LIFE-003: 禁止容器存储原始指针 / Forbid raw pointer containers
- TCC-LIFE-004: 禁止无明确生命周期的引用成员 / Forbid untracked reference members
- TCC-LIFE-005: 生命周期绑定分析 / Lifetime binding analysis ⭐

#### 借用规则 / Borrow Rules (TCC-BORROW-xxx) ⭐ 新增 / New
- TCC-BORROW-001: 冲突的可变/不可变借用 / Conflicting mutable/immutable borrows
- TCC-BORROW-002: 借用超出所有者 / Borrow outlives owner
- TCC-BORROW-003: 多个可变借用 / Multiple mutable borrows
- TCC-BORROW-004: 借用期间修改 / Modification during borrow

#### 安全模式规则 / Safety Pattern Rules (TCC-xxx-xxx) ⭐ 新增 / New
- TCC-OPTION-001: 强制空值检查 / Enforce null checks
- TCC-OPTION-002: 优先使用 std::optional / Prefer std::optional
- TCC-RESULT-001: 强制 Result 处理 / Enforce Result handling
- TCC-RESULT-002: 未检查的错误返回 / Unchecked error returns
- TCC-PANIC-001: 不安全的 unwrap / Unsafe unwrap
- TCC-PANIC-002: 禁止 panic/abort / Forbid panic/abort
- TCC-SAFE-001: 强制边界检查 / Enforce bounds checking

#### 并发规则 / Concurrency Rules (TCC-CONC-xxx)
- TCC-CONC-001: 无保护的共享状态 / Unprotected shared state
- TCC-CONC-002: 数据竞争检测 / Data race detection
- TCC-CONC-003: 死锁模式 / Deadlock patterns
- TCC-CONC-004: 线程不安全类型 / Thread-unsafe types
- TCC-MUT-001: 内部可变性安全 / Interior mutability safety ⭐
- TCC-SYNC-001: 线程安全类型检查 / Thread safety checking ⭐

---

## 快速开始 / Quick Start

### 1. 阅读核心文档 / Read Core Documentation
```
1. README.md - 了解项目理念 / Understand project philosophy
2. rust_c_cpp_spec.md - 学习核心规范 / Learn core specifications
3. rust_abstractions.md - 理解 Rust 抽象 / Understand Rust abstractions
```

### 2. 查看示例 / Browse Examples
```
1. 从正确示例开始 / Start with correct examples (01, 03, 05, 07, 09, 11)
2. 对比违规示例 / Compare with violation examples (02, 04, 06, 08, 10, 12)
3. 理解每个规则 / Understand each rule
```

### 3. 构建项目 / Build Project
```bash
# 查看构建说明 / See build instructions
cat BUILD.md

# Windows 用户 / Windows users
cat BUILDING_ON_WINDOWS.md
```

### 4. 运行测试 / Run Tests
```bash
# 构建并运行测试 / Build and run tests
mkdir build && cd build
cmake ..
cmake --build .
ctest
```

---

## 贡献指南 / Contributing

欢迎贡献！请参考：
Contributions welcome! Please see:

- [PROJECT_STRUCTURE.md](../PROJECT_STRUCTURE.md) - 代码组织 / Code organization
- [COMPLETE_MVP.md](../COMPLETE_MVP.md) - 当前任务 / Current tasks
- [Working Track .md](../Working%20Track%20.md) - 工作追踪 / Work tracking

---

## 更新日志 / Changelog

### 2026 - Rust Abstractions Integration and Build Track Reset
- ✅ Added 7 core Rust abstractions / 添加 7 个核心 Rust 抽象
- ✅ New rule categories (Borrow, Safety) / 新规则类别（借用、安全）
- ✅ 6 new comprehensive examples / 6 个新的综合示例
- ✅ Enhanced documentation / 增强文档
- ✅ Extended rule set (25+ rules) / 扩展规则集（25+ 规则）
- ✅ Added Linux-first build guide / 新增 Linux 优先构建指南
- ✅ Updated docs to reflect in-progress status / 更新文档以反映进行中状态

---

## 联系方式 / Contact

项目主页 / Project Home: [GitHub Repository]
文档问题 / Documentation Issues: [Issue Tracker]
讨论 / Discussions: [Discussion Forum]

---

**Rust C/C++ = C++ 的表达力 + Rust 的安全性**
**Rust C/C++ = C++ Expressiveness + Rust Safety**

> Safety is opt-in. Power is never removed. Escape hatches always exist.

