# RCC Memory Sidecar Guide (Windows-first)

## Goal

Run RCC as a sidecar scanner to find memory-risk issues in existing C/C++ code, with near one-command onboarding and no source rewrite.

For a machine without local RCC build environment, use the portable flow in [rcc_portable_quickstart.md](rcc_portable_quickstart.md).

---

## 1) One-time setup

1. Build RCC once in this repo (already supported by `quick-build.ps1`).
2. Confirm scanner exists:
   - `build/src/Release/rcc-check.exe`

If you do not want any setup on target machine, skip to the standalone portable doc:
- [rcc_portable_quickstart.md](rcc_portable_quickstart.md)

---

## 2) One-file scan (quick check)

PowerShell:

```powershell
& ".\build\src\Release\rcc-check.exe" --auto-stdcpp-includes -p ".\build" "C:\path\to\file.cpp"
```

Use this when you want immediate diagnostics for a specific file.

---

## 3) Near one-click folder scan (recommended)

Use the helper script:

```powershell
.\scripts\rcc-check-dir.ps1 -TargetPath "C:\path\to\project"
```

Optional TCC filter:

```powershell
.\scripts\rcc-check-dir.ps1 -TargetPath "C:\path\to\project" -TccOnly
```

What it does:
- Recursively finds `*.c`, `*.cc`, `*.cpp`, `*.cxx`
- Excludes common generated/binary folders (`bin`, `build`, `dist`, `.git`, etc.)
- Skips likely binary masquerading as source (e.g. PE `MZ` header)
- Runs RCC per file and returns non-zero when any file fails

Linux shell equivalent:

```bash
./scripts/rcc-check-dir.sh ../quickrt
```

Optional TCC filter:

```bash
./scripts/rcc-check-dir.sh ../quickrt --tcc-only
```

---

## 4) CI gate policy (practical)

- **Hard bug / parser failure**: fail pipeline.
- **Warnings**: record and triage.

This provides low-friction adoption while keeping strong memory-risk guardrails.

---

## 5) quickrt scan result (current local state)

`quickrt` has real C/C++ source files in `src/` and `include/`, so RCC sidecar scan is applicable.

Linux scan sample (8 files under `quickrt/src`):
- `ok=0`, `fail=8`
- Primary blocker: missing Windows SDK headers (`windows.h`, `winsock2.h`, `winsdkver.h`)

Interpretation:
- The sidecar pipeline is working (RCC can traverse external repo files and emit diagnostics).
- For Windows-specific projects, run scans on a Windows runner (or Linux with a full cross-toolchain/sysroot) to avoid header-resolution failures.

Actionable recommendation:
- Keep Linux as smoke scan for parser/rule robustness.
- Use Windows sidecar scan as the source of truth for memory findings in `quickrt`.

---

## 6) Known-project focus (recommended for persuasion)

For external validation and broader credibility, prioritize well-known projects over private/unknown codebases.

Current local candidates:
- `../llama.cpp` (widely known, high visibility)
- `../clusterGPU` (large systems-level codebase)

### 6.1 llama.cpp (high-confidence sidecar target)

`llama.cpp` already has compile databases (e.g. `../llama.cpp/build/compile_commands.json`), so RCC can run with better context.

Example commands:

```bash
./build-linux/src/rcc-check --auto-stdcpp-includes -p ../llama.cpp/build ../llama.cpp/src/llama.cpp
./build-linux/src/rcc-check --auto-stdcpp-includes -p ../llama.cpp/build ../llama.cpp/tests/test-chat.cpp
```

Observed sample findings (rule-code counts):
- `src/llama.cpp`: `TCC-OWN-001`, `TCC-LIFE-002`, `TCC-LIFE-003`, `TCC-CONC-001`, `TCC-SAFE-001`, `TCC-OPTION-002`
- `tests/test-chat.cpp`: `TCC-CONC-001`, `TCC-LIFE-001`, `TCC-RESULT-002`, `TCC-SAFE-001`

### 6.2 clusterGPU (medium confidence until compile DB exists)

`clusterGPU` currently has no discovered `compile_commands.json`, so scan quality is limited by missing project include paths and toolchain headers (`cuda.h`, local headers, SIMD setup).

Example fallback command:

```bash
./scripts/rcc-check-file.sh ../clusterGPU/gdrcopy/src/gdrapi.c
```

Typical blocker in current state:
- `fatal error: 'gdrdrv.h' file not found`
- `fatal error: 'cuda.h' file not found`

Recommendation:
- Generate compile database first (CMake with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) and re-run RCC on selected modules.

### 6.3 llama.cpp false-positive triage (must-do)

Initial result on `llama.cpp` includes mixed-quality findings. For persuasive reporting, split findings into confidence buckets after source review.

Reviewed examples from current local scan:

- Likely false positive:
   - `TCC-LIFE-001` at `tests/test-chat.cpp` lines 33/42/59 (`return os;` in stream operators). Returned reference points to input parameter, not local temporary.
   - `TCC-LIFE-002` at `src/llama.cpp` line 234 (`return model;` where `model` was heap-allocated via `new`).

- High-confidence issue candidate:
   - `TCC-CONC-001` at `src/llama.cpp` line 337 (`static std::string s; s.clear(); ...`). This is shared mutable static state and can race under concurrent calls.

- Medium-confidence (context-dependent):
   - `TCC-CONC-001` on mutable global test fixtures in `tests/test-chat.cpp` (line 171+). Valid as thread-safety warning, but impact depends on test execution model.

Policy for external reports:
- Only publish high-confidence and medium-confidence findings.
- Keep likely false positives in an appendix with reviewer notes.
- Require each published finding to include: rule code, source location, manual review note, and reproduction command.
