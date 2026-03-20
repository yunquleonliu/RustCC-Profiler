# RCC CI/CD Quickstart
# RCC CI/CD 快速落地指南

This is the fastest low-friction way to get real value from RCC in CI/CD.
这是在 CI/CD 中快速获得 RCC 价值的最低摩擦方案。

---

## Goal

Use RCC as a policy gate without forcing a risky whole-repo migration.
把 RCC 作为策略门禁使用，但不强行一次性迁移整个仓库。

---

## Smoothest Path (Recommended)

### Step 1: Linux baseline only

Start with Linux runners only.
先只用 Linux runner。

Why:

- current validated build/test path is Linux
- less environment drift
- fastest path to stable signal

### Step 2: Check changed files first

Gate only changed C/C++ files, not the entire codebase.
先只门禁变更的 C/C++ 文件，而不是全仓。

Why:

- immediate value on new code
- avoids legacy migration shock
- keeps CI time short

### Step 3: Use compile database from existing build

Run RCC with `-p build-linux` using your existing CMake compile database.
用现有 CMake 编译数据库，通过 `-p build-linux` 运行 RCC。

### Step 4: Fail only on RCC errors

RCC exit code `1` should block the pipeline.
RCC 返回码 `1` 时阻塞流水线。

Warnings/notes can remain informational during adoption.
在导入期可先将 warning/note 作为提示信息。

### Step 5: Expand scope gradually

After 1-2 stable sprints:
稳定 1-2 个迭代后：

- expand from changed files to selected directories
- then to broader project scope

---

## Reference CI Job (Linux)

```yaml
name: RCC Gate

on:
  pull_request:
    branches: [ main ]

jobs:
  rcc-check:
    runs-on: ubuntu-22.04

    steps:
      - uses: actions/checkout@v4

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y \
            cmake ninja-build build-essential \
            clang-14 llvm-14-dev libclang-14-dev

      - name: Configure
        run: |
          cmake -S . -B build-linux -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DLLVM_DIR=/usr/lib/llvm-14/lib/cmake/llvm \
            -DClang_DIR=/usr/lib/llvm-14/lib/cmake/clang \
            -DRCC_BUILD_TESTS=ON

      - name: Build rcc-check
        run: cmake --build build-linux --config Release

      - name: Collect changed C/C++ files
        id: changes
        run: |
          files=$(git diff --name-only origin/main...HEAD | grep -E '\\.(c|cc|cpp|cxx)$$' || true)
          echo "files<<EOF" >> $GITHUB_OUTPUT
          echo "$files" >> $GITHUB_OUTPUT
          echo "EOF" >> $GITHUB_OUTPUT

      - name: Run RCC on changed files
        if: steps.changes.outputs.files != ''
        run: |
          while IFS= read -r f; do
            [ -z "$f" ] && continue
            ./build-linux/src/rcc-check -p build-linux "$f"
          done <<< "${{ steps.changes.outputs.files }}"
```

---

## Local Developer Shortcut (Before CI)

Run RCC locally on staged files:
在本地对暂存文件运行 RCC：

```bash
git diff --name-only --cached | grep -E '\\.(c|cc|cpp|cxx)$' | while read -r f; do
  [ -z "$f" ] && continue
  ./build-linux/src/rcc-check -p build-linux "$f"
done
```

This prevents most CI surprises.
这能在大多数情况下避免 CI 才暴露问题。

---

## Most Common Adoption Mistakes

1. Gating the whole legacy tree on day one.
2. Running RCC without a valid compile database.
3. Mixing unsupported environments too early.
4. Treating warning-level findings as immediate hard failures before the team is ready.

---

## Practical Policy Levels

### Level A: Observe only

- run RCC in CI
- collect output
- do not fail PRs yet

### Level B: Gate changed files (recommended first gate)

- fail only when RCC errors appear in changed files

### Level C: Gate selected directories

- enforce RCC for agreed high-value modules

### Level D: Broad gate

- enforce RCC for most project code

Move levels only when the previous level is stable.
只有当前一级稳定后再升级。
