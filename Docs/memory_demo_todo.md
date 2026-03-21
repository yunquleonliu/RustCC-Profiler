# Memory Vulnerability Demo TODO

## Goal
Deliver a minimal, credible, sidecar demo that shows RustCC can find severe memory risks in existing C/C++ code without rewriting user files.

## Execution Checklist

- [x] Define memory-demo execution plan
- [x] Add severe memory bug demo file (`examples/13_memory_hardbugs_t.cc`)
- [x] Add fail test fixture (`tests/data/fail/memory_hardbugs.cpp`)
- [x] Wire test in `tests/CMakeLists.txt`
- [x] Add end-to-end demo guide (`Docs/memory_vulnerability_demo.md`)
- [x] Update discoverability in `README.md`
- [x] Update discoverability in `BUILD.md`
- [x] Run tests and verify expected failures are detected

## Acceptance Criteria

1. Demo file contains typical severe patterns (double free, use-after-free precursor, leak-prone ownership).
2. `rcc-check` reports violations using existing rules (e.g., `TCC-OWN-*`, `TCC-LIFE-*`).
3. CI-style command can run the demo in one line.
4. Documentation clearly explains:
   - sidecar model
   - no source rewrite required
   - hard-bug-first interpretation
