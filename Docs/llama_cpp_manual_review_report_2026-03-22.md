# Llama.cpp RCC Manual Review Report (2026-03-22)

## Scope

This report captures reproducible RCC findings on local `../llama.cpp` and separates likely true risks from likely false positives for manual audit.

Data provenance:
- Primary diagnostics are direct outputs from the RCC binary (`rcc-check` on Linux, equivalent to `rcc-check.exe` on Windows).
- This report is a Copilot consolidation layer on top of raw RCC output (classification, confidence level, and manual notes).

Sample-scanned files:
- `../llama.cpp/src/llama.cpp`
- `../llama.cpp/tests/test-chat.cpp`

Additional full-coverage pass (completed for `src/`):
- `../llama.cpp/src` C/C++ files scanned: 26/26
- Pass: 9
- Non-pass (diagnostics emitted): 17

## Reproduction Commands

```bash
# 1) Scan core source file
./build-linux/src/rcc-check --auto-stdcpp-includes -p ../llama.cpp/build ../llama.cpp/src/llama.cpp

# 2) Scan representative test file
./build-linux/src/rcc-check --auto-stdcpp-includes -p ../llama.cpp/build ../llama.cpp/tests/test-chat.cpp

# 3) Full src pass (all C/C++ files)
for f in $(find ../llama.cpp/src -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \) | sort); do
  ./build-linux/src/rcc-check --auto-stdcpp-includes -p ../llama.cpp/build "$f"
done
```

## Raw Rule Summary

For `src/llama.cpp`:
- TCC-OWN-001: 1
- TCC-LIFE-002: 1
- TCC-LIFE-003: 2
- TCC-CONC-001: 1
- TCC-OPTION-002: 5
- TCC-SAFE-001: 4

For `tests/test-chat.cpp`:
- TCC-LIFE-001: 3
- TCC-CONC-001: 5
- TCC-RESULT-002: 6
- TCC-SAFE-001: multiple notes/warnings

For full `src/` pass (26 files aggregated):
- TCC-SAFE-001: 2685
- TCC-OPTION-002: 187
- TCC-LIFE-002: 75
- TCC-OWN-001: 42
- TCC-CONC-002: 39
- TCC-OWN-002: 25
- TCC-CONC-001: 25
- TCC-LIFE-003: 19
- TCC-LIFE-004: 16
- TCC-OWN-003: 14

Note: aggregated counts include low-signal rules (especially `TCC-SAFE-001`), so publication should still use manual triage and confidence buckets.

## Manual Triage (Line-Level)

### A) High-confidence risk candidate

1. File: `../llama.cpp/src/llama.cpp:337`
- Rule: `TCC-CONC-001`
- Code:
```cpp
336  const char * llama_print_system_info(void) {
337      static std::string s;
338      s.clear();
```
- Review note: shared mutable static object in function scope can race under concurrent calls.
- Confidence: High

### B) Medium-confidence candidates (context-dependent)

1. File: `../llama.cpp/tests/test-chat.cpp:171`
- Rule: `TCC-CONC-001`
- Code:
```cpp
171  common_chat_tool special_function_tool {
```
- Review note: mutable global/static test fixtures may race in parallel test runs, but severity depends on test execution model.
- Confidence: Medium

2. File: `../llama.cpp/tests/test-chat.cpp:213`
- Rule: `TCC-CONC-001`
- Code:
```cpp
213  std::vector<common_chat_tool> tools           { special_function_tool, python_tool };
214  std::vector<common_chat_tool> llama_3_1_tools { special_function_tool, code_interpreter_tool };
```
- Review note: global mutable containers are valid concurrency smells; impact depends on test threading.
- Confidence: Medium

### C) Likely false positives

1. File: `../llama.cpp/tests/test-chat.cpp:33`, `:42`, `:59`
- Rule: `TCC-LIFE-001`
- Code:
```cpp
33  return os;
42  return os;
59  return os;
```
- Review note: these return the input stream reference parameter, not a reference to a local variable.
- Confidence: Likely false positive

2. File: `../llama.cpp/src/llama.cpp:234`
- Rule: `TCC-LIFE-002`
- Code:
```cpp
165  llama_model * model = new llama_model(params);
...
234  return model;
```
- Review note: returning heap pointer is not a dangling local pointer by itself.
- Confidence: Likely false positive

## Reviewer Checklist

Use this to manually verify each finding:
1. Confirm exact line and surrounding function semantics.
2. Check object lifetime origin (stack vs heap vs static).
3. Check thread model (single-thread test vs parallel runtime).
4. Re-run with same command and confirm deterministic hit.
5. Classify as High / Medium / Low confidence before external publication.

## Publication Guidance

- Publish only High and Medium confidence findings in primary report.
- Keep likely false positives in an appendix with rationale.
- Include exact command, file, line, and review note for every claim.
