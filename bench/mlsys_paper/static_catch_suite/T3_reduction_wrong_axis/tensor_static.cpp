// T3 reduction over wrong axis — tensor (static path) translation.
//
// The bug scenario: logits are (batch, classes); cross-entropy expects
// the sum to reduce over `classes`. The buggy code reduces over
// `batch` instead. Under positional shape systems both reductions
// produce a same-rank tensor that further math is happy with.
//
// In the typed path, reducing into a tensor whose label pack mismatches
// the expected post-reduction labels is refused at instantiation.

#include <tensor/core/typed_tensor.hpp>

int main() {
    // Logits as (batch, classes); programmer intends to reduce -> (batch,).
    tensor::core::TypedTensor<double, "batch", "classes"> logits{
        {16, 10}, std::vector<double>(160, 0.0)};

    // Buggy aggregation labels the result (classes,) — wrong axis was
    // collapsed. Asking the typed system to add this to a true
    // (batch,)-labelled tensor is refused.
    tensor::core::TypedTensor<double, "classes"> wrong_axis_sum{
        {10}, std::vector<double>(10, 0.0)};
    tensor::core::TypedTensor<double, "batch"> labels{
        {16}, std::vector<double>(16, 0.0)};

    auto loss = wrong_axis_sum + labels;   // CT: classes != batch.
    (void)loss;
    return 0;
}
