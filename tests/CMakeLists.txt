find_package(doctest CONFIG REQUIRED)

add_executable(tensor_tests
    test_version.cpp
    test_axis_shape.cpp
    test_tensor.cpp
    test_format.cpp
    test_mdspan_interop.cpp
)

target_link_libraries(tensor_tests
    PRIVATE
        tensor::tensor
        doctest::doctest
)

# Standard portable warning set; project-wide CMake helper would normalize
# these per compiler, but a single tests/ target is small enough to inline.
if(MSVC)
    target_compile_options(tensor_tests PRIVATE /W4)
    if(TENSOR_WARNINGS_AS_ERRORS)
        target_compile_options(tensor_tests PRIVATE /WX)
    endif()
else()
    target_compile_options(tensor_tests PRIVATE -Wall -Wextra -Wpedantic)
    if(TENSOR_WARNINGS_AS_ERRORS)
        target_compile_options(tensor_tests PRIVATE -Werror)
    endif()
endif()

include(doctest)
doctest_discover_tests(tensor_tests)
