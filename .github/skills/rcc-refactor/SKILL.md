---
name: rcc-refactor
description: "Use when refactoring C/C++ code to pass RCC or rcc-check, fixing TCC-OWN/TCC-LIFE/TCC-CONC diagnostics, replacing raw ownership with RAII, tightening lifetime safety, or making code safe enough for the Rust C/C++ profiler. Prefer for Linux-first RCC migration and iterative rule-driven cleanup."
---

# RCC Refactor Skill

Use this skill when you need to transform existing C/C++ code so it passes the
RCC profiler with minimal behavioral change.
当你需要在尽量不改变行为的前提下，将现有 C/C++ 代码重构为通过 RCC 分析器时，使用本技能。

---

## Scope

This skill is for refactoring code to satisfy the current RCC implementation.
本技能面向当前 RCC 实现下的代码重构。

Prefer it when the task is about:
适用场景：

- fixing `rcc-check` diagnostics
- replacing `new/delete` or `malloc/free`
- eliminating raw owning pointers
- fixing dangling reference / pointer patterns
- fixing Linux-validated concurrency hazards
- removing use-after-move patterns
- preparing files for RCC-managed project conventions

---

## Important Reality Checks

Do not assume all declared RCC rule families are equally complete.
不要假设所有已声明的 RCC 规则家族都已经同等完整。

Currently safest to rely on:
当前最可靠的方向：

- ownership core rules
- lifetime core rules
- tested concurrency rules
- use-after-move detection

Be careful with:
需要谨慎对待：

- borrow-checker-inspired rules
- safety-pattern families that still contain placeholder logic
- `_t.cc` / `@tcc` as runtime enforcement gates

Current CLI reality:
当前 CLI 现实情况：

- RCC analyzes the files explicitly passed to `rcc-check`
- `_t.cc` and `@tcc` are conventions in the repository, not a guaranteed CLI filter
- `@tcc-no-*` annotations exist in `FileDetector`, but are not yet wired into the main CLI execution path

---

## Refactor Workflow

### 1. Identify the real invocation context

Find how the target file is analyzed:
先确定目标文件如何被分析：

- Is it inside a project with `compile_commands.json`?
- Does it need explicit Clang args after `--`?
- Is the current supported path Linux?

Prefer Linux-first validation with:

```bash
./build-linux/src/rcc-check -p build-linux path/to/file.cpp
```

### 2. Fix the highest-confidence rule classes first

Refactor in this order unless the user says otherwise:
除非用户另有要求，优先按以下顺序重构：

1. ownership
2. lifetime
3. concurrency
4. move semantics
5. documentation / marker cleanup

### 3. Preserve behavior, not syntax

Aim to preserve semantics while moving code toward RCC-safe idioms:
目标是保留语义，而不是保留旧写法。

- prefer RAII over manual release
- prefer value semantics over borrowed storage when ownership is unclear
- prefer explicit synchronization over shared mutable state
- prefer local containment of unsafe legacy interfaces

### 4. Re-run RCC after each focused change set

Do not batch many unrelated rewrites before re-checking.
不要在重新验证前堆积大量无关重写。

### 5. Validate with project tests when available

Use `ctest` or the project’s normal test command after RCC passes.
在 RCC 通过后，再用 `ctest` 或项目原有测试进行验证。

---

## Transformation Recipes

### TCC-OWN-001 / TCC-OWN-002

Problem:

- `new` / `delete`

Preferred rewrites:

- `new T(...)` -> `std::make_unique<T>(...)` when ownership is unique
- shared ownership -> `std::make_shared<T>(...)`
- dynamic arrays -> `std::vector<T>` or `std::unique_ptr<T[]>`

### TCC-OWN-003

Problem:

- `malloc/free`

Preferred rewrites:

- plain buffer -> `std::vector<std::byte>` or `std::vector<char>`
- object lifetime -> constructors + RAII containers
- legacy C API boundary -> wrap raw buffer in a narrow adapter layer

### TCC-OWN-004

Problem:

- raw owning pointer fields or variables

Preferred rewrites:

- owning member -> `std::unique_ptr<T>`
- shared graph -> `std::shared_ptr<T>` only when ownership is actually shared
- container of owned heap objects -> `std::vector<T>` or `std::vector<std::unique_ptr<T>>`

### TCC-LIFE-001 / TCC-LIFE-002

Problem:

- returning references or pointers to locals

Preferred rewrites:

- return by value
- move out a value object
- extend ownership to caller explicitly

### TCC-LIFE-003

Problem:

- containers of raw pointers

Preferred rewrites:

- values if polymorphism is not needed
- `std::unique_ptr<T>` for clear ownership
- `std::reference_wrapper<T>` only when lifetime is guaranteed elsewhere

### TCC-LIFE-004

Problem:

- reference members with unclear lifetime ownership

Preferred rewrites:

- store by value
- store `std::shared_ptr<T>` / `std::unique_ptr<T>` depending on ownership model
- store a non-owning handle only when the owning object is structurally guaranteed to outlive it

### TCC-CONC-001

Problem:

- shared mutable global or static state without synchronization

Preferred rewrites:

- simple scalar counter -> `std::atomic<T>`
- composite state -> guard with `std::mutex`
- per-thread state -> `thread_local`

### TCC-CONC-002

Problem:

- risky by-reference lambda capture in concurrent contexts

Preferred rewrites:

- capture by value
- capture immutable snapshot values
- capture synchronized wrapper types instead of raw shared primitives

### TCC-OWN-005

Problem:

- use-after-move

Preferred rewrites:

- stop using the moved-from variable
- restructure control flow so ownership transfer happens last
- reinitialize the object before reuse when semantically valid

---

## Refactor Style Rules

- Prefer the smallest transformation that makes ownership and lifetime explicit.
- Avoid introducing `shared_ptr` when `unique_ptr` or value semantics are sufficient.
- Avoid hiding ownership problems behind casts or container indirection.
- Keep interop with legacy C code at narrow boundaries.
- Do not “fix” code by disabling categories unless the user explicitly wants a relaxed profile.

---

## What Not To Claim

Do not claim any of the following unless you verify them in the current repo state:
在当前仓库状态中未验证前，不要声称以下内容已经成立：

- that `_t.cc` is enforced by the CLI runtime path
- that `@tcc-no-*` per-file config is active end-to-end
- that borrow rules are fully enforced
- that all safety-pattern rules have complete data-flow analysis

---

## Recommended End State

After refactoring, aim for this loop:
重构完成后的理想循环：

1. `rcc-check` passes on Linux
2. project tests still pass
3. ownership model is easier to explain than before
4. the file can remain in the RCC-managed path without category escape hatches
