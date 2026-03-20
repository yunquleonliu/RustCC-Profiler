# RCC Keywords And Profiles
# RCC 关键词与配置画像

This document collects the key user-facing terms, markers, categories, and rule
prefixes used by the RCC profiler.
本文整理 RCC 分析器中用户可见的关键术语、标记、类别与规则前缀。

---

## 1. Core Names

### RCC

`RCC` is the user-facing product name: Rust C/C++.
`RCC` 是面向用户的产品名：Rust C/C++。

You will see it in:
你会在以下位置看到它：

- CLI banner
- success / failure messages
- documentation

### TCC

`TCC` is the internal legacy / implementation-facing prefix still used across
rule ids, namespaces, and some comments.
`TCC` 是仍然保留在规则 ID、命名空间和部分注释中的内部前缀。

You will see it in:
你会在以下位置看到它：

- `namespace tcc`
- rule ids like `TCC-OWN-001`
- class names and source filenames

Practical interpretation:
实践中的理解方式：

- Product branding: `RCC`
- Rule / implementation prefix: `TCC`

---

## 2. File Markers

### `_t.cc`

The repository’s RCC-style file suffix.
仓库中的 RCC 风格文件后缀。

Examples:
示例：

- `01_smart_pointers_t.cc`
- `move_semantics_safe.cpp` in tests is still analyzed because it is explicitly passed to the tool

Current status:
当前状态：

- Defined by `FileDetector`
- Used as a project convention
- Not currently enforced as a mandatory CLI gate

### `@tcc`

Annotation marker recognized by `FileDetector`.
`FileDetector` 识别的注解标记。

Example:

```cpp
// @tcc
```

Current status:
当前状态：

- Recognized by file-detection code
- Not yet wired into the top-level CLI execution path as a hard filter

---

## 3. Per-File Config Annotations

Defined in file-detection code:
在文件检测代码中定义：

- `@tcc-no-ownership`
- `@tcc-no-lifetime`
- `@tcc-no-concurrency`

Intended meaning:
意图含义：

- disable one category for one file
- 针对单个文件禁用一个规则类别

Current status:
当前状态：

- parsed by `FileDetector`
- not yet connected to `RuleEngine` through the CLI path

---

## 4. CLI Keywords

Current supported options:
当前支持的命令行选项：

- `-p <dir>`
- `--verbose`
- `--rcc-version`
- `--no-ownership`
- `--no-lifetime`
- `--no-concurrency`

Not currently present:
当前不存在：

- `--no-safety`

Important parsing keyword:
重要解析关键字：

- `--` separates RCC arguments from Clang arguments

Example:

```bash
./build-linux/src/rcc-check -p build-linux src/foo.cpp -- -std=c++17
```

---

## 5. Severity Keywords

RCC diagnostics use three severity levels:
RCC 诊断使用三种严重级别：

- `Error`
- `Warning`
- `Note`

Practical meaning:
实践含义：

- `Error`: contributes to failing exit code
- `Warning`: informational / advisory unless accompanied by errors
- `Note`: suggestion-level guidance

---

## 6. Rule Prefixes

### `TCC-OWN-*`

Ownership and move semantics.
所有权与移动语义。

Examples:

- `TCC-OWN-001` forbid `new`
- `TCC-OWN-002` forbid `delete`
- `TCC-OWN-003` forbid `malloc/free`
- `TCC-OWN-004` raw owning pointer detection
- `TCC-OWN-005` use-after-move

### `TCC-LIFE-*`

Lifetime and reference safety.
生命周期与引用安全。

Examples:

- dangling reference returns
- dangling pointer returns
- raw pointers in containers
- reference members without clear lifetime tracking

### `TCC-CONC-*`

Concurrency and shared-state checks.
并发与共享状态检查。

Examples:

- unsynchronized mutable global / static state
- risky lambda by-reference capture in concurrent contexts

### `TCC-BORROW-*`

Borrow-checker-inspired rules.
借用检查器风格规则。

Current status:
当前状态：

- terminology exists
- implementation is still partial / skeletal

### `TCC-OPTION-*`

Optional / null-handling guidance.
可选值 / 空值处理指导。

### `TCC-RESULT-*`

Result / error-handling guidance.
结果 / 错误处理指导。

### `TCC-PANIC-*`

Panic / unwrap-style hazard guidance.
panic / unwrap 风险指导。

### `TCC-SAFE-*`

General safety rules such as bounds-related guidance.
如边界访问等通用安全规则。

---

## 7. Profile Ideas Already Present In The Repo

Even though the project does not yet expose formal named profiles, these practical
profiles already exist in behavior and docs.
虽然项目还没有正式暴露命名配置画像，但行为和文档中已经隐含了几种实用画像。

### Full RCC profile

Meaning:

- run all enabled categories
- use normal `rcc-check -p build-linux ...`

### Ownership-relaxed profile

Meaning:

- run without ownership rules

Command:

```bash
./build-linux/src/rcc-check -p build-linux src/foo.cpp --no-ownership
```

### Lifetime-relaxed profile

Command:

```bash
./build-linux/src/rcc-check -p build-linux src/foo.cpp --no-lifetime
```

### Concurrency-relaxed profile

Command:

```bash
./build-linux/src/rcc-check -p build-linux src/foo.cpp --no-concurrency
```

These are not yet named profiles in code, but they are the real operational
profiles users can run today.
这些在代码中尚未作为正式 profile 命名，但它们就是用户今天真正可运行的配置画像。

---

## 8. Escape-Hatch Keywords

The repo consistently uses the idea of escape hatches.
仓库始终保留 escape hatch 的概念。

Current escape-hatch patterns include:
当前 escape hatch 模式包括：

- stop using RCC on that file
- remove RCC annotation conventions
- disable a category from the CLI
- move code outside the strict RCC-managed path

This matches the project philosophy: safety is opt-in and power is not removed.
这与项目哲学一致：安全是可选择的，能力不会被剥夺。

---

## 9. Terms To Use Carefully In Future Docs

These terms should be used precisely:
这些术语在后续文档中应当谨慎准确使用：

- `supported`: currently validated on Linux only
- `implemented`: should mean the rule has real analysis, not just a class skeleton
- `detected automatically`: should only be used if CLI/runtime actually enforces it
- `profile`: should mean either a real CLI mode or a clearly documented convention

---

## 10. Recommended Stable Vocabulary

For future user-facing material, prefer this wording:
未来面向用户的文案建议优先采用以下表达：

- `RCC profiler`
- `rcc-check`
- `Linux-supported build path`
- `rule category`
- `rule id`
- `escape hatch`
- `project convention` for `_t.cc` and `@tcc` until CLI enforcement is wired up
