---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Python package overview — `tensor.*` reference

> **Diátaxis quadrant**: Reference. Information-oriented; one-line summary per public symbol. For the formal contract (introduced-in version + pinning test), see [`../../api-contract/python-public-surface.md`](../../api-contract/python-public-surface.md). For a beginner walkthrough, see [`../tutorials/python-getting-started.md`](../tutorials/python-getting-started.md).

## At a glance

```python
import tensor

# Top-level — wraps tensor::core
tensor.__version__               # str
tensor.hello()                   # → "hello from tensor::core"  (install smoke)
tensor.Axis(label, extent)       # → Axis instance
tensor.DynamicShape(axes_list)   # → DynamicShape; .rank() / .total() / .axes()
tensor.DynamicTensor(shape, values)       # → rank-erased tensor over double
tensor.DynamicTensorF32(shape, values)    # → same, over float
tensor.contract(a, b)            # → Einstein-sum over shared axis labels
tensor.from_numpy(arr, labels)   # → DynamicTensor / F32 from a numpy array
# Phase 6.5 (planned):
tensor.set_backend(name)         # → switch active backend in {"reference","eigen","webgpu"}
tensor.current_backend()         # → str
tensor.list_available_backends() # → list[str]

# tensor.autograd — wraps tensor::autograd
import tensor.autograd as ag
ag.DynamicVariable(t)            # → wraps DynamicTensor with optional grad
ag.DynamicVariableF32(t)         # → same, over float
ag.exp / log / relu / neg(v)     # → activations
ag.sin / cos / sqrt(v)           # → Bundle B activations
ag.dot(a, b)                     # → autograd-aware contract
ag.sum_all(v)                    # → collapse to scalar
ag.reduce_along_label(v, label)  # → Bundle B single-axis autograd sum
ag.backward(loss)                # → reverse-mode walk
ag.zero_grad(v)                  # → clear gradients
ag.sgd_update(v, lr)             # → one SGD step

# tensor.tex — wraps tensor::tex
import tensor.tex as tex
tex.parse(s)                     # → Expression AST
tex.to_latex(expr)               # → canonical LaTeX
tex.Expression()                 # → empty Expression
tex.Evaluator()                  # → bind + evaluate (double)
tex.EvaluatorF32()               # → same, over float
```

## Operators on `DynamicVariable`

`+`, `-`, `*`, `/` — element-wise / Einstein-broadcast (same semantics as `DynamicTensor`); each registers a backward on the `Tape`. The `__truediv__` variant was added in Bundle B (PR #109) so the perspective-divide pattern `(y¹/y³, y²/y³)` in the BA notebook differentiates correctly.

## Numerical agreement guarantee

Every Python entry point produces a result that agrees element-wise with the C++ canonical answer within `1e-12` for `double` / `1e-5` for `float` (the QO-4 envelope in [`../../arc42/10-quality/overview.md`](../../arc42/10-quality/overview.md)). Tests at `python/tests/test_arithmetic.py`, `test_contract_numpy.py`, `test_autograd.py`, `test_autograd_extensions.py`, `test_tex.py` enforce this.

## How to install with a specific backend (Phase 6.5)

```bash
pip install tensor-named-axis           # reference only (default, ~5 MB)
pip install tensor-named-axis[eigen]    # + Eigen SIMD/GEMM (~20 MB total)
pip install tensor-named-axis[webgpu]   # + Dawn-backed WebGPU (~60 MB total)
pip install tensor-named-axis[all]      # all three
```

See [`../how-to/use-set-backend.md`](../how-to/use-set-backend.md) for the runtime switch flow.

## What's NOT public

Symbols starting with `_` (e.g. `_tensor_native`, `_ScalarVariable`) are implementation details; they may change between minor versions. The smoke test `python/tests/test_smoke.py::test_public_surface_is_minimal` pins the public set; this reference is its human-readable face.

## Cross-references

- Formal contract (with introduced-in versions): [`../../api-contract/python-public-surface.md`](../../api-contract/python-public-surface.md).
- C++ equivalent surface: [`./cpp-namespace-overview.md`](./cpp-namespace-overview.md).
- Glossary: [`../../arc42/12-glossary/overview.md`](../../arc42/12-glossary/overview.md).
- Detailed design: [`../../detailed-design/python-sdk-binding-surface.md`](../../detailed-design/python-sdk-binding-surface.md).
