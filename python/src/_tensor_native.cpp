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
    //
    // Pass a const char* literal rather than a std::string here: nanobind's
    // `m.attr() = ...` expects something it can wrap as a nb::object
    // directly; std::string is not automatically convertible at module
    // init time and throws `std::bad_cast` on import.
    m.attr("__version__") = "0.1.0+dev";

    m.def("hello",
          []() -> const char* { return "hello from tensor::core"; },
          "Smoke binding: returns a fixed greeting from the C++ side. "
          "Proves that the nanobind extension loaded and a function call "
          "round-trips Python → C++ → Python.");
}
