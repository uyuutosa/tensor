// SPDX-License-Identifier: MIT
// Quick debug smoke for broadcast_add via the project's webgpu::Backend.

#include <cmath>
#include <cstddef>
#include <iostream>

#include <tensor/core/axis.hpp>
#include <tensor/core/broadcast.hpp>
#include <tensor/core/dynamic_shape.hpp>
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/backend/reference.hpp>
#include <tensor/core/backend/webgpu.hpp>

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using WB = tensor::core::backend::webgpu::Backend;
using RB = tensor::core::backend::reference::Backend;

int main() {
    DynamicTensor<float> a(DynamicShape{Axis{"i", 4}}, {1.0f, 2.0f, 3.0f, 4.0f});
    DynamicTensor<float> b(DynamicShape{Axis{"j", 3}}, {10.0f, 20.0f, 30.0f});
    auto plan = tensor::core::broadcast_shapes(a.shape(), b.shape());

    std::cout << "plan.result.rank=" << plan.result.rank() << "\n";
    std::cout << "a_source:";
    for (auto s : plan.a_source) std::cout << " " << static_cast<long long>(s);
    std::cout << "\nb_source:";
    for (auto s : plan.b_source) std::cout << " " << static_cast<long long>(s);
    std::cout << "\n";

    WB w; RB r;
    auto wo = w.broadcast_add(a, b, plan);
    auto ro = r.broadcast_add(a, b, plan);

    std::cout << "w_out:";
    for (std::size_t i = 0; i < wo.size(); ++i) std::cout << " " << wo[i];
    std::cout << "\nr_out:";
    for (std::size_t i = 0; i < ro.size(); ++i) std::cout << " " << ro[i];
    std::cout << "\n";

    int mismatches = 0;
    for (std::size_t i = 0; i < wo.size(); ++i) {
        if (std::fabs(wo[i] - ro[i]) > 1e-5f) ++mismatches;
    }
    std::cout << "mismatches: " << mismatches << " / " << wo.size() << "\n";
    return mismatches == 0 ? 0 : 1;
}
