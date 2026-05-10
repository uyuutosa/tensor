// SPDX-License-Identifier: MIT
//
// tensor::autograd::tape — thread-local recording tape.
//
// Each tape entry is a type-erased BackwardOp (std::function<void()>)
// whose closure captures shared_ptrs to the GradAccum objects of the
// inputs and outputs of one primitive operator. During backward(), the
// tape walks entries in reverse and invokes each closure, which reads
// the output's accumulated gradient and contributes to the inputs'.
//
// This tape design is intentionally close to micrograd / tinygrad in
// shape, with the upgrade that the captured tensors are full
// Tensor<T, N>s rather than scalars.

#pragma once

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

#include "tensor/core/tensor.hpp"

namespace tensor::autograd {

// GradAccum — a per-Variable mutable gradient slot. Lives on the heap
// and is shared between the Variable that owns it and tape closures
// that need to write to it.
template <class T, std::size_t N>
struct GradAccum {
    tensor::core::Tensor<T, N> grad;
    bool initialized{false};

    void contribute(tensor::core::Tensor<T, N> const& delta) {
        if (!initialized) {
            grad = delta;
            initialized = true;
        } else {
            // Element-wise accumulation. Shapes must match (autograd MVP
            // restricts element-wise ops to same-shape inputs; broadcast
            // backward is a follow-up).
            for (std::size_t i = 0; i < grad.size(); ++i) {
                grad[i] = grad[i] + delta[i];
            }
        }
    }
};

// Tape — holds an ordered list of backward closures. Thread-local so
// concurrent autograd graphs in different threads don't interfere.
class Tape {
public:
    using Entry = std::function<void()>;

    void record(Entry e) { entries_.push_back(std::move(e)); }

    // Walk entries in reverse; each closure reads its captured output
    // grad accumulator and writes to the input ones.
    void backward() {
        for (auto it = entries_.rbegin(); it != entries_.rend(); ++it) {
            (*it)();
        }
    }

    void clear() { entries_.clear(); }

    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }

    [[nodiscard]] static Tape& current() {
        thread_local Tape t;
        return t;
    }

private:
    std::vector<Entry> entries_;
};

// Helper: scoped clear — clears the tape on destruction. Useful as a
// guard around training-loop iterations.
struct TapeReset {
    ~TapeReset() { Tape::current().clear(); }
};

}  // namespace tensor::autograd
