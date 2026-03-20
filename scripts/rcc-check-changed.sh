#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  scripts/rcc-check-changed.sh [--staged] [--tcc-only]

Options:
  --staged    Check staged files instead of working tree diff.
  --tcc-only  Only analyze files ending with _t.cc or containing @tcc in first 100 lines.
EOF
}

mode="working"
tcc_only=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --staged)
      mode="staged"
      shift
      ;;
    --tcc-only)
      tcc_only=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 4
      ;;
  esac
done

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

if [[ "$mode" == "staged" ]]; then
  mapfile -t changed_files < <(git diff --cached --name-only --diff-filter=ACMRT)
else
  mapfile -t changed_files < <(git diff --name-only --diff-filter=ACMRT)
fi

source_files=()
for file in "${changed_files[@]}"; do
  if [[ ! -f "$file" ]]; then
    continue
  fi
  case "$file" in
    *.c|*.cc|*.cpp|*.cxx)
      source_files+=("$file")
      ;;
  esac
done

if [[ ${#source_files[@]} -eq 0 ]]; then
  echo "No changed C/C++ source files to check."
  exit 0
fi

if [[ $tcc_only -eq 1 ]]; then
  filtered=()
  for file in "${source_files[@]}"; do
    if [[ "$file" == *_t.cc ]]; then
      filtered+=("$file")
      continue
    fi
    if head -n 100 "$file" | grep -q "@tcc"; then
      filtered+=("$file")
    fi
  done
  source_files=("${filtered[@]}")
fi

if [[ ${#source_files[@]} -eq 0 ]]; then
  echo "No files matched selected filters."
  exit 0
fi

failed=0
for file in "${source_files[@]}"; do
  echo "==> RCC checking $file"
  if ! "$repo_root/scripts/rcc-check-file.sh" "$repo_root/$file"; then
    failed=1
  fi
done

if [[ $failed -ne 0 ]]; then
  echo "One or more files failed RCC checks."
  exit 1
fi

echo "All selected files passed RCC checks."
