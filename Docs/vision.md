# Tough C — Vision

> This is a philosophy document.
> 这是理念文档。
>
> For implementation status and build readiness, see:
> 实现进度和构建可用性请查看：
> - `Working Track .md`
> - `BUILDING_ON_LINUX.md`

## The C++ Federation Promise

C++ is not a single language. It is a **federation of capabilities**:

- C for raw control
- OOP for structure
- STL for abstraction
- Templates and constexpr for compile-time power
- Undefined behavior and bit-level tricks for last-resort solutions

The true promise of this federation is simple:

> **For every problem, there exists a C++ that can solve it.**

This promise creates engineering confidence and long-term survivability.

---

## The Problem with Early Restrictions

Requiring engineers to give up capabilities at project start
assumes near-perfect foresight.

That assumption is false.

At the beginning of a project:
- performance constraints are unknown
- critical paths are unclear
- hardware and concurrency demands evolve

Languages that permanently restrict capability too early
effectively say:

> *If you guessed wrong, you are stuck.*

C++ never says this.

---

## Tough C’s Role

Tough C does not weaken C++.
It **extends its promise**.

Instead of forbidding power, Tough C **delays it**.

It asks one question:

> **“Do you really need this now?”**

---

## Reversible Safety

Tough C enforces safety only where explicitly requested.

- Safety is local (file-level or module-level)
- Unsafe escape is always available
- Migration is incremental

This reversibility is critical.
It allows engineers to explore safely
without sacrificing future options.

---

## Tough C vs Rust (Philosophical Difference)

Rust guarantees safety by removing options early.

Tough C guarantees survivability by preserving options forever.

Tough C does not compete with Rust.
It serves engineers who need the **C++ federation guarantee**.

---

## AI-Generated Code Reality

As AI generates more C/C++ code:
- humans no longer verify intent line by line
- subtle lifetime and concurrency bugs increase
- review capacity becomes the bottleneck

Tough C shifts responsibility from *writing carefully*
to *allowing code to exist*.

This is the correct abstraction for the AI era.

---

## Long-Term Vision

- A well-defined Tough C profile
- A fast, deterministic verifier
- Clear failure modes
- Zero impact on existing compilers
- Full coexistence with standard C/C++

Tough C aims to become:

> **The safety gate for serious C/C++ systems.**
