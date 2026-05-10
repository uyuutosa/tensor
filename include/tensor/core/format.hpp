// SPDX-License-Identifier: MIT
//
// tensor::core::format — pretty-printing for Tensor.
//
// The output format is a deliberate descendant of the 2016 ASCII boxes
// (see archive/legacy-2016/Tensor/Tensor.cpp Test_main): an info banner
// summarising rank / shape / total size, then a values block. The legacy
// "Up or down" line is dropped — it referred to covariant / contravariant
// flags that the rewrite does not preserve in M2.

#pragma once

#include <cstddef>
#include <ostream>
#include <sstream>
#include <string>

#include "tensor/core/concepts.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::core {

namespace detail {

template <TensorLike Tt>
void print_info_block(std::ostream& os, Tt const& t) {
    auto const& shape = t.shape();
    auto const n = shape.rank();
    os << "-*-Infos-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
    os << "Total size  : " << t.size() << '\n';
    os << "Shape       : (";
    for (std::size_t i = 0; i < n; ++i) {
        os << shape[i].label << ": " << shape[i].extent;
        if (i + 1 < n) {
            os << ", ";
        }
    }
    os << ")\n";
    os << "Num. of Dim.: " << n << '\n';
}

template <class T>
void print_values_1d(std::ostream& os, Tensor<T, 1> const& t) {
    os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
    os << '[';
    for (std::size_t i = 0; i < t.shape()[0].extent; ++i) {
        os << ' ' << t(i);
        if (i + 1 < t.shape()[0].extent) {
            os << ',';
        }
    }
    os << " ]\n";
}

template <class T>
void print_values_2d(std::ostream& os, Tensor<T, 2> const& t) {
    os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
    auto const rows = t.shape()[0].extent;
    auto const cols = t.shape()[1].extent;
    os << '[';
    for (std::size_t i = 0; i < rows; ++i) {
        if (i > 0) {
            os << ' ';
        }
        os << '[';
        for (std::size_t j = 0; j < cols; ++j) {
            os << ' ' << t(i, j);
            if (j + 1 < cols) {
                os << ',';
            }
        }
        os << " ]";
        if (i + 1 < rows) {
            os << '\n';
        }
    }
    os << "]\n";
}

template <class T, std::size_t N>
void print_values_flat(std::ostream& os, Tensor<T, N> const& t) {
    os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
    os << '[';
    for (std::size_t i = 0; i < t.size(); ++i) {
        os << ' ' << t[i];
        if (i + 1 < t.size()) {
            os << ',';
        }
    }
    os << " ]\n";
    os << "(rank " << N << " — flat dump; pretty rank-N print is M3+)\n";
}

}  // namespace detail

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& os, Tensor<T, N> const& t) {
    detail::print_info_block(os, t);
    if constexpr (N == 1) {
        detail::print_values_1d(os, t);
    } else if constexpr (N == 2) {
        detail::print_values_2d(os, t);
    } else {
        detail::print_values_flat(os, t);
    }
    return os;
}

// DynamicTensor variant — picks rank-1/2 specialised printers when the
// runtime shape rank matches; otherwise flat dump.
template <class T>
std::ostream& operator<<(std::ostream& os, DynamicTensor<T> const& t) {
    detail::print_info_block(os, t);
    auto const r = t.shape().rank();
    if (r == 1) {
        // Promote to a temporary Tensor<T,1>-shaped view of the printer
        // by writing the rank-1 values block inline; we cannot directly
        // call print_values_1d(t) because it expects Tensor<T,1>.
        os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
        os << '[';
        for (std::size_t i = 0; i < t.size(); ++i) {
            os << ' ' << t[i];
            if (i + 1 < t.size()) {
                os << ',';
            }
        }
        os << " ]\n";
    } else if (r == 2) {
        os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
        auto const rows = t.shape()[0].extent;
        auto const cols = t.shape()[1].extent;
        os << '[';
        for (std::size_t i = 0; i < rows; ++i) {
            if (i > 0) {
                os << ' ';
            }
            os << '[';
            for (std::size_t j = 0; j < cols; ++j) {
                os << ' ' << t[i * cols + j];
                if (j + 1 < cols) {
                    os << ',';
                }
            }
            os << " ]";
            if (i + 1 < rows) {
                os << '\n';
            }
        }
        os << "]\n";
    } else {
        os << "-*-Values-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n";
        os << '[';
        for (std::size_t i = 0; i < t.size(); ++i) {
            os << ' ' << t[i];
            if (i + 1 < t.size()) {
                os << ',';
            }
        }
        os << " ]\n";
        os << "(rank " << r << " — flat dump; pretty rank-N print is M3+)\n";
    }
    return os;
}

// Convenience: Python-style "to string" without going through a stream.
template <class T, std::size_t N>
[[nodiscard]] std::string to_string(Tensor<T, N> const& t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template <class T>
[[nodiscard]] std::string to_string(DynamicTensor<T> const& t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

}  // namespace tensor::core
