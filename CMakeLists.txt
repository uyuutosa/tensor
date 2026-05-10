cmake_minimum_required(VERSION 3.25)

project(tensor
    VERSION 0.0.1
    DESCRIPTION "Header-only C++20/23 educational library for named-axis tensor algebra"
    HOMEPAGE_URL "https://github.com/uyuutosa/tensor"
    LANGUAGES CXX
)

# ─── Project options ──────────────────────────────────────────────────────────
option(TENSOR_BUILD_TESTS        "Build the test suite"        ON)
option(TENSOR_WARNINGS_AS_ERRORS "Treat warnings as errors"    OFF)

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

# ─── Install rules ────────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS tensor EXPORT tensorTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT tensorTargets
    FILE tensorTargets.cmake
    NAMESPACE tensor::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tensor
)
