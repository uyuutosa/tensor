cmake_minimum_required(VERSION 3.25)

project(tensor
    VERSION 0.2.0
    DESCRIPTION "Header-only C++20/23 named-axis differentiable tensor library aspiring to canonical-reference quality (ADR-0015)"
    HOMEPAGE_URL "https://github.com/uyuutosa/tensor"
    LANGUAGES CXX
)

# ─── Project options ──────────────────────────────────────────────────────────
option(TENSOR_BUILD_TESTS        "Build the test suite"        ON)
option(TENSOR_BUILD_BENCH        "Build the bench suite"       OFF)
option(TENSOR_WARNINGS_AS_ERRORS "Treat warnings as errors"    OFF)
# KernelBackend adapter selection (ADR-0011). `reference` is the always-
# available default. `eigen` enables the Eigen3 adapter (SIMD element-wise
# + BLAS-flavoured GEMM).
set(TENSOR_KERNEL_BACKEND "reference" CACHE STRING
    "Active KernelBackend adapter (reference | eigen | webgpu)")
set_property(CACHE TENSOR_KERNEL_BACKEND PROPERTY STRINGS reference eigen webgpu)

# ─── Header-only interface library ────────────────────────────────────────────
add_library(tensor INTERFACE)
add_library(tensor::tensor ALIAS tensor)

target_include_directories(tensor INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(tensor INTERFACE cxx_std_20)

# fmt is used until std::format is universally available across our compiler matrix.
find_package(fmt CONFIG REQUIRED)
target_link_libraries(tensor INTERFACE fmt::fmt)

# mdspan polyfill (Kokkos reference impl, default namespace
# Kokkos::Experimental::). Used by tensor/core/mdspan_interop.hpp; the
# adapter detects native std::mdspan via __cpp_lib_mdspan when available.
find_package(mdspan CONFIG REQUIRED)
target_link_libraries(tensor INTERFACE std::mdspan)

# Eigen3 is required when the active backend is `eigen` (ADR-0011 / P2.5.M3).
# The reference backend has no third-party deps, so non-eigen builds skip
# the find_package call entirely.
if(TENSOR_KERNEL_BACKEND STREQUAL "eigen")
    find_package(Eigen3 CONFIG REQUIRED)
    target_link_libraries(tensor INTERFACE Eigen3::Eigen)
    target_compile_definitions(tensor INTERFACE TENSOR_HAS_EIGEN=1)
endif()

# WebGPU adapter (ADR-0012, Phase 3 P3.M3.2). Per ADR-0016 (superseding
# ADR-0014 §Decision Outcome point 2) the project talks to Dawn directly
# via Dawn's own RAII C++ wrapper `<webgpu/webgpu_cpp.h>`; the legacy
# gpu.cpp vendoring is no longer load-bearing.
#
# Selecting -DTENSOR_KERNEL_BACKEND=webgpu pulls in Dawn via
# find_package(Dawn CONFIG REQUIRED) — the consumer is responsible for
# making the Dawn vcpkg port discoverable (typically via
# -DCMAKE_PREFIX_PATH=<vcpkg-install>/x64-linux). The project's default
# vcpkg.json builtin-baseline does NOT include `dawn`; consumers
# override the baseline locally per ADR-0014 / discussion-points Axis E.
#
# The four element-wise binary methods (add / sub / mul / div) and the
# four unary methods (exp / log / relu / neg) on
# tensor::core::backend::webgpu::Backend dispatch through Dawn on this
# code path. contract / broadcast_* / reduce_sum / unbroadcast continue
# to delegate to reference::Backend until P3.M4.2 / P3.M5 wire them.
if(TENSOR_KERNEL_BACKEND STREQUAL "webgpu")
    find_package(Dawn CONFIG REQUIRED)
    target_link_libraries(tensor INTERFACE dawn::webgpu_dawn)
    target_compile_definitions(tensor INTERFACE TENSOR_HAS_WEBGPU=1)
    message(STATUS "tensor: WebGPU backend selected — linked against dawn::webgpu_dawn (ADR-0016).")
endif()

# mdspan integration is temporarily disabled; the vcpkg-shipped Kokkos
# polyfill puts symbols in `Kokkos::` (not `std::` / `std::experimental::`),
# and the right namespace adapter is a Phase 1.5 follow-up. The header
# tensor/core/mdspan_interop.hpp is preserved for that follow-up but is
# excluded from the umbrella include and the test build.

# ─── Tests ────────────────────────────────────────────────────────────────────
if(TENSOR_BUILD_TESTS AND PROJECT_IS_TOP_LEVEL)
    enable_testing()
    add_subdirectory(tests)
endif()

if(TENSOR_BUILD_BENCH AND PROJECT_IS_TOP_LEVEL)
    add_subdirectory(bench)
endif()

# ─── Install rules ────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS tensor EXPORT tensorTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT tensorTargets
    FILE tensorTargets.cmake
    NAMESPACE tensor::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tensor
)
