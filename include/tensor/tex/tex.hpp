// SPDX-License-Identifier: MIT
//
// tensor::tex — DrivingAdapter umbrella header (ADR-0009).
//
// `tensor::tex` produces tensor::core-bound expression graphs from
// LaTeX-subset strings. Per the dependency rule, this adapter only
// reaches into core through concepts.hpp (and concrete public types
// like Axis); core never depends on tex.

#pragma once

#include "tensor/tex/expression.hpp"
#include "tensor/tex/parser.hpp"
#include "tensor/tex/render.hpp"
#include "tensor/tex/udl.hpp"
