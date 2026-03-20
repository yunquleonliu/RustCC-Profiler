#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage:
  scripts/rcc-check-file.sh <source-file> [-- <extra-clang-args>]

Environment overrides:
  RCC_BUILD_DIR   Build directory (default: <repo>/build-linux, fallback <repo>/build)
  RCC_BIN         Path to rcc-check binary (default: <build-dir>/src/rcc-check)
EOF
}

if [[ $# -lt 1 ]]; then
  usage
  exit 4
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source_file="$1"
shift

if [[ "$source_file" != /* ]]; then
  source_file="$(cd "$repo_root" && realpath "$source_file")"
fi

if [[ ! -f "$source_file" ]]; then
  echo "Error: Source file not found: $source_file" >&2
  exit 5
fi

build_dir="${RCC_BUILD_DIR:-$repo_root/build-linux}"
if [[ ! -d "$build_dir" ]]; then
  build_dir="$repo_root/build"
fi

if [[ ! -d "$build_dir" ]]; then
  echo "Error: Build directory not found. Set RCC_BUILD_DIR or create build-linux/build." >&2
  exit 4
fi

rcc_bin="${RCC_BIN:-$build_dir/src/rcc-check}"
if [[ ! -x "$rcc_bin" ]]; then
  echo "Error: rcc-check binary not found or not executable: $rcc_bin" >&2
  exit 4
fi

compile_db="$build_dir/compile_commands.json"
use_fallback=1
if [[ -f "$compile_db" ]] && grep -Fq "\"$source_file\"" "$compile_db"; then
  use_fallback=0
fi

if [[ $use_fallback -eq 1 ]]; then
  echo "Info: source file not present in compile_commands.json, enabling fallback stdcpp includes." >&2
  exec "$rcc_bin" --auto-stdcpp-includes -p "$build_dir" "$source_file" "$@"
fi

exec "$rcc_bin" -p "$build_dir" "$source_file" "$@"
