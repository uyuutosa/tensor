---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ namespace overview — `tensor::*` reference

> **Diátaxis quadrant**: Reference. Information-oriented; lists every public C++ namespace and the symbols that live there with cross-refs to the detailed-design. For "how do I…?" guidance, see [`../how-to/`](../how-to/). For first-time orientation, see [`../tutorials/cpp-named-axis-basics.md`](../tutorials/cpp-named-axis-basics.md).

## At a glance

```text
tensor::
├── core::                              Domain centerpiece
│   ├── Axis                             label + extent pair
│   ├── Shape<N>                         compile-time-rank shape
│   ├── DynamicShape                     runtime-rank shape (ordered Axis list)
│   ├── Tensor<T, N>                     compile-time-rank tensor
│   ├── DynamicTensor<T>                 runtime-rank tensor
│   ├── TypedTensor<T, "i", "j">         compile-time-labelled tensor
│   ├── LabelTag<S>                      compile-time label tag (NTTP)
│   ├── FixedString<N>                   string literal as NTTP
│   ├── operator+ / - / * / /            Einstein-style broadcast arithmetic
│   ├── contract(a, b)                   Einstein-sum named-axis contraction
│   ├── BroadcastPlan / ContractPlan     plan structs (named operations)
│   ├── concepts::AxisLike               concept on Axis-shaped types
│   ├── concepts::ShapeLike              concept on Shape-like types
│   ├── concepts::KernelBackend          the 15-method backend port
│   ├── literals::operator""_ax          UDL for compile-time labels
│   ├── backend::
│   │   ├── reference::Backend           pure-C++ KernelBackend
│   │   ├── eigen::Backend               Eigen 3.4-backed KernelBackend
│   │   └── webgpu::Backend              Dawn-backed KernelBackend (12 of 15 methods on real GPU)
│   ├── format                           ASCII-box renderer (operator<<)
│   └── mdspan_interop::mdview            mdspan adapter
├── autograd::                          Domain extension — tape-based reverse-mode
│   ├── Variable<T, N>                   compile-time-rank autograd variable
│   ├── DynamicVariable<T>               runtime-rank autograd variable
│   ├── Tape (thread_local)              the reverse-mode tape
│   ├── exp / log / relu / neg           activations with closed-form gradients
│   ├── sin / cos / sqrt                 (Bundle B) trig + sqrt activations
│   ├── operator/                         (Bundle B) quotient-rule autograd op
│   ├── dot(a, b)                        autograd-aware named-axis contract
│   ├── sum_all(v)                       collapse to scalar
│   ├── reduce_along_label(v, label)     (Bundle B) single-axis autograd sum
│   ├── backward(loss)                   reverse-mode tape walk
│   ├── zero_grad(v)                     clear gradients
│   ├── sgd_update(v, lr)                vanilla SGD step
│   └── gradient_check(fn, vars)         finite-difference check
└── tex::                               DrivingAdapter — LaTeX-subset DSL
    ├── Expression                       parsed AST
    ├── operator""_tex                   the headline UDL
    ├── parse(s)                         runtime parse
    ├── to_latex(expr)                   canonical LaTeX renderer
    ├── Evaluator<T>                     bind named tensors → evaluate
    └── EvaluatorF32                     same, over float
```

## Per-namespace pointers

### `tensor::core`

Detailed design: [`../../detailed-design/tensor-core.md`](../../detailed-design/tensor-core.md). Header root: [`include/tensor/core/`](../../../include/tensor/core/). Anchor ADRs: [0002](../../arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md), [0004](../../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md), [0009](../../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [0011](../../arc42/09-decisions/0011-kernel-backend-port-api.md).

The Domain centerpiece. Three tensor types fit a 2-axis design space — compile-time vs runtime rank × labelled vs unlabelled. See [`../how-to/named-tensor-types.md`](../how-to/named-tensor-types.md) for the picker.

### `tensor::autograd`

Detailed design: [`../../detailed-design/tensor-autograd.md`](../../detailed-design/tensor-autograd.md). Header root: [`include/tensor/autograd/`](../../../include/tensor/autograd/). Anchor ADRs: [0007](../../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [0009](../../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).

Tape-based reverse-mode. Bundle B (PR #109) added `sin` / `cos` / `sqrt` / `operator/` / `reduce_along_label` to support trigonometric rotation parameterisations + perspective divide in the MVG demos.

### `tensor::tex`

Detailed design: [`../../detailed-design/tensor-tex.md`](../../detailed-design/tensor-tex.md). Header root: [`include/tensor/tex/`](../../../include/tensor/tex/). Anchor ADR: [0005](../../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).

The `_tex` UDL parses at compile time. `Evaluator<T>` runs the AST at runtime against bound named tensors.

### `tensor::core::backend::*`

Detailed designs: [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md) (the port), `webgpu-element-wise-kernels.md` / `webgpu-gemm-kernel.md` / `webgpu-broadcast-kernels.md` (the three WGSL adapter pieces). Anchor ADRs: [0006](../../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [0010](../../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [0011](../../arc42/09-decisions/0011-kernel-backend-port-api.md), [0012](../../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md), [0016](../../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md).

Three adapters as of 2026-05-14: `reference` (canonical CPU), `eigen` (SIMD + GEMM), `webgpu` (Dawn-backed; 12 of 15 methods on real GPU for `float`).

## Stability promise

Pre-`1.0.0`: no ABI commitments per [ADR-0010 §Decision Outcome point 3](../../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md). The vocabulary above is stable in the citability-discipline sense (every name traces to a math source via §12); the binary layout is not.

## Cross-references

- Python equivalent surface: [`./python-package-overview.md`](./python-package-overview.md).
- API contract per module: [`../../api-contract/`](../../api-contract/).
- Glossary entries for the vocabulary: [`../../arc42/12-glossary/overview.md`](../../arc42/12-glossary/overview.md).
- Source headers: [`include/tensor/`](../../../include/tensor/).
