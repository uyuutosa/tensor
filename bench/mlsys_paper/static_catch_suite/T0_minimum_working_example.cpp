// Demonstrates a compile-time axis-mismatch refusal from the tensor library.
// Compile with:
//   g++ -std=c++20 -I/home/yu/proj/tensor/include /tmp/axis_mismatch_demo.cpp 2>&1 | head -25
#include <tensor/core/typed_tensor.hpp>
#include <tensor/core/label_tag.hpp>

using namespace tensor::core::literals;

int main() {
    // Two tensors with mismatched axis labels: one carries "i", the other "j".
    tensor::core::TypedTensor<double, "i"> a{{3}, {1.0, 2.0, 3.0}};
    tensor::core::TypedTensor<double, "j"> b{{3}, {4.0, 5.0, 6.0}};

    // This addition asks the compiler to align (i) with (j) — refused.
    auto c = a + b;
    return 0;
}
