#!/usr/bin/env bash
# Build + run the minimal Dawn instance/adapter smoke.
# Confirms that vcpkg-installed Dawn (dawn[core,vulkan]) links against the
# system Vulkan stack and reports the local RTX 3090 as the adapter.
#
# Does NOT use gpu.cpp — gpu.cpp@0.2.0 has callback-info-struct ABI drift
# vs Dawn 2026-04 (see docs/reports/2026-05-12_gpu-cpp-dawn-abi-drift.md).
# This smoke talks to the C ABI directly.

set -euo pipefail

VCPKG_INSTALLED="${VCPKG_INSTALLED:-$HOME/vcpkg/installed/x64-linux}"

if [[ ! -f "${VCPKG_INSTALLED}/lib/libwebgpu_dawn.a" ]]; then
    echo "ERROR: libwebgpu_dawn.a not at ${VCPKG_INSTALLED}/lib/"
    exit 1
fi

export PKG_CONFIG_PATH="${VCPKG_INSTALLED}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

DAWN_CFLAGS=$(pkg-config --cflags unofficial_webgpu_dawn)
DAWN_LIBS=$(pkg-config --libs unofficial_webgpu_dawn)

g++ \
    -std=c++20 \
    -O2 \
    -Wall -Wextra -Wpedantic \
    ${DAWN_CFLAGS} \
    -o smoke_dawn_instance \
    smoke_dawn_instance.cpp \
    ${DAWN_LIBS} \
    -lvulkan -lpthread -ldl

echo
echo "=== running smoke_dawn_instance ==="
./smoke_dawn_instance
