add_executable(tensor_bench bench.cpp)

target_link_libraries(tensor_bench PRIVATE tensor::tensor)

# Bench is performance-sensitive: prefer Release-tuning even when the
# top-level preset is Debug. The user can override with -DCMAKE_BUILD_TYPE.
if(MSVC)
    target_compile_options(tensor_bench PRIVATE /W4)
else()
    target_compile_options(tensor_bench PRIVATE -Wall -Wextra -Wpedantic -O2)
endif()
