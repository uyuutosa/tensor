# P6.M1 — CMake build for the Python extension module (`_tensor_native`).
# Invoked by scikit-build-core via the repo-root pyproject.toml; not used
# directly by maintainers. The repo's main CMakeLists (one level up) builds
# the C++ library / tests / bench / notebook tooling; this file is solely
# the nanobind extension's build script.

cmake_minimum_required(VERSION 3.25)
project(tensor_python_sdk
    DESCRIPTION "Python bindings (P6.M1 scaffold) for tensor named-axis library"
    LANGUAGES CXX
)

# Match the C++ Domain baseline (ADR-0002).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# scikit-build-core invokes us; Python_EXECUTABLE / interpreter / etc. are
# already populated. find_package keeps the local-build path usable too.
find_package(Python 3.9
    REQUIRED COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

# nanobind discovers itself from its pip install via its CMake config.
# The pyproject.toml declares `nanobind>=2.0,<3` as a build dep so this
# always resolves when scikit-build-core drives the build.
find_package(nanobind CONFIG REQUIRED)

# The C++ Domain headers live one level up at ../include.
get_filename_component(TENSOR_INCLUDE_DIR
    "${CMAKE_CURRENT_LIST_DIR}/../include"
    ABSOLUTE
)

nanobind_add_module(
    _tensor_native
    STABLE_ABI
    NB_STATIC
    src/_tensor_native.cpp
)

target_include_directories(_tensor_native
    PRIVATE
    "${TENSOR_INCLUDE_DIR}"
)

target_compile_features(_tensor_native PRIVATE cxx_std_20)

# Install destination matches wheel.packages = ["python/tensor"] in
# pyproject.toml so the .so lands at tensor/_tensor_native.<abi>.so.
install(TARGETS _tensor_native LIBRARY DESTINATION tensor)
