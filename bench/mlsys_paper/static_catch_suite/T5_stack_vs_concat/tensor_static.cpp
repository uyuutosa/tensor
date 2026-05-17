// T5 stack-vs-concat confusion — tensor (static path) translation.
//
// The bug scenario: the programmer means to add a new length-2 axis
// (stack) but calls concat() along an existing axis instead. Under
// positional shapes the result has rank r instead of r+1, and the
// downstream .view() either misaligns or crashes operators later.
//
// The typed path: appending two (token, dim)-labelled tensors element-
// wise via operator+ when the intent is "stack into (seq, token, dim)"
// is refused because the result of operator+ is still rank-2 (the
// SameLabels constraint refuses adding (seq, token, dim) and
// (token, dim) directly).

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "token", "dim"> token0{
        {16, 64}, std::vector<double>(16*64, 0.0)};
    tensor::core::TypedTensor<double, "token", "dim"> token1{
        {16, 64}, std::vector<double>(16*64, 0.0)};

    // Wanted: stack into (stack_pos, token, dim). Wrote: element-wise
    // append, then try to add it to the (stack_pos, token, dim) shape
    // a downstream operator needs.
    auto bug = token0 + token1;
    tensor::core::TypedTensor<double, "stack_pos", "token", "dim"> expected{
        {2, 16, 64}, std::vector<double>(2*16*64, 0.0)};

    auto result = bug + expected;  // CT: rank packs differ.
    (void)result;
    return 0;
}
