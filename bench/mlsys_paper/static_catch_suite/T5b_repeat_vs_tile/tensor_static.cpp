// T5b stack-vs-concat (variant b) — repeat vs tile.
//
// To repeat a (token, dim)-tensor along a new `n` axis the writer
// should produce (n, token, dim); the bug version tiles along the
// existing `token` axis instead, producing (n·token, dim) and
// breaking downstream rank assumptions.

#include <tensor/core/typed_tensor.hpp>

int main() {
    tensor::core::TypedTensor<double, "token", "dim"> base{
        {32, 64}, std::vector<double>(32*64, 0.0)};
    // Buggy: write the result as if rank-2 was kept (tile-along-token);
    // downstream expects a rank-3 (n, token, dim).
    tensor::core::TypedTensor<double, "token", "dim"> tiled{
        {64, 64}, std::vector<double>(64*64, 0.0)};   // 2·32 in `token`
    tensor::core::TypedTensor<double, "n", "token", "dim"> expected{
        {2, 32, 64}, std::vector<double>(2*32*64, 0.0)};
    auto bad = tiled + expected;   // CT: rank packs differ.
    (void)bad;
    return 0;
}
