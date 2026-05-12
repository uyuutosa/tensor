// P6.M1 — Python SDK scaffold smoke binding.
//
// Goal: exercise the nanobind + scikit-build-core toolchain end-to-end
// (compile, install, import, call) before the actual DynamicTensor /
// DynamicVariable / Evaluator bindings land in P6.M2 / M4 / M5. See
// docs/impl-plans/2026-05-12_phase-6-python-sdk.md.

#include <nanobind/nanobind.h>
#include <string>

namespace nb = nanobind;

NB_MODULE(_tensor_native, m) {
    m.doc() =
        "tensor Python SDK — native nanobind extension (P6.M1 smoke). "
        "Subsequent milestones bind DynamicTensor / DynamicVariable / "
        "Evaluator / backend-selection; this module currently exposes only "
        "a version attribute and a smoke greeting.";

    // The version string is replicated rather than read from the C++ side
    // because the C++ headers do not currently expose a TENSOR_VERSION
    // macro. When P6.M6 ships the release ceremony for 0.2.0 the
    // pyproject.toml `version` and this constant move in lockstep — at
    // which point a TENSOR_VERSION macro in tensor/version.hpp lets us
    // drop the duplication.
    m.attr("__version__") = std::string{"0.1.0+dev"};

    m.def("hello",
          []() -> std::string { return std::string{"hello from tensor::core"}; },
          "Smoke binding: returns a fixed greeting from the C++ side. "
          "Proves that the nanobind extension loaded and a function call "
          "round-trips Python → C++ → Python.");
}
