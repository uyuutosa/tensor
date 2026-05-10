// SPDX-License-Identifier: MIT
//
// tensor::autograd — Domain-extension umbrella header (ADR-0009).
//
// Use this single include to bring in Variable, Tape, sum_all, backward,
// and gradient_check. The autograd subsystem extends tensor::core; per
// the dependency rule it depends on core but core does not depend on
// autograd.

#pragma once

#include "tensor/autograd/activations.hpp"
#include "tensor/autograd/broadcast_ops.hpp"
#include "tensor/autograd/concepts.hpp"
#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/grad_check.hpp"
#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"
