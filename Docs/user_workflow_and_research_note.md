# RCC User Workflow and Research Note
# RCC 用户工作流与研究说明

This note is for both day-to-day users and project owners.
本文同时面向日常用户与项目维护者。

---

## 1. Simplified User Workflow

Target workflow:
目标工作流：

1. Keep writing normal C/C++ (`.cc`, `.cpp`) or migrate selected files to RCC lane (`_t.cc` or `@tcc` convention).
2. Refactor with AI + RCC rules.
3. Run RCC before compile.
4. Compile and build as usual.

Recommended one-command entrypoints:
推荐的一条命令入口：

```bash
scripts/rcc-check-file.sh path/to/file.cc
```

```bash
scripts/rcc-check-changed.sh --tcc-only
```

These wrappers reduce manual `-p` and fallback include management.
这些包装脚本可以减少手工处理 `-p` 与回退包含路径参数。

---

## 2. Finished vs Not Finished Checklist

### Finished

- `rcc-check` now fails when no source file is provided.
- `rcc-check` now fails when source file path does not exist.
- Non-zero Clang processing result now returns compilation error exit code.
- New helper script: `scripts/rcc-check-file.sh`.
- New helper script: `scripts/rcc-check-changed.sh`.
- New native fallback mode: `--auto-stdcpp-includes`.

### Not Finished Yet

- `_t.cc` / `@tcc` are still conventions, not an enforced runtime file-selection gate.
- `@tcc-no-*` per-file policy is not fully wired into active rule execution.
- Some declared advanced rules remain partial/in-progress.

---

## 3. Why This Matters

RCC is most useful when it acts as a sidecar safety gate:
RCC 的最大价值在于作为“侧边安全闸门”：

- minimal disruption to existing projects
- no compiler replacement
- clear pass/fail contract before compile

This keeps adoption realistic for large C++ codebases.
这使得大型 C++ 代码库的落地更现实。

---

## 4. ArXiv Positioning (Draft)

Suggested claim style:
建议的论文主张风格：

- Avoid: "C++17 is as safe as Rust".
- Prefer: "A Rust-inspired policy profile can make C++17 workflows significantly safer in practice, while preserving C++ compatibility and escape hatches."

Reason:
原因：

- Rust gives language-level guarantees.
- RCC gives policy/tooling-level enforcement.
- These are different guarantee tiers.

A stronger but still defensible title candidate:
一个更强但仍可辩护的题目候选：

- "RustCC: A Pre-Compilation Safety Profile for Incremental Hardening of C++17"

---

## 5. Evidence Pack Needed for Publication

Before publishing claims, collect:
在发表主张之前，建议补齐：

1. Benchmarks on real projects (defect classes caught, false positives, overhead).
2. Before/after migration case studies.
3. CI gate outcomes over time.
4. Rule coverage matrix (implemented vs planned).
5. Reproducible scripts and datasets.

Without this pack, keep claims directional, not absolute.
没有这些证据时，主张应保持“方向性”，避免“绝对性”。
