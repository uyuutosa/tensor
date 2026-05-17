// T2b silent broadcast (variant b) — size-1 dim labelled wrong.
//
// A model expects a per-channel scaling factor of shape (channel,).
// The buggy upstream produces a (1, channel)-shaped tensor labelled
// (batch, channel) — the labels mismatch a true (channel,) tensor.

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "batch", "channel"> activations{
        {16, 64}, std::vector<double>(16*64, 0.0)};
    tensor::core::TypedTensor<double, "batch", "channel"> scale_wrong{
        {1, 64}, std::vector<double>(64, 0.0)};   // rank 2, intent was rank 1
    tensor::core::TypedTensor<double, "channel"> scale_correct{
        {64}, std::vector<double>(64, 0.0)};
    auto bad = scale_correct + scale_wrong;        // CT: rank packs differ.
    (void)bad;
    return 0;
}
