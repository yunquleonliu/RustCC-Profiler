# Linux-First Integration Plan
# Linux 优先集成方案

This document turns the remaining integration TODO items into a concrete,
current-state plan based on what the repository already supports today.
本文将剩余集成任务转化为基于当前仓库真实能力的具体方案。

---

## 1. Current Supported Baseline

Today’s supported verification path is:
当前受支持的验证路径为：

- Linux only
- `rcc-check` built in `build-linux`
- analysis driven by `compile_commands.json`
- exit-code based integration

This is already proven by the Linux-only GitHub Actions workflow and the green
Linux test suite.
这一点已经由 Linux-only GitHub Actions 工作流与绿色测试集证明。

---

## 2. VS Code Integration Plan

### Phase 1: Zero-extension integration

The fastest useful path is a workspace task plus a problem matcher.
最快速且实用的路径是工作区任务加 problem matcher。

Suggested task shape:
建议任务形式：

```json
{
  "label": "rcc-check current file",
  "type": "shell",
  "command": "${workspaceFolder}/build-linux/src/rcc-check",
  "args": [
    "-p", "${workspaceFolder}/build-linux",
    "${file}"
  ],
  "problemMatcher": "$rcc-check"
}
```

Suggested problem matcher based on current diagnostic format:
基于当前诊断格式的 problem matcher 建议：

```json
{
  "name": "rcc-check",
  "owner": "cpp",
  "fileLocation": ["absolute"],
  "pattern": {
    "regexp": "^(.*):(\\d+):(\\d+): (error|warning|note) / .*: (.*)$",
    "file": 1,
    "line": 2,
    "column": 3,
    "severity": 4,
    "message": 5
  }
}
```

Why this works:
它可行的原因：

- RCC diagnostics already use `file:line:column:`
- severity is stable at the front of the message
- VS Code can surface findings directly in Problems view

### Phase 2: File-selection workflow

Because the CLI currently analyzes whatever file you pass, the initial VS Code
workflow should target:
由于当前 CLI 会分析你显式传入的文件，因此 VS Code 第一阶段应针对：

- current file
- selected file list
- changed files in git

### Phase 3: Optional extension later

When the repo is ready, a dedicated extension could add:
未来若仓库成熟，可增加专用扩展：

- “Run RCC on current file” command
- “Convert file to RCC convention” command
- quick-fix links to examples / docs
- project detection for `build-linux` and `compile_commands.json`

Do not start with an extension.
不要一开始就做扩展。

The zero-extension task-based workflow is enough to prove value.
基于任务的零扩展方案已足够证明价值。

---

## 3. Real-Project Workflow Plan

The near-term workflow for real local projects should be incremental.
面向真实本地项目的近期工作流应是增量式的。

### Stage 1: Project enablement

Requirements:
要求：

- generate `compile_commands.json`
- build successfully on Linux
- choose an initial target subset of files

Recommended starting set:
建议起始集合：

- one module
- one library boundary
- one set of newly touched files

### Stage 2: RCC gate on selected files

Do not try to gate the whole legacy tree on day one.
不要在第一天就尝试对整个遗留代码树做强制门禁。

Start with:
建议从以下方式开始：

- new code only
- changed files only
- explicitly curated RCC-managed directories

### Stage 3: Expand coverage

After the first subset becomes stable:
首批子集稳定后：

- expand file coverage gradually
- reduce category opt-outs
- convert conventions into policy

### Stage 4: CI gate

Only after local workflow is stable:
仅在本地工作流稳定后：

- run RCC in CI on the selected project subset
- fail the pipeline on exit code `1`

---

## 4. Make Integration Plan

### Case A: Project already has `compile_commands.json`

This is the easiest case.
这是最容易的情况。

Add a Make target like:

```make
RCC := ./build-linux/src/rcc-check
RCC_DB := build-linux
RCC_FILES := src/foo.cpp src/bar.cpp

.PHONY: rcc-check
rcc-check:
	$(RCC) -p $(RCC_DB) $(RCC_FILES)
```

Blocking gate:

```make
.PHONY: rcc-gate
rcc-gate: rcc-check
```

Then wire it into the desired path:

```make
all: rcc-gate
```

or

```make
check: rcc-gate
```

### Case B: Make project without `compile_commands.json`

Generate a compilation database first.
先生成编译数据库。

Practical options:
实践可选项：

- use `bear -- make`
- use a CMake shim for the target subset
- maintain a small RCC-only compile database for the checked files

Recommended short-term policy:
推荐的短期策略：

- do not block full Make builds until compile database generation is reliable
- first make `rcc-check` an explicit developer target

### Case C: Current-file or staged-file integration

Useful lightweight target:

```make
.PHONY: rcc-staged
rcc-staged:
	git diff --name-only --cached | grep -E '\\.(c|cc|cpp|cxx)$$' | xargs -r $(RCC) -p $(RCC_DB)
```

This is best for pre-commit or local validation, not for the first whole-project gate.
这更适合 pre-commit 或本地验证，不适合作为最初的全项目门禁。

---

## 5. Recommended Adoption Order

1. Linux build and tests stay green
2. Add VS Code task + problem matcher
3. Add Make target for explicit RCC runs
4. Apply RCC to a small real-project subset
5. Expand to CI gate on selected files
6. Revisit Windows support later

---

## 6. What To Avoid For Now

- claiming Windows is supported today
- forcing whole-tree adoption on legacy projects immediately
- building a VS Code extension before task-based workflow proves value
- depending on `_t.cc` / `@tcc` as if the current CLI already enforces them

---

## 7. Near-Term Deliverables

The minimum practical deliverables are:
近期最实用的交付物应为：

- a checked-in `.vscode/tasks.json` example later, if desired
- a reusable problem matcher snippet
- a `Makefile` integration recipe in docs
- a “real project onboarding” checklist based on Linux + compile database
