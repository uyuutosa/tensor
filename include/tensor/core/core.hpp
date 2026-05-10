// SPDX-License-Identifier: MIT
//
// tensor::core — Domain umbrella header.
//
// Per ADR-0009, this is the user-facing entry point for the Domain layer.
// Adapters (tensor::tex, tensor::gpu, tensor::autograd) MUST NOT include
// this file; they include only `concepts.hpp` plus the specific concrete
// types they need (Axis, Shape) by their dedicated headers.

#pragma once

#include "tensor/core/axis.hpp"
#include "tensor/core/backend/reference.hpp"
#include "tensor/core/broadcast.hpp"
#include "tensor/core/concepts.hpp"
#include "tensor/core/contract.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/format.hpp"
#include "tensor/core/function_tensor.hpp"
#include "tensor/core/label_tag.hpp"
// "tensor/core/mdspan_interop.hpp" — temporarily not in the umbrella; the
// vcpkg-shipped Kokkos polyfill puts symbols in `Kokkos::` not `std::` /
// `std::experimental::`, and the right namespace adapter has been deferred
// to a Phase 1.5 follow-up. The header is preserved in the repo for that
// follow-up to reuse but is not built or tested at this milestone.
#include "tensor/core/ops.hpp"
#include "tensor/core/reference_tensor.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"
#include "tensor/core/version.hpp"
