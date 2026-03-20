# Draft ArXiv Paper: RustCC and Incremental Safety Hardening for C++17

## Working Title

RustCC: A Pre-Compilation Safety Profile for Incremental Hardening of C++17 Codebases

## Abstract (Draft)

Modern C++ systems often require low-level control while operating in environments where code volume and change velocity exceed manual review capacity. We present RustCC, a pre-compilation safety profiler for C++ that enforces a Rust-inspired policy profile before standard compilation. Rather than replacing compilers or introducing a new language, RustCC adds a policy gate over existing C++ toolchains. The gate emits actionable diagnostics and integrates with CI via exit-code contracts. Our design goal is incremental adoption: teams can apply checks to selected files, retain escape hatches, and preserve existing build infrastructure. We describe the rule taxonomy, deployment workflow, and practical integration constraints. We argue that policy-enforced C++17 can become significantly safer in practice, while noting that this differs from Rust's language-level guarantees.

## Core Claim Boundaries

Claims to make:

- RustCC improves practical safety outcomes in C++ workflows through pre-compilation policy enforcement.
- Rust-inspired constraints can be adopted incrementally without replacing compilers.
- Safety gating can be integrated with existing CI and build pipelines.

Claims to avoid (without stronger evidence):

- "C++17 is equivalent to Rust in safety guarantees."
- "RustCC eliminates all memory and concurrency defects."

## Suggested Contributions Section

1. A compatibility-first architecture for safety gating in standard C++ toolchains.
2. A rule family taxonomy for ownership, lifetime, concurrency, and Rust-inspired patterns.
3. An incremental rollout model for real repositories (single-file, changed-files, CI gate).
4. A reproducible evaluation protocol for detection utility and integration overhead.

## Evaluation Plan

1. Detection utility:
- measure violations found by category
- measure confirmed true positives and false positives

2. Migration trajectory:
- baseline defect profile before RustCC gate
- trend over staged adoption windows

3. Overhead:
- runtime overhead per file
- CI runtime impact

4. Developer friction:
- command complexity before/after helper tooling
- fix turnaround time

## Threats to Validity

- Rule coverage is incomplete for some advanced checks.
- Detection quality depends on compile database quality and invocation fidelity.
- Results may vary by codebase domain and coding style.

## Reproducibility Checklist

- Pin compiler and LLVM/Clang versions.
- Publish command recipes and helper scripts.
- Publish representative pass/fail datasets.
- Publish rule coverage matrix with implemented vs planned checks.

## Current Repo Mapping

- User workflow and implementation status: Docs/user_workflow_and_research_note.md
- Practical execution path: Docs/user_guide.md
- End-to-end operation: Docs/end_to_end.md
- Integration strategy: Docs/integration_plan.md
