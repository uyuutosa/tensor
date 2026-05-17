// T3b reduction over wrong axis (variant b) — argmax misrouting.
//
// Classifier head: (batch, classes) → argmax over `classes` → (batch,).
// Buggy aggregation produces (classes,) — different label pack from
// what the downstream cross-entropy expects.

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "batch"> targets{
        {32}, std::vector<double>(32, 0.0)};
    // The buggy argmax labelled its output (classes,) instead of (batch,).
    tensor::core::TypedTensor<double, "classes"> argmax_wrong{
        {10}, std::vector<double>(10, 0.0)};
    auto comparison = targets + argmax_wrong;      // CT: batch != classes.
    (void)comparison;
    return 0;
}
