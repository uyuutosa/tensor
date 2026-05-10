// SPDX-License-Identifier: MIT
//
// tensor::core::TypedTensor — compile-time-labelled tensor wrapper.
//
// Realises the second half of the ADR-0004 hybrid named-axis API. The
// existing `Tensor<T, N>` carries axes as runtime values; `TypedTensor`
// carries the same data with **the labels lifted to non-type template
// parameters**, so the compiler enforces axis-label matching at every
// operator site.
//
// Concretely:
//
//   TypedTensor<double, "i"_ax, "j"_ax> a{ {{2, 3}}, {1, 2, 3, 4, 5, 6} };
//   TypedTensor<double, "i"_ax, "j"_ax> b{ {{2, 3}}, {6, 5, 4, 3, 2, 1} };
//   auto c = a + b;     //  ✅ same labels — compiles and runs.
//
//   TypedTensor<double, "i"_ax> p{ {{3}}, {1, 2, 3} };
//   TypedTensor<double, "j"_ax> q{ {{3}}, {4, 5, 6} };
//   auto r = p + q;     //  ❌ static_assert: "axis labels must match".
//
// To bring back the runtime-broadcast Einstein algebra, drop into
// `DynamicTensor` via the `to_dynamic()` member.

#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/label_tag.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

template <class T, FixedString... Labels>
class TypedTensor {
public:
    static constexpr std::size_t rank() noexcept { return sizeof...(Labels); }
    using value_type = T;

    // Construct from a static-rank Tensor whose axes already carry the
    // matching labels at runtime. No runtime check is required because
    // construction-from-extents (the next overload) is the canonical
    // entry point and it pins labels at compile time.
    explicit TypedTensor(Tensor<T, sizeof...(Labels)> inner) : inner_(std::move(inner)) {}

    // Construct from an extent-per-label array. The axes' labels are
    // taken from the compile-time NTTP pack. This is the recommended
    // entry point for application code.
    TypedTensor(std::array<std::size_t, sizeof...(Labels)> const& extents,
                std::vector<T> values) {
        std::array<Axis, sizeof...(Labels)> axes = make_axes(extents);
        Shape<sizeof...(Labels)> shape{axes};
        if (values.size() != shape.total()) {
            throw std::invalid_argument(
                "TypedTensor: value count != product of extents");
        }
        inner_ = Tensor<T, sizeof...(Labels)>(std::move(shape), std::move(values));
    }

    // ─── Inspection ──────────────────────────────────────────────────────
    [[nodiscard]] Tensor<T, sizeof...(Labels)> const& inner() const noexcept {
        return inner_;
    }
    [[nodiscard]] std::size_t size() const noexcept { return inner_.size(); }
    [[nodiscard]] T const& operator[](std::size_t i) const noexcept { return inner_[i]; }
    [[nodiscard]] T& operator[](std::size_t i) noexcept { return inner_[i]; }

    // ─── Convert to runtime DynamicTensor (gives up the type-level labels) ─
    [[nodiscard]] DynamicTensor<T> to_dynamic() const {
        return DynamicTensor<T>(inner_);
    }

    // ─── Type-level label introspection ──────────────────────────────────
    // Get the i-th label as a string_view at compile time. The FixedString
    // values are converted to string_view at compile time so we never need
    // to store heterogeneously-sized FixedStrings in one array.
    template <std::size_t I>
    static constexpr std::string_view label_at() noexcept {
        constexpr std::array<std::string_view, sizeof...(Labels)> views{Labels.view()...};
        return views[I];
    }

    friend bool operator==(TypedTensor const&, TypedTensor const&) = default;

private:
    static std::array<Axis, sizeof...(Labels)> make_axes(
        std::array<std::size_t, sizeof...(Labels)> const& extents) {
        std::array<Axis, sizeof...(Labels)> out{};
        std::size_t i = 0;
        ((out[i] = Axis{Labels.view(), extents[i]}, ++i), ...);
        return out;
    }

    Tensor<T, sizeof...(Labels)> inner_{};
};

// ─── Concept: SameLabels<TypedTensor A, TypedTensor B> ───────────────────────
//
// True iff the two types have identical FixedString packs. Used by the
// operators to drive the static_assert message.
namespace detail {
template <class A, class B>
struct same_labels_impl : std::false_type {};

template <class T, FixedString... La, class U, FixedString... Lb>
    requires (sizeof...(La) == sizeof...(Lb)) && ((La == Lb) && ...)
struct same_labels_impl<TypedTensor<T, La...>, TypedTensor<U, Lb...>> : std::true_type {};
}  // namespace detail

template <class A, class B>
concept SameLabels = detail::same_labels_impl<A, B>::value;

// ─── Operators ───────────────────────────────────────────────────────────────
//
// Static-label arithmetic. The constraints enforce label-pack equality
// at compile time; the static_assert in the body gives a learner-friendly
// error message if a user circumvents the constraint via SFINAE tricks.
template <class T, FixedString... La, FixedString... Lb>
[[nodiscard]] TypedTensor<T, La...> operator+(TypedTensor<T, La...> const& a,
                                              TypedTensor<T, Lb...> const& b) {
    static_assert(SameLabels<TypedTensor<T, La...>, TypedTensor<T, Lb...>>,
                  "TypedTensor: axis labels must match at compile time. "
                  "Drop into DynamicTensor with .to_dynamic() if you need broadcast.");
    Tensor<T, sizeof...(La)> sum(a.inner().shape());
    for (std::size_t i = 0; i < sum.size(); ++i) sum[i] = a[i] + b[i];
    return TypedTensor<T, La...>(std::move(sum));
}

template <class T, FixedString... La, FixedString... Lb>
[[nodiscard]] TypedTensor<T, La...> operator-(TypedTensor<T, La...> const& a,
                                              TypedTensor<T, Lb...> const& b) {
    static_assert(SameLabels<TypedTensor<T, La...>, TypedTensor<T, Lb...>>,
                  "TypedTensor: axis labels must match at compile time.");
    Tensor<T, sizeof...(La)> diff(a.inner().shape());
    for (std::size_t i = 0; i < diff.size(); ++i) diff[i] = a[i] - b[i];
    return TypedTensor<T, La...>(std::move(diff));
}

template <class T, FixedString... La, FixedString... Lb>
[[nodiscard]] TypedTensor<T, La...> operator*(TypedTensor<T, La...> const& a,
                                              TypedTensor<T, Lb...> const& b) {
    static_assert(SameLabels<TypedTensor<T, La...>, TypedTensor<T, Lb...>>,
                  "TypedTensor: axis labels must match at compile time.");
    Tensor<T, sizeof...(La)> prod(a.inner().shape());
    for (std::size_t i = 0; i < prod.size(); ++i) prod[i] = a[i] * b[i];
    return TypedTensor<T, La...>(std::move(prod));
}

}  // namespace tensor::core
