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

// Prefer C++23 native <mdspan> when available (libstdc++ 13+ / libc++ 17+);
// otherwise fall back to the kokkos polyfill (vcpkg port `mdspan`), which
// installs at <experimental/mdspan> and reaches the types via
// MDSPAN_IMPL_STANDARD_NAMESPACE::MDSPAN_IMPL_PROPOSED_NAMESPACE (defaults
// Kokkos::Experimental). The `__has_include` probe makes this robust to
// either path; the user can override with -DTENSOR_USE_STD_MDSPAN if their
// toolchain requires it.
#if defined(__cpp_lib_mdspan) && __cpp_lib_mdspan >= 202207L && __has_include(<mdspan>)
#  include <mdspan>
#  define TENSOR_HAVE_STD_MDSPAN 1
#elif __has_include(<experimental/mdspan>)
#  include <experimental/mdspan>
#elif __has_include(<mdspan>)
#  include <mdspan>
#else
// Neither <mdspan> nor <experimental/mdspan> found in the current
// toolchain (most often: xeus-cpp's Clang-Repl which doesn't ship the
// Kokkos polyfill that vcpkg provides for the C++ build). Define the
// MDSPAN_INTEROP_UNAVAILABLE marker so downstream code can fall back
// to non-mdspan paths. Tutorials that only need the runtime tensor
// surface (DynamicTensor, ops, contract, autograd, tex) compile fine;
// only the `mdview` / `from_mdspan` helpers below become unavailable.
#  define TENSOR_MDSPAN_INTEROP_UNAVAILABLE 1
#endif

#ifndef TENSOR_MDSPAN_INTEROP_UNAVAILABLE

#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core::detail {

#if defined(TENSOR_USE_STD_MDSPAN) || defined(TENSOR_HAVE_STD_MDSPAN)
// Native C++23 std::mdspan.
template <class T, class Extents>
using mdspan_t = std::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = std::dextents<IndexType, Rank>;
#else
// kokkos/mdspan polyfill — current vcpkg port (`mdspan@0.6.0`) exposes
// the types at `std::experimental::`. Older vcpkg ports defaulted to
// `Kokkos::Experimental::`; we accommodate both via the polyfill's own
// macros when defined, otherwise fall back to `std::experimental::`.
#  if defined(MDSPAN_IMPL_STANDARD_NAMESPACE) && defined(MDSPAN_IMPL_PROPOSED_NAMESPACE)
template <class T, class Extents>
using mdspan_t = MDSPAN_IMPL_STANDARD_NAMESPACE::MDSPAN_IMPL_PROPOSED_NAMESPACE
    ::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = MDSPAN_IMPL_STANDARD_NAMESPACE::MDSPAN_IMPL_PROPOSED_NAMESPACE
    ::dextents<IndexType, Rank>;
#  else
template <class T, class Extents>
using mdspan_t = std::experimental::mdspan<T, Extents>;
template <class IndexType, std::size_t Rank>
using dextents_t = std::experimental::dextents<IndexType, Rank>;
#  endif
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

#endif  // !TENSOR_MDSPAN_INTEROP_UNAVAILABLE
