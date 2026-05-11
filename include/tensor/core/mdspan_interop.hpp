// SPDX-License-Identifier: MIT
//
// tensor::core::mdspan_interop — bidirectional bridge to std::mdspan
// (or the Kokkos polyfill when std::mdspan is unavailable).
//
// Per ADR-0002, mdspan is the lingua franca for cross-library data
// exchange. The active namespace is detected via the polyfill's own
// macros (MDSPAN_IMPL_STANDARD_NAMESPACE / MDSPAN_IMPL_PROPOSED_NAMESPACE);
// the Kokkos reference impl defaults them to `Kokkos` / `Experimental`.
// If C++23 `std::mdspan` is natively available (feature-test macro
// `__cpp_lib_mdspan` ≥ 202207), the std path is used instead.

#pragma once

#include <cstddef>
#include <utility>
#include <vector>

// Setting MDSPAN_IMPL_STANDARD_NAMESPACE / MDSPAN_IMPL_PROPOSED_NAMESPACE
// before including <mdspan> would force the polyfill's namespace; we keep
// the polyfill's defaults (Kokkos::Experimental::) and reach the types via
// the macros it leaves defined.
#include <mdspan>

#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core::detail {

#if defined(TENSOR_USE_STD_MDSPAN) || \
    (defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L)
template <class T, class Extents>
using mdspan_t = std::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = std::dextents<IndexType, Rank>;
#else
// Kokkos polyfill — types live at MDSPAN_IMPL_STANDARD_NAMESPACE::
// MDSPAN_IMPL_PROPOSED_NAMESPACE::, both of which the polyfill defines
// (defaults: Kokkos / Experimental). Using the macros makes the adapter
// robust to vcpkg's port-level configuration choices.
#  ifndef MDSPAN_IMPL_STANDARD_NAMESPACE
#    define MDSPAN_IMPL_STANDARD_NAMESPACE Kokkos
#  endif
#  ifndef MDSPAN_IMPL_PROPOSED_NAMESPACE
#    define MDSPAN_IMPL_PROPOSED_NAMESPACE Experimental
#  endif
template <class T, class Extents>
using mdspan_t = MDSPAN_IMPL_STANDARD_NAMESPACE::MDSPAN_IMPL_PROPOSED_NAMESPACE
    ::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = MDSPAN_IMPL_STANDARD_NAMESPACE::MDSPAN_IMPL_PROPOSED_NAMESPACE
    ::dextents<IndexType, Rank>;
#endif

}  // namespace tensor::core::detail

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
// axis labels. Default layout and accessor only at this milestone.
template <class T, std::size_t N>
[[nodiscard]] Tensor<std::remove_const_t<T>, N> from_mdspan(
    detail::mdspan_t<T, detail::dextents_t<std::size_t, N>> view,
    Shape<N> shape) {
    using U = std::remove_const_t<T>;
    std::vector<U> buf(view.size());
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
        for (std::size_t i = 0; i < view.size(); ++i) {
            buf[i] = view.data_handle()[i];
        }
    }
    return Tensor<U, N>(std::move(shape), std::move(buf));
}

}  // namespace tensor::core
