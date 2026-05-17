// T1b transposed contraction (variant b) — matmul on flipped axes.
//
// Two matrices: A: (i, k), B: (k, j). Intended product: A·B over k.
// Bug: writer accidentally builds B with axes (j, k); the contraction
// over the common axis fails at the typed level because the operator
// signature requires identical label packs across operands.

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "i", "k"> A{
        {3, 4}, std::vector<double>(12, 0.0)};
    tensor::core::TypedTensor<double, "j", "k"> B_flipped{
        {5, 4}, std::vector<double>(20, 0.0)};
    auto out = A + B_flipped;       // CT: label pack differs (i,k) vs (j,k).
    (void)out;
    return 0;
}
