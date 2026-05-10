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
#include "tensor/core/broadcast.hpp"
#include "tensor/core/concepts.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/format.hpp"
#include "tensor/core/mdspan_interop.hpp"
#include "tensor/core/ops.hpp"
#include "tensor/core/shape.hpp"
#include "tensor/core/tensor.hpp"
#include "tensor/core/version.hpp"
