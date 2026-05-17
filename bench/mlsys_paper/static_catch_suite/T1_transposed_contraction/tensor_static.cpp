// T1 transposed contraction — tensor (static path) translation.
//
// Build the multi-head attention scores under the same bug premise as the
// einops translation: Q was laid out (batch, time, head, dim) upstream but
// the contraction expects (batch, head, time, dim).
//
// In the static-axis path, Q's TypedTensor type carries the axis labels:
//
//     TypedTensor<double, "batch", "time", "head", "dim"> Q { ... };
//
// The contraction the programmer wrote expects an operand whose axis
// labels are (batch, head, time, dim). The library's contract operator
// (also templated on the contracted-axes pack) refuses the call at
// template instantiation time: the static_assert message reads
// "axis labels must match at compile time."
//
// Expected outcome: CT.

#include <tensor/core/typed_tensor.hpp>
#include <tensor/core/label_tag.hpp>
#include <tensor/tex/tex.hpp>

using namespace tensor::core::literals;

int main() {
    tensor::core::TypedTensor<double, "batch", "time", "head", "dim"> Q{
        {2, 8, 4, 16}, std::vector<double>(2*8*4*16, 0.0)
    };
    tensor::core::TypedTensor<double, "batch", "head", "time", "dim"> K{
        {2, 4, 8, 16}, std::vector<double>(2*4*8*16, 0.0)
    };

    // The contraction below expects Q and K to share the (batch, head, dim)
    // axes; Q's pack is (batch, time, head, dim) — refused at instantiation.
    auto scores = tensor::core::contract<"dim">(Q, K);  // static_assert fires here
    (void)scores;
    return 0;
}
