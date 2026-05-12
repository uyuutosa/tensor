// P6.M3 — adds the named-axis contraction primitive + NumPy interop on
// top of M2's `Axis` / `DynamicShape` / `DynamicTensor<{double,float}>`
// + arithmetic surface. Cross-validated against `tests/test_contract.cpp`
// and ``np.einsum`` for representative contractions. See the Phase 6
// impl-plan and ADR-0018.

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "tensor/core/axis.hpp"
#include "tensor/core/contract.hpp"
#include "tensor/core/dynamic_shape.hpp"
#include "tensor/core/dynamic_tensor.hpp"
#include "tensor/core/format.hpp"
#include "tensor/core/ops.hpp"

namespace nb = nanobind;
using namespace nb::literals;

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::contract;
using tensor::core::to_string;

namespace {

// ─── from_numpy / to_numpy helpers ────────────────────────────────────────
//
// nanobind's `<nanobind/ndarray.h>` is the framework-agnostic ndarray
// surface (numpy, torch, jax, tensorflow). For M3 we use it with the
// `nb::numpy` framework tag so the Python side sees plain
// ``numpy.ndarray``. Zero-copy reads on input (from_numpy reads through
// the buffer); allocate-and-copy on output (`.numpy()` returns a fresh
// Python-owned ndarray) so the lifetime story is unambiguous — the
// DynamicTensor and the returned ndarray are independent objects.

template <class T>
DynamicTensor<T> from_numpy_typed(
    nb::ndarray<T, nb::c_contig> const& arr,
    std::vector<std::string> const& labels)
{
    if (labels.size() != arr.ndim()) {
        throw std::invalid_argument(
            "from_numpy: number of labels (" + std::to_string(labels.size()) +
            ") does not match array rank (" + std::to_string(arr.ndim()) + ")");
    }

    std::vector<Axis> axes;
    axes.reserve(arr.ndim());
    for (std::size_t k = 0; k < arr.ndim(); ++k) {
        axes.emplace_back(std::string_view{labels[k]},
                          static_cast<std::size_t>(arr.shape(k)));
    }

    DynamicShape shape{std::move(axes)};
    DynamicTensor<T> result{shape};

    // arr.c_contig => stride pattern is row-major contiguous, so a
    // straight std::copy_n from arr.data() into result.data() is valid.
    std::copy_n(arr.data(), arr.size(), result.data());
    return result;
}

template <class T>
nb::ndarray<nb::numpy, T> to_numpy_typed(DynamicTensor<T> const& t)
{
    auto const& axes = t.shape().axes();
    auto const ndim = t.shape().rank();

    // Allocate a fresh Python-owned buffer. The capsule deleter cleans
    // it up when Python's refcount drops to zero, decoupling the
    // ndarray's lifetime from the originating DynamicTensor.
    std::size_t const n = t.size();
    T* buf = new T[n];
    std::copy_n(t.data(), n, buf);

    std::vector<std::size_t> shape(ndim);
    for (std::size_t k = 0; k < ndim; ++k) {
        shape[k] = axes[k].extent;
    }

    nb::capsule owner(buf, [](void* p) noexcept {
        delete[] static_cast<T*>(p);
    });

    return nb::ndarray<nb::numpy, T>(buf, ndim, shape.data(), owner);
}

// ─── DynamicTensor binding ───────────────────────────────────────────────

template <class T>
void bind_dynamic_tensor(nb::module_& m, char const* py_name)
{
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
             "Flat-buffer element access. For multi-dimensional indexing "
             "use NumPy interop via `.numpy()` then `arr[i, j, ...]`.")

        .def("numpy",
             [](DT const& t) { return to_numpy_typed<T>(t); },
             "Return a fresh ``numpy.ndarray`` copy of the buffer with "
             "the tensor's shape extents. Labels are dropped — NumPy is "
             "positional. Use ``from_numpy`` to round-trip back into the "
             "named-axis world.")

        .def("__repr__",
             [py_name](DT const& t) {
                 std::ostringstream os;
                 os << py_name << "(\n" << to_string(t) << ")";
                 return os.str();
             })
        .def("__str__", [](DT const& t) { return to_string(t); })

        // Arithmetic with Einstein-style label broadcast — same-dtype only.
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
        "tensor Python SDK — native nanobind extension (P6.M3). "
        "Adds `contract` (named-axis Einstein-sum) + NumPy interop "
        "(from_numpy / .numpy()) on top of M2's `DynamicTensor` + four "
        "arithmetic operators. Subsequent milestones add autograd (M4), "
        "`tex.Evaluator` (M5), and runtime backend selection (M6).";

    m.attr("__version__") = "0.1.0+dev";

    m.def("hello",
          []() -> char const* { return "hello from tensor::core"; },
          "Smoke binding from P6.M1; kept as a quick `did the install "
          "work?` diagnostic.");

    // ── Axis ──────────────────────────────────────────────────────────
    nb::class_<Axis>(m, "Axis")
        .def("__init__",
             [](Axis* self, std::string label, std::size_t extent) {
                 new (self) Axis(std::string_view{label}, extent);
             },
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
        .def("rank", &DynamicShape::rank)
        .def("total", &DynamicShape::total)
        .def("axes", &DynamicShape::axes,
             nb::rv_policy::reference_internal)
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

    // ── Free-function contraction ─────────────────────────────────────
    //
    // `tensor::core::contract` is the named-axis Einstein-sum primitive:
    // axes that appear in both inputs are summed over; axes that appear
    // in only one form the result. Matrix-matrix / matrix-vector / outer
    // product / inner product are all the same op specialised by the
    // label structure (see tests/test_contract.cpp).

    m.def("contract",
          [](DynamicTensor<double> const& a, DynamicTensor<double> const& b) {
              return contract(a, b);
          },
          "a"_a, "b"_a,
          "Einstein-sum contraction over shared axis labels. Mirrors "
          "``np.einsum(\"ij,jk->ik\", A, B)`` when ``a.labels = ('i','j')`` "
          "and ``b.labels = ('j','k')``. Same dtype only.");

    m.def("contract",
          [](DynamicTensor<float> const& a, DynamicTensor<float> const& b) {
              return contract(a, b);
          },
          "a"_a, "b"_a,
          "Float32 overload of contract().");

    // ── NumPy interop ────────────────────────────────────────────────

    m.def("from_numpy",
          [](nb::ndarray<double, nb::c_contig> arr,
             std::vector<std::string> labels) {
              return from_numpy_typed<double>(arr, labels);
          },
          "arr"_a, "labels"_a,
          "Wrap a contiguous float64 ``numpy.ndarray`` as a "
          "``DynamicTensor`` with the given axis labels. The labels list "
          "must have one entry per array dimension. Data is copied — the "
          "returned tensor is independent of the NumPy array.");

    m.def("from_numpy",
          [](nb::ndarray<float, nb::c_contig> arr,
             std::vector<std::string> labels) {
              return from_numpy_typed<float>(arr, labels);
          },
          "arr"_a, "labels"_a,
          "Float32 overload of from_numpy().");
}
