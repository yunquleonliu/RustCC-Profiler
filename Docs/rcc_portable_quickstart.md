# RCC Portable Quickstart (No local RCC build environment)

## Purpose

Use RCC on another Windows machine without building RCC there.

---

## A. Create a portable bundle (on your current machine)

From repo root:

```powershell
.\scripts\package-rcc-portable.ps1
```

Output:
- `dist/rcc-portable-<timestamp>.zip`

What is included:
- `bin/rcc-check.exe`
- `bin/*.dll` (LLVM runtime DLLs)
- `scripts/rcc-check-dir.ps1`
- `run-rcc-dir.ps1` (entry command)
- this quickstart doc

---

## B. Use on a clean machine

1. Copy the zip to target machine.
2. Unzip (for example to `D:\rcc-portable`).
3. Open PowerShell in that folder.
4. Run:

```powershell
.\run-rcc-dir.ps1 -TargetPath "D:\your_project"
```

Optional filter:

```powershell
.\run-rcc-dir.ps1 -TargetPath "D:\your_project" -TccOnly
```

---

## C. Expected behavior

- Recursively scans `*.c`, `*.cc`, `*.cpp`, `*.cxx`
- Skips common generated folders (`bin`, `build`, `dist`, `.git`, etc.)
- Skips likely binary files (e.g., PE/MZ)
- Returns non-zero exit code if any file scan fails

---

## D. Troubleshooting

- If PowerShell blocks scripts:

```powershell
powershell -ExecutionPolicy Bypass -File .\run-rcc-dir.ps1 -TargetPath "D:\your_project"
```

- If no source files are found, verify target path contains real text C/C++ sources and not generated binary artifacts.
