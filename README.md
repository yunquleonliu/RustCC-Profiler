# Tough C (TCC)

**Tough C** is a pre-compilation profiler that enforces memory and concurrency safety rules for C/C++ *before* code is allowed to exist.

Tough C does not replace C or C++.  It **completes the C++ federation promise**.

> *C Federation always covers your butt.*

---

## What Is Tough C?

Tough C is **not**:
- a new programming language
- a new compiler
- a style guide or lint tool

Tough C **is**:
- a **pre-compilation verifier**
- with **hard rejection power**
- enforcing a **Tough C profile**
- while preserving **full C/C++ escape hatches**

If code fails Tough C rules, it **does not get compiled**.

---

## Why Tough C Exists

Programmers choose C/C++ not because they ignore danger, but because C++ makes a unique promise:

> **You can do anything.**

That promise matters when:
- performance suddenly becomes critical
- hardware-level control is required
- concurrency models must change
- no higher-level abstraction fits

Languages that restrict capability *too early* trap engineers when their initial judgment turns out wrong.

C++ never traps you.

Tough C extends this promise by adding **reversible safety**.

---

## Core Philosophy

- Safety is **opt-in**
- Power is **never removed**
- Escape hatches always exist

If Tough C becomes too restrictive:
- move the file out of Tough C
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

Tough C answers that question.

---

## What Tough C Enforces (MVP Scope)

### Core Safety Rules

- **Ownership discipline** - Smart pointer enforcement, move semantics tracking
- **Lifetime safety** - Borrow checker, reference validation
- **Explicit concurrency models** - Thread safety annotations

### Rust-Inspired Abstractions

Tough C integrates validated Rust memory safety abstractions:

- **Move Semantics** (TCC-OWN-005~007) - Use-after-move detection, ownership transfer tracking
- **Borrow Checker** (TCC-BORROW-001~004) - Mutable/immutable borrow conflicts, lifetime binding
- **Option Pattern** (TCC-OPTION-001~002) - Explicit null handling, prefer `std::optional`
- **Result Pattern** (TCC-RESULT-001~002) - Mandatory error handling, no ignored returns
- **Safety Patterns** (TCC-SAFE-001, TCC-PANIC-001~002) - Bounds checking, no unsafe unwraps

See [Rust Abstractions Documentation](Docs/rust_abstractions.md) for complete details.

Tough C does **not** try to prove programs correct.
It defines **what is acceptable to write**.

---

## Project Status

🚧 **Active Development (Implementation Ahead of Build Verification)**

What is implemented:
- ✅ Core rule engine with Clang AST integration
- ✅ Ownership, lifetime, and concurrency rule modules
- ✅ Rust-inspired extension modules (move / borrow / safety patterns)
- ✅ Expanded examples and test data

What is still in progress:
- ⚠ Several newly added rules still contain placeholder analysis logic
- ⚠ Full build/test verification is pending on clean Linux environment
- ⚠ Some docs are being reconciled from historical "MVP complete" language

Recommended validation path:
- Build and verify on Linux first (`BUILDING_ON_LINUX.md`)
- Then replay validation on Windows if needed

---

## Documentation

- [tough C .md](Docs/tough%20C%20.md) - Core specification and semantic enhancements
- [Rust Abstractions](Docs/rust_abstractions.md) - Integrated Rust safety patterns
- [Vision](Docs/vision.md) - Project philosophy and goals
- [Manifesto](Tough%20C%20Menifesto.md) - Design rationale
- [Building](BUILD.md) - Build instructions
- [Build on Linux](BUILDING_ON_LINUX.md) - Clean Linux verification workflow
- [Build on Windows](BUILDING_ON_WINDOWS.md) - Windows setup and troubleshooting
- [Project Structure](PROJECT_STRUCTURE.md) - Code organization
- [Working Track](Working%20Track%20.md) - Live implementation status

---

## License

TBD (likely permissive, e.g. Apache-2.0 or MIT)

---

## Slogan

> **Tough C does not reduce what C/C++ can do.  
It only delays when you must do it.**
