#!/usr/bin/env bash
# tools/check-vendored.sh — fails CI if any subdirectory of third_party/
# lacks a VENDORED_FROM file. Enforces ADR-0014 §Compliance.
#
# Usage: tools/check-vendored.sh [--quiet]
#
# Exit code: 0 on success, 1 on missing-vendor-record.

set -euo pipefail

quiet=0
if [[ "${1:-}" == "--quiet" ]]; then
    quiet=1
fi

repo_root="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
third_party_dir="${repo_root}/third_party"

if [[ ! -d "${third_party_dir}" ]]; then
    [[ "${quiet}" -eq 0 ]] && echo "tools/check-vendored.sh: no third_party/ directory; nothing to check."
    exit 0
fi

missing=()
checked=0
for dir in "${third_party_dir}"/*/; do
    [[ -d "${dir}" ]] || continue
    checked=$((checked + 1))
    if [[ ! -f "${dir}/VENDORED_FROM" ]]; then
        missing+=("${dir}")
    fi
done

if (( ${#missing[@]} > 0 )); then
    echo "tools/check-vendored.sh: ${#missing[@]} vendored director(y/ies) missing VENDORED_FROM:"
    for d in "${missing[@]}"; do
        echo "  - ${d}"
    done
    echo
    echo "Each subdirectory of third_party/ must contain a VENDORED_FROM file"
    echo "naming the upstream repo, commit, license, and re-vendor procedure."
    echo "See third_party/gpu_cpp/VENDORED_FROM for a working example."
    exit 1
fi

[[ "${quiet}" -eq 0 ]] && echo "tools/check-vendored.sh: all ${checked} vendored director(y/ies) have VENDORED_FROM. OK."
exit 0
