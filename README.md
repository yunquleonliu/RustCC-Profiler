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

## What Rust C/C++ Enforces (MVP Scope)

### Core Safety Rules

- **Ownership discipline** - Smart pointer enforcement, move semantics tracking
- **Lifetime safety** - Borrow checker, reference validation
- **Explicit concurrency models** - Thread safety annotations

### Rust-Inspired Abstractions

Rust C/C++ integrates validated Rust memory safety abstractions:

- **Move Semantics** (TCC-OWN-005~007) - Use-after-move detection, ownership transfer tracking
- **Borrow Checker** (TCC-BORROW-001~004) - Mutable/immutable borrow conflicts, lifetime binding
- **Option Pattern** (TCC-OPTION-001~002) - Explicit null handling, prefer `std::optional`
- **Result Pattern** (TCC-RESULT-001~002) - Mandatory error handling, no ignored returns
- **Safety Patterns** (TCC-SAFE-001, TCC-PANIC-001~002) - Bounds checking, no unsafe unwraps

See [Rust Abstractions Documentation](Docs/rust_abstractions.md) for complete details.

Rust C/C++ does **not** try to prove programs correct.
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

- [rust_c_cpp_spec.md](Docs/rust_c_cpp_spec.md) - Core specification and semantic enhancements
- [Rust Abstractions](Docs/rust_abstractions.md) - Integrated Rust safety patterns
- [Vision](Docs/vision.md) - Project philosophy and goals
- [Manifesto](Rust_Cpp_Manifesto.md) - Design rationale
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

> **Rust C/C++ does not reduce what C/C++ can do.  
It only delays when you must do it.**

