#!/usr/bin/env bash
# Build + run the kAddF32 GPU smoke test on the local RTX 3090.
#
# Prerequisites:
#   ~/vcpkg/vcpkg install dawn  (in classic mode; ~30-60 min build)
#
# Lib path (auto-discovered after install):
#   ~/vcpkg/installed/x64-linux/{include,lib}/

set -euo pipefail

VCPKG_INSTALLED="${VCPKG_INSTALLED:-$HOME/vcpkg/installed/x64-linux}"
PROJECT_ROOT="${PROJECT_ROOT:-$HOME/proj/tensor}"

if [[ ! -d "${VCPKG_INSTALLED}/include/webgpu" ]]; then
    echo "ERROR: vcpkg-installed webgpu headers not found at ${VCPKG_INSTALLED}/include/webgpu/"
    echo "Run:   ~/vcpkg/vcpkg install dawn   first."
    exit 1
fi

# Discover Dawn shared-library names produced by the port.
shopt -s nullglob
DAWN_LIBS=(${VCPKG_INSTALLED}/lib/libwebgpu*.so ${VCPKG_INSTALLED}/lib/libdawn*.so)
shopt -u nullglob
if [[ ${#DAWN_LIBS[@]} -eq 0 ]]; then
    echo "ERROR: no libwebgpu*/libdawn* under ${VCPKG_INSTALLED}/lib/"
    ls "${VCPKG_INSTALLED}/lib/" 2>&1 | head -20
    exit 1
fi
echo "Dawn libs found:"
for L in "${DAWN_LIBS[@]}"; do echo "  ${L}"; done

# Compile + link.
g++ \
    -std=c++20 \
    -O2 \
    -Wall -Wextra -Wpedantic \
    -I"${PROJECT_ROOT}/third_party/gpu_cpp" \
    -I"${PROJECT_ROOT}/include" \
    -I"${VCPKG_INSTALLED}/include" \
    -Wl,-rpath,"${VCPKG_INSTALLED}/lib" \
    -o smoke_add \
    smoke_add.cpp \
    "${DAWN_LIBS[@]}"

echo
echo "=== running smoke_add ==="
./smoke_add
