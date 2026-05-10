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

// Toolchain selector: TENSOR_USE_STD_MDSPAN is set by CMake when the user
// requests std::mdspan from <mdspan>. Otherwise we use the Kokkos reference
// implementation (vendored via vcpkg) which exposes the same API under
// std:: as well.
#if defined(TENSOR_USE_STD_MDSPAN)
#    include <mdspan>
#else
// Kokkos reference impl from <mdspan> ships in `std::` namespace too when
// configured with appropriate macros; this is what the `mdspan` vcpkg port
// provides. The header path under vcpkg is the same `<mdspan>`.
#    include <mdspan>
#endif

#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

namespace detail {
// Compile-time helper: build a std::dextents<size_t, N>-shaped extent
// holder from a Shape<N> by copying extents.
template <std::size_t... I>
constexpr auto make_dextents(Shape<sizeof...(I)> const& s, std::index_sequence<I...>) {
    return std::dextents<std::size_t, sizeof...(I)>(s[I].extent...);
}
}  // namespace detail

// mdview(t) — non-owning std::mdspan over t's buffer, row-major layout.
template <class T, std::size_t N>
[[nodiscard]] auto mdview(Tensor<T, N>& t) {
    auto ext = detail::make_dextents(t.shape(), std::make_index_sequence<N>{});
    return std::mdspan<T, std::dextents<std::size_t, N>>(t.data(), ext);
}

template <class T, std::size_t N>
[[nodiscard]] auto mdview(Tensor<T, N> const& t) {
    auto ext = detail::make_dextents(t.shape(), std::make_index_sequence<N>{});
    return std::mdspan<T const, std::dextents<std::size_t, N>>(t.data(), ext);
}

// from_mdspan — copy an mdspan into a fresh owning Tensor with the supplied
// axis labels. The labels must match the mdspan rank N.
template <class T, std::size_t N, class Layout, class Accessor>
[[nodiscard]] Tensor<std::remove_const_t<T>, N> from_mdspan(
    std::mdspan<T, std::dextents<std::size_t, N>, Layout, Accessor> view,
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
