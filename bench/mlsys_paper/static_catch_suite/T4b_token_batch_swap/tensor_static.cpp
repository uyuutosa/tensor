// T4b layout swap (variant b) — token-major vs batch-major.
//
// Sequence model: one module returns (token, batch, dim) (RNN
// convention); next module expects (batch, token, dim) (Transformer
// convention). Without explicit permutation the rank-3 buffer would
// reinterpret silently under positional shape systems.

#include <tensor/core/typed_tensor.hpp>

void transformer_block(
    tensor::core::TypedTensor<double, "batch", "token", "dim"> const&);

int main() {
    tensor::core::TypedTensor<double, "token", "batch", "dim"> rnn_out{
        {128, 32, 256}, std::vector<double>(128*32*256, 0.0)};
    transformer_block(rnn_out);   // CT: label order does not match.
    return 0;
}
