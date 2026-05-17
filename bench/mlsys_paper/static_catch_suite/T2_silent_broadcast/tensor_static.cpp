// T2 silent broadcast — tensor (static path) translation.
//
// The bug scenario: a (32, 128) feature batch is to be biased by a
// per-feature bias vector. Upstream the bias was transposed and is now
// shaped (128, 1) labelled (feature, _) rather than the intended
// (feature,) labelled (feature,). Positional broadcasting would
// happily stretch (128, 1) against (32, 128) along the wrong axis;
// the typed path refuses because the rank packs differ.

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "batch", "feature">
        features{{32, 128}, std::vector<double>(32*128, 0.0)};
    tensor::core::TypedTensor<double, "feature", "broadcast_dim">
        bias_transposed{{128, 1}, std::vector<double>(128, 0.0)};

    auto biased = features + bias_transposed;   // CT: pack rank differs.
    (void)biased;
    return 0;
}
