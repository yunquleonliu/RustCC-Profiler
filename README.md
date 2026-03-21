# Rust C/C++ (RCC)

**Rust C/C++** is a pre-compilation profiler that enforces memory and concurrency safety rules for C/C++ *before* code is allowed to exist.

Rust C/C++ does not replace C or C++.  It **completes the C++ federation promise**.

> *C Federation always covers your butt.*

---

## What Is Rust C/C++?

Rust C/C++ is **not**:
- a new programming language
- a new compiler
- a style guide or lint tool

Rust C/C++ **is**:
- a **pre-compilation verifier**
- with **hard rejection power**
- enforcing a **Rust C/C++ profile**
- while preserving **full C/C++ escape hatches**

If code fails Rust C/C++ rules, it **does not get compiled**.

---

## Why Rust C/C++ Exists

Programmers choose C/C++ not because they ignore danger, but because C++ makes a unique promise:

> **You can do anything.**

That promise matters when:
- performance suddenly becomes critical
- hardware-level control is required
- concurrency models must change
- no higher-level abstraction fits

Languages that restrict capability *too early* trap engineers when their initial judgment turns out wrong.

C++ never traps you.

Rust C/C++ extends this promise by adding **reversible safety**.

---

## Core Philosophy

- Safety is **opt-in**
- Power is **never removed**
- Escape hatches always exist

If Rust C/C++ becomes too restrictive:
- move the file out of Rust C/C++
- drop to raw C/C++
- use pointer arithmetic, bit tricks, or hardware primitives

No capability is lost. It is only **postponed until truly needed**.

---

## Designed for the AI Era

In an age where:
- large amounts of code are AI-generated
- humans no longer inspect every line
- correctness matters more than elegance

The real question becomes:

> **Should this code be allowed to exist at all?**

Rust C/C++ answers that question.

---

## What RCC Enforces Today

### Core Safety Rules

- **Ownership discipline** - Smart pointer enforcement, move semantics tracking
- **Lifetime safety** - Borrow checker, reference validation
- **Explicit concurrency models** - Thread safety annotations

### Rust-Inspired Abstractions

Rust C/C++ integrates Rust-inspired abstractions:

- **Move Semantics** (TCC-OWN-005~007) - Use-after-move detection, ownership transfer tracking
- **Borrow Checker** (TCC-BORROW-001~004) - Mutable/immutable borrow conflicts, lifetime binding
- **Option Pattern** (TCC-OPTION-001~002) - Explicit null handling, prefer `std::optional`
- **Result Pattern** (TCC-RESULT-001~002) - Mandatory error handling, no ignored returns
- **Safety Patterns** (TCC-SAFE-001, TCC-PANIC-001~002) - Bounds checking, no unsafe unwraps

See [Rust Abstractions Documentation](Docs/rust_abstractions.md) for conceptual details.

Rust C/C++ does **not** try to prove programs correct.
It defines **what is acceptable to write**.

---

## Current Status

🚧 **Active Development (Implementation Ahead of Build Verification)**

Validated and stable now:
- ✅ Linux-first build and test path
- ✅ Linux-only CI pipeline is green
- ✅ Core ownership/lifetime/concurrency checks used by tests
- ✅ Use-after-move checks used by tests

Still in progress:
- ⚠ Some declared borrow and safety-pattern rules are partial/placeholder
- ⚠ File-detector annotations exist but are not yet wired as a hard CLI gate

Recommended validation path:
- Build and verify on Linux (`BUILDING_ON_LINUX.md`)
- Windows support is deferred for now and will be added back later

---

## Start Here

If you are new to this project, read in this order:

1. [User Guide](Docs/user_guide.md)
2. [Build on Linux](BUILDING_ON_LINUX.md)
3. [CI/CD Quickstart](Docs/ci_cd_quickstart.md)
4. [Integration Plan](Docs/integration_plan.md)
5. [Keywords and Profiles](Docs/keywords_and_profiles.md)
6. [End-to-End Guide](Docs/end_to_end.md)
7. [User Workflow and Research Note](Docs/user_workflow_and_research_note.md)

Quick one-command usage:

```bash
scripts/rcc-check-file.sh examples/01_smart_pointers_t.cc
```

```bash
scripts/rcc-check-changed.sh --tcc-only
```

---

## Documentation

- [Documentation Index](Docs/INDEX.md) - Complete map of available docs
- [User Guide](Docs/user_guide.md) - Practical usage of rcc-check
- [Keywords and Profiles](Docs/keywords_and_profiles.md) - RCC/TCC terms and profile modes
- [CI/CD Quickstart](Docs/ci_cd_quickstart.md) - Fastest low-friction CI adoption path
- [Integration Plan](Docs/integration_plan.md) - VS Code, CI, and Make rollout strategy
- [End-to-End Guide](Docs/end_to_end.md) - One complete path from setup to production usage
- [User Workflow and Research Note](Docs/user_workflow_and_research_note.md) - simplified workflow and publication-safe claim framing
- [ArXiv Draft](Docs/arxiv_draft_rustcc_cpp17.md) - manuscript starter with bounded claims and evaluation plan
- [rust_c_cpp_spec.md](Docs/rust_c_cpp_spec.md) - Core specification and semantic enhancements
- [Rust Abstractions](Docs/rust_abstractions.md) - Integrated Rust safety patterns
- [Vision](Docs/vision.md) - Project philosophy and goals
- [Manifesto](Rust_Cpp_Manifesto.md) - Design rationale
- [Building](BUILD.md) - Build instructions
- [Build on Linux](BUILDING_ON_LINUX.md) - Current supported verification workflow
- [Build on Windows](BUILDING_ON_WINDOWS.md) - Deferred setup notes for future re-enable
- [Project Structure](PROJECT_STRUCTURE.md) - Code organization
- [Working Track](Working%20Track%20.md) - Live implementation status

---

## License

ALL RIGHTS RESERVED! 

TBD (likely ONLY open to contributors). Strictly exclude all big (over 10 employees) businesses for now.

---

## Slogan

> **Rust C/C++ does not reduce what C/C++ can do.  
It only delays when you must do it.**

