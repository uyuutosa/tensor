// T4 layout swap — tensor (static path) translation.
//
// The bug scenario: a module that documents its output as (head, batch,
// dim) is consumed by downstream code that expects (batch, head, dim)
// without an explicit permutation. Under positional types `.view()`
// would silently reinterpret the buffer with the wrong semantics.
//
// In the typed path, the upstream tensor's labels are part of its type;
// passing it to a downstream that expects a different label order is
// refused at the call site.

#include <tensor/core/typed_tensor.hpp>

void downstream_expects_BHD(
    tensor::core::TypedTensor<double, "batch", "head", "dim"> const&);

int main() {
    tensor::core::TypedTensor<double, "head", "batch", "dim"> upstream{
        {8, 32, 64}, std::vector<double>(8*32*64, 0.0)};

    downstream_expects_BHD(upstream);  // CT: label order does not match.
    return 0;
}
