// SPDX-License-Identifier: MIT
//
// tensor::core::mdspan_interop — bidirectional bridge to std::mdspan
// (or the Kokkos polyfill when std::mdspan is unavailable).
//
// Per ADR-0002 + ADR-0007, mdspan is the lingua franca for cross-library
// data exchange. Tensor<T, N> exposes an mdview() returning a non-owning
// std::mdspan<T, dextents<N>> view; from_mdspan() rebuilds an owning
// Tensor by copy (for now — a non-owning wrapper would require a future
// `TensorView` type).

#pragma once

#include <cstddef>
#include <utility>
#include <vector>

// Toolchain selector. Three cases:
//   1. TENSOR_USE_STD_MDSPAN — user explicitly requested C++23 std::mdspan.
//   2. __cpp_lib_mdspan defined and ≥ 202207 — std::mdspan available natively
//      via the platform's libc++ / libstdc++ (C++23 toolchains).
//   3. Otherwise — fall back to the Kokkos reference implementation, which
//      vcpkg ships in `std::experimental::` (the namespace mandated by P0009).
#include <mdspan>

namespace tensor::core::detail {
#if defined(TENSOR_USE_STD_MDSPAN) || \
    (defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L)
template <class T, class Extents>
using mdspan_t = std::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = std::dextents<IndexType, Rank>;
#else
template <class T, class Extents>
using mdspan_t = std::experimental::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = std::experimental::dextents<IndexType, Rank>;
#endif
}  // namespace tensor::core::detail

#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

namespace detail {
// Compile-time helper: build a dextents<size_t, N>-shaped extent holder
// from a Shape<N> by copying extents.
template <std::size_t... I>
constexpr auto make_dextents(Shape<sizeof...(I)> const& s, std::index_sequence<I...>) {
    return dextents_t<std::size_t, sizeof...(I)>(s[I].extent...);
}
}  // namespace detail

// mdview(t) — non-owning mdspan over t's buffer, row-major layout.
template <class T, std::size_t N>
[[nodiscard]] auto mdview(Tensor<T, N>& t) {
    auto ext = detail::make_dextents(t.shape(), std::make_index_sequence<N>{});
    return detail::mdspan_t<T, detail::dextents_t<std::size_t, N>>(t.data(), ext);
}

template <class T, std::size_t N>
[[nodiscard]] auto mdview(Tensor<T, N> const& t) {
    auto ext = detail::make_dextents(t.shape(), std::make_index_sequence<N>{});
    return detail::mdspan_t<T const, detail::dextents_t<std::size_t, N>>(t.data(), ext);
}

// from_mdspan — copy an mdspan into a fresh owning Tensor with the supplied
// axis labels. The labels must match the mdspan rank N. Default layout and
// accessor only at this milestone; M3 generalises.
template <class T, std::size_t N>
[[nodiscard]] Tensor<std::remove_const_t<T>, N> from_mdspan(
    detail::mdspan_t<T, detail::dextents_t<std::size_t, N>> view,
    Shape<N> shape) {
    using U = std::remove_const_t<T>;
    std::vector<U> buf(view.size());
    // Copy element-by-element via mdspan operator() (works for any layout
    // policy; not the most efficient for layout_right + contiguous, but
    // M2 prioritises clarity over perf).
    if constexpr (N == 1) {
        for (std::size_t i = 0; i < view.extent(0); ++i) {
            buf[i] = view(i);
        }
    } else if constexpr (N == 2) {
        std::size_t k = 0;
        for (std::size_t i = 0; i < view.extent(0); ++i) {
            for (std::size_t j = 0; j < view.extent(1); ++j) {
                buf[k++] = view(i, j);
            }
        }
    } else {
        // For N >= 3, fall back to assuming contiguous layout_right and
        // memcpy-equivalent. M3 will generalise to arbitrary layouts.
        for (std::size_t i = 0; i < view.size(); ++i) {
            buf[i] = view.data_handle()[i];
        }
    }
    return Tensor<U, N>(std::move(shape), std::move(buf));
}

}  // namespace tensor::core
