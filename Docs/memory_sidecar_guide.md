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

---

## 4) CI gate policy (practical)

- **Hard bug / parser failure**: fail pipeline.
- **Warnings**: record and triage.

This provides low-friction adoption while keeping strong memory-risk guardrails.

---

## 5) quickrt scan result (current local state)

Current quickrt folder scan found no valid C/C++ source files outside generated/binary paths.

- Found candidate before filtering: `quickrt/bin/quickrt.exe.c`
- Actual content is PE/binary, not text C source
- Result: parser errors (expected for binary blob), not actionable memory diagnostics

So current actionable result is:
- **Warnings**: 0 actionable
- **Errors**: 0 actionable RCC memory findings
- **Action needed**: point RCC to real source directories (e.g. `src/`, `include/`) for meaningful findings
