#!/usr/bin/env bash
# rcc-ci-check.sh — CI gate: enforce RCC rules on all _t.cc files
#
# Usage:
#   scripts/rcc-ci-check.sh <target-dir> [--build-dir <rcc-build-dir>]
#
# Exits 0 only when every _t.cc / @tcc-annotated file passes rcc-check.
# Plain .cpp files are skipped (advisory; do not fail CI).
#
# This script is the recommended way to add RCC to any CI pipeline:
#
#   # GitHub Actions example
#   - run: |
#       ./scripts/rcc-ci-check.sh src/ --build-dir build-linux
#
# Environment:
#   RCC_BUILD_DIR   Alternative path to the build directory (fallback).
#   RCC_BIN         Explicit path to the rcc-check binary.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 <target-dir> [--build-dir <dir>]" >&2
  exit 4
fi

target_dir="$1"
shift

# Forward remaining args to rcc-check-dir.sh
exec "$script_dir/rcc-check-dir.sh" "$target_dir" --tcc-only "$@"
