cmake_minimum_required(VERSION 3.25)

project(tensor
    VERSION 0.0.1
    DESCRIPTION "Header-only C++20/23 educational library for named-axis tensor algebra"
    HOMEPAGE_URL "https://github.com/uyuutosa/tensor"
    LANGUAGES CXX
)

# ─── Project options ──────────────────────────────────────────────────────────
option(TENSOR_BUILD_TESTS    "Build the test suite"                ON)
option(TENSOR_WARNINGS_AS_ERRORS "Treat warnings as errors"        OFF)
option(TENSOR_USE_STD_MDSPAN "Use std::mdspan instead of polyfill" OFF)

# ─── Header-only interface library ────────────────────────────────────────────
add_library(tensor INTERFACE)
add_library(tensor::tensor ALIAS tensor)

target_include_directories(tensor INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(tensor INTERFACE cxx_std_20)

# ─── mdspan: std::mdspan if requested, otherwise the Kokkos polyfill ──────────
if(TENSOR_USE_STD_MDSPAN)
    target_compile_definitions(tensor INTERFACE TENSOR_USE_STD_MDSPAN=1)
else()
    find_package(mdspan CONFIG REQUIRED)
    target_link_libraries(tensor INTERFACE std::mdspan)
endif()

# fmt is used until std::format is universally available across our compiler matrix.
find_package(fmt CONFIG REQUIRED)
target_link_libraries(tensor INTERFACE fmt::fmt)

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
