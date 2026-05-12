// P6.M2 — Python bindings for the headline named-axis surface:
// `Axis` + `DynamicShape` + `DynamicTensor<double>` + `DynamicTensor<float>`
// with the four arithmetic operators (`+`, `-`, `*`, `/`) under Einstein-
// style label broadcast. Cross-validated against the C++ test suite
// (tests/test_ops.cpp + tests/test_broadcast.cpp) within `1e-12` for
// `double`. See docs/impl-plans/2026-05-12_phase-6-python-sdk.md and
// docs/arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md.

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <cstddef>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/format.hpp"
#include "tensor/core/ops.hpp"

namespace nb = nanobind;
using namespace nb::literals;

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::to_string;

namespace {

// Bind `DynamicTensor<T>` as a Python class. Two type-specialised classes
// land — `DynamicTensor` (T = double, the educational default) and
// `DynamicTensorF32` (T = float, for the WebGPU `float`-only path). Cross-
// dtype operators are deliberately not exposed in M2; promote in Python
// (`tensor.DynamicTensor.from_f32(tf32)`) or wait for M3+.
template <class T>
void bind_dynamic_tensor(nb::module_& m, char const* py_name) {
    using DT = DynamicTensor<T>;

    nb::class_<DT>(m, py_name)
        .def(nb::init<>())
        .def(nb::init<DynamicShape>(), "shape"_a)
        .def(nb::init<DynamicShape, std::vector<T>>(),
             "shape"_a, "buffer"_a,
             "Construct from a `DynamicShape` and a flat Python list of "
             "values. Length must equal `shape.total()`.")

        .def_prop_ro("shape", &DT::shape, nb::rv_policy::reference_internal)
        .def("size", &DT::size,
             "Number of elements in the flat buffer (equals shape.total()).")
        .def("__len__", &DT::size)
        .def("__getitem__",
             [](DT const& t, std::size_t i) -> T {
                 if (i >= t.size()) {
                     throw nb::index_error("DynamicTensor index out of range");
                 }
                 return t[i];
             },
             "i"_a,
             "Flat-buffer element access. Multi-index access lands in M3 "
             "with the NumPy buffer-protocol interop.")

        .def("__repr__",
             [py_name](DT const& t) {
                 std::ostringstream os;
                 os << py_name << "(\n" << to_string(t) << ")";
                 return os.str();
             })
        .def("__str__", [](DT const& t) { return to_string(t); })

        // Arithmetic with Einstein-style label broadcast — same-dtype only
        // for M2. `Ta + Tb` cross-dtype path of ops.hpp is deferred per
        // ADR-0018 §Considered Options Axis F.
        .def("__add__",
             [](DT const& a, DT const& b) -> DT { return a + b; })
        .def("__sub__",
             [](DT const& a, DT const& b) -> DT { return a - b; })
        .def("__mul__",
             [](DT const& a, DT const& b) -> DT { return a * b; })
        .def("__truediv__",
             [](DT const& a, DT const& b) -> DT { return a / b; });
}

}  // anonymous namespace

NB_MODULE(_tensor_native, m) {
    m.doc() =
        "tensor Python SDK — native nanobind extension (P6.M2). "
        "Exposes Axis / DynamicShape / DynamicTensor<double> / "
        "DynamicTensor<float> with the four arithmetic operators under "
        "Einstein-style label broadcast. Subsequent milestones add "
        "contract / NumPy interop (M3), autograd (M4), tex.Evaluator "
        "(M5), and runtime backend selection (M6).";

    m.attr("__version__") = "0.1.0+dev";

    m.def("hello",
          []() -> char const* { return "hello from tensor::core"; },
          "Smoke binding from P6.M1; kept as a quick `did the install "
          "work?` diagnostic.");

    // ── Axis ──────────────────────────────────────────────────────────
    nb::class_<Axis>(m, "Axis")
        .def(nb::init([](std::string label, std::size_t extent) {
                 return Axis{std::string_view{label}, extent};
             }),
             "label"_a, "extent"_a,
             "Construct an `Axis` from a string label and a non-negative "
             "extent. The label carries the named-axis semantic the "
             "library is built around.")
        .def_rw("label", &Axis::label)
        .def_rw("extent", &Axis::extent)
        .def("__repr__",
             [](Axis const& a) {
                 std::ostringstream os;
                 os << "Axis(\"" << a.label << "\", " << a.extent << ")";
                 return os.str();
             })
        .def("__eq__",
             [](Axis const& a, Axis const& b) {
                 return a.label == b.label && a.extent == b.extent;
             })
        .def("__hash__",
             [](Axis const& a) -> std::size_t {
                 return std::hash<std::string>{}(a.label) ^ (a.extent << 1);
             });

    // ── DynamicShape ──────────────────────────────────────────────────
    nb::class_<DynamicShape>(m, "DynamicShape")
        .def(nb::init<>())
        .def(nb::init<std::vector<Axis>>(), "axes"_a,
             "Construct a `DynamicShape` from a Python list of `Axis` "
             "objects.")
        .def("rank", &DynamicShape::rank,
             "Number of axes (length of the shape).")
        .def("total", &DynamicShape::total,
             "Product of every axis extent (total element count).")
        .def("axes", &DynamicShape::axes,
             nb::rv_policy::reference_internal,
             "Return the ordered list of axes.")
        .def("__len__", &DynamicShape::rank)
        .def("__repr__",
             [](DynamicShape const& s) {
                 std::ostringstream os;
                 os << "DynamicShape([";
                 for (std::size_t i = 0; i < s.rank(); ++i) {
                     if (i > 0) {
                         os << ", ";
                     }
                     os << "Axis(\"" << s.axes()[i].label << "\", "
                        << s.axes()[i].extent << ")";
                 }
                 os << "])";
                 return os.str();
             });

    // ── DynamicTensor<double> + <float> ───────────────────────────────
    bind_dynamic_tensor<double>(m, "DynamicTensor");
    bind_dynamic_tensor<float>(m, "DynamicTensorF32");
}
