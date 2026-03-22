#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  scripts/rcc-check-dir.sh <target-path> [--tcc-only] [--build-dir <dir>]

Options:
  --tcc-only        Only analyze files ending with _t.cc or containing @tcc in first 100 lines.
  --build-dir <dir> Build directory that contains src/rcc-check and compile_commands.json.

Environment overrides:
  RCC_BUILD_DIR     Build directory fallback if --build-dir is not provided.
  RCC_BIN           Optional explicit path to rcc-check binary.
EOF
}

if [[ $# -lt 1 ]]; then
  usage
  exit 4
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

target_path=""
tcc_only=0
build_dir_override=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --tcc-only)
      tcc_only=1
      shift
      ;;
    --build-dir)
      if [[ $# -lt 2 ]]; then
        echo "Error: --build-dir requires a value." >&2
        exit 4
      fi
      build_dir_override="$2"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --*)
      echo "Unknown option: $1" >&2
      usage
      exit 4
      ;;
    *)
      if [[ -n "$target_path" ]]; then
        echo "Error: multiple target paths provided." >&2
        usage
        exit 4
      fi
      target_path="$1"
      shift
      ;;
  esac
done

if [[ -z "$target_path" ]]; then
  echo "Error: target path is required." >&2
  usage
  exit 4
fi

if [[ "$target_path" != /* ]]; then
  target_path="$(cd "$repo_root" && realpath "$target_path")"
fi

if [[ ! -d "$target_path" ]]; then
  echo "Error: target directory not found: $target_path" >&2
  exit 5
fi

if [[ -n "$build_dir_override" ]]; then
  export RCC_BUILD_DIR="$build_dir_override"
fi

is_likely_binary() {
  local file="$1"
  local magic

  if [[ ! -s "$file" ]]; then
    return 1
  fi

  # Skip PE binaries accidentally named as source.
  magic="$(LC_ALL=C od -An -N2 -t x1 "$file" | tr -d ' \n')"
  if [[ "$magic" == "4d5a" ]]; then
    return 0
  fi

  return 1
}

collect_sources() {
  local root="$1"

  find "$root" \
    -type d \( -name .git -o -name build -o -name build-linux -o -name out -o -name dist -o -name bin -o -name node_modules -o -name .cache -o -name target \) -prune -o \
    -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \) -print
}

mapfile -t files < <(collect_sources "$target_path")

if [[ ${#files[@]} -eq 0 ]]; then
  echo "No C/C++ source files found under $target_path after filters."
  exit 0
fi

checked=0
failed=0
skipped_binary=0
skipped_filter=0

for file in "${files[@]}"; do
  if [[ $tcc_only -eq 1 ]]; then
    if [[ "$file" != *_t.cc ]] && ! head -n 100 "$file" | grep -q "@tcc"; then
      skipped_filter=$((skipped_filter + 1))
      continue
    fi
  fi

  if is_likely_binary "$file"; then
    echo "SKIP(binary): $file"
    skipped_binary=$((skipped_binary + 1))
    continue
  fi

  echo "RCC checking: $file"
  if ! "$repo_root/scripts/rcc-check-file.sh" "$file"; then
    failed=$((failed + 1))
  fi
  checked=$((checked + 1))
done

echo
echo "Summary: checked=$checked failed=$failed skipped_binary=$skipped_binary skipped_filter=$skipped_filter"

if [[ $failed -ne 0 ]]; then
  exit 1
fi

exit 0
