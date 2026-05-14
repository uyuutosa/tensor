# P6.M1 — CMake build for the Python extension module.
# P6.5.M1 — Parameterised by TENSOR_KERNEL_BACKEND env var (per
# ADR-0019). Defaults to "reference" so the existing
# `_tensor_native.<abi>.so` output stays backward-compatible. When
# TENSOR_KERNEL_BACKEND is set to {eigen,webgpu}, the extension
# module name becomes `_tensor_native_<backend>` and the matching
# C++ side links against the corresponding KernelBackend adapter.
#
# Invoked by scikit-build-core via the repo-root pyproject.toml; not
# used directly by maintainers. The repo's main CMakeLists (one level
# up) builds the C++ library / tests / bench / notebook tooling; this
# file is solely the nanobind extension's build script.

cmake_minimum_required(VERSION 3.25)
project(tensor_python_sdk
    DESCRIPTION "Python bindings for tensor named-axis library (Phase 6 + 6.5)"
    LANGUAGES CXX
)

# Match the C++ Domain baseline (ADR-0002).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# scikit-build-core invokes us; Python_EXECUTABLE / interpreter / etc.
# are already populated. find_package keeps the local-build path
# usable too.
find_package(Python 3.9
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

# nanobind discovers itself from its pip install via its CMake config.
find_package(nanobind CONFIG REQUIRED)

# ─── Backend selection (Phase 6.5 / ADR-0019) ────────────────────
# Read TENSOR_KERNEL_BACKEND from the cache or env var. Default is
# "reference" so legacy `pip install -e .` flows produce the same
# `_tensor_native` module name as before P6.5.
if(NOT DEFINED TENSOR_KERNEL_BACKEND)
    if(DEFINED ENV{TENSOR_KERNEL_BACKEND})
        set(TENSOR_KERNEL_BACKEND "$ENV{TENSOR_KERNEL_BACKEND}")
    else()
        set(TENSOR_KERNEL_BACKEND "reference")
    endif()
endif()

set(_VALID_BACKENDS reference eigen webgpu)
if(NOT TENSOR_KERNEL_BACKEND IN_LIST _VALID_BACKENDS)
    message(FATAL_ERROR
        "TENSOR_KERNEL_BACKEND must be one of: ${_VALID_BACKENDS}. "
        "Got: ${TENSOR_KERNEL_BACKEND}")
endif()

message(STATUS "Python SDK: TENSOR_KERNEL_BACKEND=${TENSOR_KERNEL_BACKEND}")

# Module name:
#   - reference build → `_tensor_native` (backward compatible with
#     the existing `from ._tensor_native import …` in tensor/__init__.py).
#   - eigen / webgpu  → `_tensor_native_<backend>` so the three
#     companion wheels can coexist under the same `tensor/` package
#     (PEP-420 namespace package — see ADR-0019).
if(TENSOR_KERNEL_BACKEND STREQUAL "reference")
    set(_MODULE_NAME _tensor_native)
else()
    set(_MODULE_NAME _tensor_native_${TENSOR_KERNEL_BACKEND})
endif()

# The C++ Domain headers live one level up at ../include.
get_filename_component(TENSOR_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../include"
    ABSOLUTE
)

nanobind_add_module(
    ${_MODULE_NAME}
    STABLE_ABI
    NB_STATIC
    src/_tensor_native.cpp
)

target_include_directories(${_MODULE_NAME}
    PRIVATE
    "${TENSOR_INCLUDE_DIR}"
)

target_compile_features(${_MODULE_NAME} PRIVATE cxx_std_20)

# Pass the backend name + the nanobind module name into the C++ source.
# - TENSOR_KERNEL_BACKEND_NAME: exported as `<module>.__backend__` for
#   the Python adapter's `tensor.set_backend()` / `current_backend()` /
#   `list_available_backends()` logic.
# - TENSOR_NB_MODULE_NAME: the literal identifier nanobind's NB_MODULE
#   macro stringifies. Must be a token (not a quoted string), so we
#   pass it directly into the preprocessor.
target_compile_definitions(${_MODULE_NAME}
    PRIVATE
    TENSOR_KERNEL_BACKEND_NAME="${TENSOR_KERNEL_BACKEND}"
    TENSOR_NB_MODULE_NAME=${_MODULE_NAME}
)

# Backend-specific link deps. The eigen / webgpu adapters live in
# the C++ Domain (`include/tensor/core/backend/*/`); the Python SDK
# consumes them through the same KernelBackend port the C++
# tutorials exercise. M1 keeps this minimal — eigen / webgpu adapter
# selection is a no-op at the binding level because the Python
# surface (DynamicTensor + ops) currently routes through the
# Domain's runtime-rank path which is backend-agnostic. M2 wires
# `set_backend()` to *choose* which `.so` provides the surface;
# M3 onward will route the underlying contract / broadcast / etc.
# calls through the matching adapter.
if(TENSOR_KERNEL_BACKEND STREQUAL "eigen")
    find_package(Eigen3 CONFIG REQUIRED)
    target_link_libraries(${_MODULE_NAME} PRIVATE Eigen3::Eigen)
    target_compile_definitions(${_MODULE_NAME} PRIVATE TENSOR_HAS_EIGEN=1)
elseif(TENSOR_KERNEL_BACKEND STREQUAL "webgpu")
    find_package(Dawn CONFIG REQUIRED)
    target_link_libraries(${_MODULE_NAME} PRIVATE dawn::webgpu_dawn)
    target_compile_definitions(${_MODULE_NAME} PRIVATE TENSOR_HAS_WEBGPU=1)
endif()

# Install destination matches wheel.packages = ["python/tensor"] in
# pyproject.toml so the .so lands at tensor/_tensor_native[_*]?.<abi>.so.
install(TARGETS ${_MODULE_NAME} LIBRARY DESTINATION tensor)
