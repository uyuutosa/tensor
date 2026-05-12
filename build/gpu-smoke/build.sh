#!/usr/bin/env bash
# Build + run the kAddF32 GPU smoke test on the local RTX 3090.
#
# Prerequisites:
#   ~/vcpkg/vcpkg install 'dawn[core,vulkan]' --triplet x64-linux
#
# Lib path (vcpkg-installed):
#   ~/vcpkg/installed/x64-linux/{include,lib}/
#
# Dawn is shipped as static libwebgpu_dawn.a + abseil deps. We resolve
# the link line via pkg-config (unofficial_webgpu_dawn).

set -euo pipefail

VCPKG_INSTALLED="${VCPKG_INSTALLED:-$HOME/vcpkg/installed/x64-linux}"
PROJECT_ROOT="${PROJECT_ROOT:-$HOME/proj/tensor}"

if [[ ! -f "${VCPKG_INSTALLED}/lib/libwebgpu_dawn.a" ]]; then
    echo "ERROR: libwebgpu_dawn.a not at ${VCPKG_INSTALLED}/lib/"
    echo "Run:   ~/vcpkg/vcpkg install 'dawn[core,vulkan]' --triplet x64-linux"
    exit 1
fi

export PKG_CONFIG_PATH="${VCPKG_INSTALLED}/lib/pkgconfig:${PKG_CONFIG_PATH:-}"

DAWN_CFLAGS=$(pkg-config --cflags unofficial_webgpu_dawn)
DAWN_LIBS=$(pkg-config --libs unofficial_webgpu_dawn)

echo "Dawn cflags: ${DAWN_CFLAGS}"
echo "Dawn libs:   ${DAWN_LIBS}"

# -lvulkan = system Vulkan loader; pthread / dl are pulled in by Dawn
# and absl. -lX11 -lXrandr -lXinerama -lXcursor -lXi -lXxf86vm are not
# needed because we built dawn[core,vulkan] without X11 / GL.
g++ \
    -std=c++20 \
    -O2 \
    -Wall -Wextra -Wpedantic \
    -I"${PROJECT_ROOT}/third_party/gpu_cpp" \
    -I"${PROJECT_ROOT}/include" \
    ${DAWN_CFLAGS} \
    -o smoke_add \
    smoke_add.cpp \
    ${DAWN_LIBS} \
    -lvulkan -lpthread -ldl

echo
echo "=== running smoke_add ==="
./smoke_add
