// SPDX-License-Identifier: MIT
//
// tensor — the canonical reference for differentiable named-axis tensor
// computation in modern C++ (ADR-0013). Header-only, C++20/23, educational-
// first and production-capable via swappable KernelBackend adapters (ADR-0010
// / ADR-0011).
//
// Umbrella header — wires the public surface together:
//   tensor::core      — named-axis algebra (Tensor, Axis, Shape, broadcast,
//                       contract, ports declared in concepts.hpp). Domain.
//   tensor::autograd  — tape-based reverse-mode autograd over named-axis
//                       tensors (Variable, DynamicVariable, dot, exp/log/relu/
//                       neg, gradient_check, sgd_update). Domain (extends core).
//   tensor::tex       — LaTeX-subset parser, _tex UDL, Evaluator. DrivingAdapter.
//
// Citations: see CITATION.cff at the repo root and the ADR sequence under
// docs/arc42/09-decisions/ (the bibliography of this work, per ADR-0013).
//
// References:
//   - docs/arc42/01-introduction-and-goals/overview.md — goals (§1)
//   - docs/arc42/05-building-blocks/overview.md       — building blocks (§5)
//   - docs/arc42/09-decisions/                        — fourteen ADRs (§9)
//   - book/intro.md                                   — Jupyter Book front matter

#pragma once

#include "tensor/core/core.hpp"
