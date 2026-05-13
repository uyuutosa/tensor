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

#include "tensor/autograd/activations.hpp"
#include "tensor/autograd/broadcast_ops.hpp"
#include "tensor/autograd/contract_ops.hpp"
#include "tensor/autograd/dynamic_variable.hpp"
#include "tensor/autograd/sgd.hpp"
#include "tensor/autograd/tape.hpp"
#include "tensor/autograd/variable.hpp"

#include "tensor/tex/evaluate.hpp"
#include "tensor/tex/expression.hpp"
#include "tensor/tex/parser.hpp"
#include "tensor/tex/render.hpp"

namespace nb = nanobind;
using namespace nb::literals;

using tensor::core::Axis;
using tensor::core::DynamicShape;
using tensor::core::DynamicTensor;
using tensor::core::contract;
using tensor::core::to_string;

namespace ag = tensor::autograd;
namespace tex = tensor::tex;

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

// ─── DynamicVariable<T> binding (used by the `tensor.autograd` submodule) ──

template <class T>
void bind_dynamic_variable(nb::module_& m, char const* py_name) {
    using DV = ag::DynamicVariable<T>;
    using DT = DynamicTensor<T>;

    nb::class_<DV>(m, py_name)
        .def(nb::init<>())
        .def(nb::init<DT, bool>(),
             "value"_a, "requires_grad"_a = false,
             "Wrap a DynamicTensor as a tape-tracked Variable. If "
             "`requires_grad=True`, gradient is accumulated through every "
             "subsequent op and surfaced via `.grad` after backward().")

        .def_prop_ro("value", &DV::value, nb::rv_policy::reference_internal,
                     "The forward-pass tensor value.")
        .def_prop_ro("grad",
                     [](DV const& v) -> DT const& { return v.grad(); },
                     nb::rv_policy::reference_internal,
                     "Accumulated gradient (set by backward()). Throws if "
                     "the variable does not require_grad.")
        .def_prop_ro("requires_grad", &DV::requires_grad)
        .def("zero_grad", &DV::zero_grad,
             "Reset the accumulated gradient to zero. No-op when "
             "requires_grad=False.")
        .def("seed_grad", &DV::seed_grad, "seed"_a,
             "Manually set the gradient accumulator — used internally "
             "by `backward()` for the loss-side seed.")
        .def("__repr__",
             [py_name](DV const& v) {
                 std::ostringstream os;
                 os << py_name << "(value=" << to_string(v.value())
                    << ", requires_grad=" << (v.requires_grad() ? "True" : "False")
                    << ")";
                 return os.str();
             })

        // Arithmetic — autograd-aware. Each returns a new DV whose
        // backward closure walks the right derivative back to the input.
        .def("__add__",
             [](DV const& a, DV const& b) -> DV { return a + b; })
        .def("__sub__",
             [](DV const& a, DV const& b) -> DV { return a - b; })
        .def("__mul__",
             [](DV const& a, DV const& b) -> DV { return a * b; });
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

    // ─── tensor.autograd submodule ─────────────────────────────────────
    //
    // The autograd subsystem extends the Domain (per ADR-0007 + ADR-0009).
    // Python sees it as `tensor.autograd`; the C++ side is the
    // `tensor::autograd::` namespace.

    nb::module_ autograd = m.def_submodule(
        "autograd",
        "Tape-based reverse-mode autograd over named-axis tensors. "
        "See docs/detailed-design/tensor-autograd.md for the design. "
        "P6.M4 surface — DynamicVariable + arithmetic + activations + "
        "dot + sum_all + backward + sgd_update.");

    bind_dynamic_variable<double>(autograd, "DynamicVariable");
    bind_dynamic_variable<float>(autograd, "DynamicVariableF32");

    // Scalar (rank-0) Variable<T, 0>. Surfaced only as the return type
    // of `sum_all` and the argument type of `backward`; users do not
    // construct it directly.
    using ScalarVarD = ag::Variable<double, 0>;
    nb::class_<ScalarVarD>(autograd, "_ScalarVariable")
        .def_prop_ro("value",
                     [](ScalarVarD const& v) -> double { return v.value()[0]; },
                     "Scalar forward value (the loss).")
        .def_prop_ro("requires_grad", &ScalarVarD::requires_grad);

    // Activations — autograd-aware.
    autograd.def("exp",
                 [](ag::DynamicVariable<double> const& x) { return ag::exp(x); });
    autograd.def("log",
                 [](ag::DynamicVariable<double> const& x) { return ag::log(x); });
    autograd.def("relu",
                 [](ag::DynamicVariable<double> const& x) { return ag::relu(x); });
    autograd.def("neg",
                 [](ag::DynamicVariable<double> const& x) { return ag::neg(x); });

    // Contraction — autograd-aware Einstein-sum (the autograd counterpart
    // of `tensor.contract`).
    autograd.def("dot",
                 [](ag::DynamicVariable<double> const& a,
                    ag::DynamicVariable<double> const& b) { return ag::dot(a, b); },
                 "a"_a, "b"_a,
                 "Autograd-aware contraction. Forward = `tensor.contract`; "
                 "backward routes gradient as `dL/da = contract(dL/dy, b)` + "
                 "symmetric.");

    // Sum-all → scalar Variable<T, 0>. The canonical bridge from a
    // multi-element tensor expression to the scalar loss that backward()
    // walks from.
    autograd.def("sum_all",
                 [](ag::DynamicVariable<double> const& x) { return ag::sum_all(x); },
                 "x"_a,
                 "Sum every element of `x` into a scalar Variable (rank 0). "
                 "Combined with `backward(...)` this is the canonical "
                 "loss-side seed for reverse-mode propagation.");

    // The reverse-mode entry point.
    autograd.def("backward",
                 [](ScalarVarD& loss) { ag::backward(loss); },
                 "loss"_a,
                 "Walk the tape backward from `loss` (a rank-0 scalar "
                 "Variable). Populates `.grad` on every DynamicVariable "
                 "with `requires_grad=True` reached on the forward path.");

    // SGD update — returns a new DynamicTensor with `v.value - lr * v.grad`.
    autograd.def("sgd_update",
                 [](ag::DynamicVariable<double> const& v, double lr) {
                     return ag::sgd_update(v, lr);
                 },
                 "v"_a, "lr"_a,
                 "Return a new DynamicTensor with one step of vanilla SGD "
                 "applied: `v.value - lr * v.grad`. Combine with a fresh "
                 "DynamicVariable wrapping for the next training iter.");

    // ─── tensor.tex submodule ──────────────────────────────────────────
    //
    // The `_tex` UDL surface in Python form. C++ users write
    // `R"(c_{ij} = a_i + b_j)"_tex`; Python users write
    // `tensor.tex.parse("c_{ij} = a_i + b_j")` (UDLs do not exist in
    // Python). The Evaluator + bind / evaluate pattern is identical.

    nb::module_ tex_mod = m.def_submodule(
        "tex",
        "LaTeX-subset DSL — \"the formula is the program\" (ADR-0005). "
        "P6.M5 surface — parse / to_latex / Expression / Evaluator. "
        "C++ users reach for the `_tex` UDL; Python users call "
        "`tensor.tex.parse(...)` and the `Evaluator` class.");

    // Expression — opaque AST node holder. The Python side gets repr()
    // via to_latex but does not introspect the structure. Subsequent
    // milestones could expose IndexedVar / BinOp / Sum / Equation /
    // Group node classes if a structural-walk use case appears.
    nb::class_<tex::Expression>(tex_mod, "Expression")
        .def(nb::init<>(),
             "Default-construct an empty Expression (no AST root). "
             "Users normally obtain Expressions via `parse(...)`; the "
             "default ctor is exposed for completeness and parity with "
             "the C++ side.")
        .def("empty", &tex::Expression::empty,
             "True when the expression carries no AST nodes.")
        .def("__repr__",
             [](tex::Expression const& e) {
                 std::ostringstream os;
                 os << "Expression(R\"(";
                 if (!e.empty()) {
                     os << tex::to_latex(e);
                 }
                 os << ")\")";
                 return os.str();
             })
        .def("__str__",
             [](tex::Expression const& e) {
                 return e.empty() ? std::string{} : tex::to_latex(e);
             });

    tex_mod.def("parse",
                [](std::string s) {
                    return tex::parse(std::string_view{s});
                },
                "source"_a,
                "Parse a LaTeX-subset string into an `Expression` AST. "
                "Equivalent to C++'s `R\"(...)\" _tex` UDL — Python has "
                "no UDLs, so this is the access path.");

    tex_mod.def("to_latex",
                [](tex::Expression const& e) { return tex::to_latex(e); },
                "expression"_a,
                "Render an Expression back to canonical LaTeX. Round-trip "
                "property: `parse(to_latex(e)) == e` for every supported "
                "Expression (see tests/test_tex_parser.cpp).");

    // Evaluator — template-instantiated for double + float. Bind named
    // tensors to AST IndexedVar leaves, then evaluate.
    using EvalD = tex::Evaluator<double>;
    nb::class_<EvalD>(tex_mod, "Evaluator")
        .def(nb::init<>(),
             "Default-construct an Evaluator. Bind tensors via `.bind()`, "
             "then call `.evaluate(expr)` to get a `DynamicTensor`.")
        .def("bind",
             [](EvalD& self, std::string name, DynamicTensor<double> value) {
                 self.bind(std::move(name), std::move(value));
             },
             "name"_a, "value"_a,
             "Bind a runtime tensor to a name. Subsequent `IndexedVar` "
             "nodes with that name resolve to this tensor at evaluate().")
        .def("evaluate",
             [](EvalD const& self, tex::Expression const& expr) {
                 return self.evaluate(expr);
             },
             "expression"_a,
             "Walk the AST and evaluate against the bound tensors. "
             "Returns a `DynamicTensor` shaped by the expression structure.");

    using EvalF = tex::Evaluator<float>;
    nb::class_<EvalF>(tex_mod, "EvaluatorF32")
        .def(nb::init<>())
        .def("bind",
             [](EvalF& self, std::string name, DynamicTensor<float> value) {
                 self.bind(std::move(name), std::move(value));
             },
             "name"_a, "value"_a)
        .def("evaluate",
             [](EvalF const& self, tex::Expression const& expr) {
                 return self.evaluate(expr);
             },
             "expression"_a);
}
