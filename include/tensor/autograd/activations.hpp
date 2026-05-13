// SPDX-License-Identifier: MIT
//
// tensor::autograd::activations — element-wise differentiable primitives.
//
// These are the building blocks a learner needs to compose loss functions
// and small NN layers. Each function takes a `Variable<T, N>` and returns
// a `Variable<T, N>` of the same shape; the backward closure captures the
// input value(s) it needs and contributes to the input's grad accumulator
// during `backward()`.

#pragma once

#include <cmath>
#include <cstddef>

#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// ─── exp ──────────────────────────────────────────────────────────────────────
// y = exp(x); dy/dx = exp(x) = y.
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> exp(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::exp(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        // Capture output value (= dy/dx for exp).
        auto out_val_copy = out.value();
        Tape::current().record([x_acc, out_acc, out_val_copy]() {
            core_tensor dx(out_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * out_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── log ──────────────────────────────────────────────────────────────────────
// y = log(x); dy/dx = 1 / x. Precondition: x > 0 element-wise (not enforced).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> log(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::log(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] / x_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── relu ─────────────────────────────────────────────────────────────────────
// y = max(0, x); dy/dx = I(x > 0).
//
// At x = 0 the subderivative is taken to be 0 (the conventional choice;
// matches PyTorch / TensorFlow / tinygrad). The educational notebook in
// P2.M5 will discuss the choice explicitly.
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> relu(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = x.value()[i] > T{0} ? x.value()[i] : T{0};
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = x_val_copy[i] > T{0} ? out_acc->grad[i] : T{0};
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── neg / unary operator- ────────────────────────────────────────────────────
// y = -x; dy/dx = -1 (element-wise).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> neg(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = -x.value()[i];
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        Tape::current().record([x_acc, out_acc]() {
            core_tensor dx(out_acc->grad.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = -out_acc->grad[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> operator-(Variable<T, N> const& x) {
    return neg(x);
}

// ─── sin ──────────────────────────────────────────────────────────────────────
// y = sin(x); dy/dx = cos(x). Capture x for the backward closure (we
// re-evaluate cos at backward time to avoid a second forward buffer).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> sin(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::sin(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * std::cos(x_val_copy[i]);
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── cos ──────────────────────────────────────────────────────────────────────
// y = cos(x); dy/dx = -sin(x).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> cos(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::cos(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = -out_acc->grad[i] * std::sin(x_val_copy[i]);
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── sqrt ─────────────────────────────────────────────────────────────────────
// y = sqrt(x); dy/dx = 1 / (2 sqrt(x)) = 1 / (2 y). Capture the output
// value (cheaper backward than re-deriving from x). Precondition: x > 0
// element-wise (not enforced; std::sqrt of negative returns NaN).
template <class T, std::size_t N>
[[nodiscard]] Variable<T, N> sqrt(Variable<T, N> const& x) {
    using core_tensor = tensor::core::Tensor<T, N>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::sqrt(x.value()[i]);
    }
    bool req = x.requires_grad();
    Variable<T, N> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto out_val_copy = out.value();
        Tape::current().record([x_acc, out_acc, out_val_copy]() {
            core_tensor dx(out_val_copy.shape());
            T const half = T{1} / T{2};
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * half / out_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

// ─── DynamicVariable variants ─────────────────────────────────────────────────
//
// The dynamic-rank counterparts of the four primitives above. Same forward
// and backward semantics; the closure now captures DynamicTensor rather than
// fixed-rank Tensor.

}  // namespace tensor::autograd

// Forward-declare DynamicVariable so we don't pull dynamic_variable.hpp into
// every consumer of activations.hpp. The actual definitions live in their
// own header to keep the include graph thin.
#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/core/dynamic_tensor.hpp"

namespace tensor::autograd {

template <class T>
[[nodiscard]] DynamicVariable<T> exp(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::exp(x.value()[i]);
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto out_val_copy = out.value();
        Tape::current().record([x_acc, out_acc, out_val_copy]() {
            core_tensor dx(out_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * out_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> log(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::log(x.value()[i]);
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] / x_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> relu(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = x.value()[i] > T{0} ? x.value()[i] : T{0};
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = x_val_copy[i] > T{0} ? out_acc->grad[i] : T{0};
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> neg(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = -x.value()[i];
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        Tape::current().record([x_acc, out_acc]() {
            core_tensor dx(out_acc->grad.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = -out_acc->grad[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> operator-(DynamicVariable<T> const& x) {
    return neg(x);
}

// ─── sin / cos / sqrt — DynamicVariable variants ─────────────────────────────

template <class T>
[[nodiscard]] DynamicVariable<T> sin(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::sin(x.value()[i]);
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * std::cos(x_val_copy[i]);
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> cos(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::cos(x.value()[i]);
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto x_val_copy = x.value();
        Tape::current().record([x_acc, out_acc, x_val_copy]() {
            core_tensor dx(x_val_copy.shape());
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = -out_acc->grad[i] * std::sin(x_val_copy[i]);
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

template <class T>
[[nodiscard]] DynamicVariable<T> sqrt(DynamicVariable<T> const& x) {
    using core_tensor = tensor::core::DynamicTensor<T>;
    core_tensor out_v(x.value().shape());
    for (std::size_t i = 0; i < out_v.size(); ++i) {
        out_v[i] = std::sqrt(x.value()[i]);
    }
    bool req = x.requires_grad();
    DynamicVariable<T> out(std::move(out_v), req);
    if (req) {
        auto x_acc = x.accum();
        auto out_acc = out.accum();
        auto out_val_copy = out.value();
        Tape::current().record([x_acc, out_acc, out_val_copy]() {
            core_tensor dx(out_val_copy.shape());
            T const half = T{1} / T{2};
            for (std::size_t i = 0; i < dx.size(); ++i) {
                dx[i] = out_acc->grad[i] * half / out_val_copy[i];
            }
            if (x_acc) x_acc->contribute(dx);
        });
    }
    return out;
}

}  // namespace tensor::autograd
