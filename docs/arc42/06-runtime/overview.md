---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Runtime View (arc42 §6)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §6 (Runtime View)                                        |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §6: this file shows **how the building blocks interact at runtime** for the project's most important scenarios. Not every scenario lives here — only those that exercise multiple containers in a way that benefits from being walked through.

## Scenario 1 — `c = a + b` via the runtime broadcast path

**Trigger.** A learner writes `auto c = a + b;` in a tutorial cell, where `a` and `b` are `DynamicTensor<double>` instances with axis labels `i` and `j` respectively.

**Walk-through.**

```text
1. operator+(a, b)                                 (tensor::core::ops)
2.  → broadcast_shapes(a.shape(), b.shape())       (tensor::core::broadcast)
3.    returns BroadcastPlan{result, a_source, b_source}
4.  → reference::Backend::broadcast_add(a, b, plan)  (selected adapter — KernelBackend port)
5.    iterates result-flat-index 0..result_size:
6.       a_flat = project_index(i, plan.a_source)
7.       b_flat = project_index(i, plan.b_source)
8.       out[i] = a[a_flat] + b[b_flat]
9. returns DynamicTensor<double>{plan.result, out}
```

**Why this is interesting.** The Domain (`tensor::core::ops` + `broadcast.hpp`) computes the *plan* without knowing which backend will execute. The selected adapter (`reference::Backend` or `eigen::Backend` or `webgpu::Backend`) consumes the plan and produces the output. Swapping `-DTENSOR_KERNEL_BACKEND=eigen` at configure time changes step 4 without touching steps 1–3.

## Scenario 2 — `_tex` formula evaluated end-to-end

**Trigger.** A user writes `auto expr = R"(c_{ij} = a_i b_j)"_tex; evaluator.bind("a", a).bind("b", b); auto c = evaluator.evaluate(expr);`.

**Walk-through.**

```text
1. operator""_tex(string_view src)                 (tensor::tex::operator""_tex)
2.  → tensor::tex::parse(src)
3.    produces Expression AST: Equation{LHS=IndexedVar{"c", {"i","j"}},
4.                                       RHS=BinOp{*, IndexedVar{"a", {"i"}},
5.                                                    IndexedVar{"b", {"j"}}}}
6. evaluator.bind("a", a)                          (records DynamicTensor binding)
7. evaluator.bind("b", b)
8. evaluator.evaluate(expr)                        (tensor::tex::Evaluator)
9.  → traverse RHS:
10.     visit BinOp{*}: → a_i * b_j
11.       → tensor::core::broadcast_shapes(a_i, b_j) → outer product plan
12.       → reference::Backend::broadcast_mul(a, b, plan)
13. returns DynamicTensor<double> with shape {Axis{"i", N}, Axis{"j", M}}
```

**Why this is interesting.** The TeX surface (DrivingAdapter) consumes `tensor::core` types but never the other way around. The same operator+ implementation runs whether the call originates from `operator+(a, b)` (Scenario 1) or from `parse + evaluate` (Scenario 2). This is the [§4 G-3 goal](../01-introduction-and-goals/overview.md): "the formula is the program".

## Scenario 3 — Autograd backward pass

**Trigger.** A user computes `auto loss = (W * x + b - y).sum_all(); loss.backward();`.

**Walk-through.**

```text
1. Forward pass — every operator pushes a closure onto tensor::autograd::Tape::current():
   - W.mul(x)        → registers closure that propagates grad_out → (grad_W += grad_out * x.T,
                                                                     grad_x += W.T * grad_out)
   - (W*x).add(b)    → registers closure that propagates grad_out → (grad_(W*x) += grad_out,
                                                                     grad_b += unbroadcast(grad_out, …))
   - (...).sub(y)    → similar
   - sum_all()       → registers closure that propagates scalar grad → broadcast back to result shape

2. loss.backward(/* grad = 1.0 */):
   - Walks the tape in reverse.
   - Invokes each registered closure with the appropriate grad_out.
   - Each closure updates the GradAccum for its input Variables.

3. After backward, W.grad(), x.grad(), b.grad() hold accumulated gradients.
```

**Why this is interesting.** The Tape is `thread_local std::vector<std::function<void()>>` — no special infrastructure beyond the standard library. Gradients flow through the *same* port the forward pass used: `unbroadcast` lives in `tensor::core`, called both by the forward broadcast and by the backward closure to undo the broadcast. Per [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), autograd is the Domain *extended*, not a sibling.

## Scenario 4 — Backend swap at configure time

**Trigger.** A learner runs `cmake --preset=default -DTENSOR_KERNEL_BACKEND=eigen` after the default `reference` build.

**Walk-through.**

```text
1. CMakeLists.txt sees the cache variable change.
2. TENSOR_KERNEL_BACKEND=eigen triggers:
   - find_package(Eigen3 CONFIG REQUIRED)
   - target_link_libraries(tensor INTERFACE Eigen3::Eigen)
   - target_compile_definitions(tensor INTERFACE TENSOR_HAS_EIGEN=1)
3. Headers under include/tensor/core/backend/eigen.hpp are now visible.
4. The user's source includes #include <tensor/core/backend/eigen.hpp> and instantiates:
   tensor::core::backend::eigen::Backend backend;
5. Compile-time concept check: static_assert(KernelBackend<eigen::Backend>) — confirms the
   adapter implements all 15 methods.
6. Runtime: backend.add(a, b) uses Eigen::Map<Eigen::ArrayXd> + element-wise + as a SIMD path
   for double; falls through to reference for other types.
```

**Why this is interesting.** Step 5 — `static_assert(KernelBackend<eigen::Backend>)` — is the compile-time guarantee that the swap is safe. If a future contributor adds a `Backend` that fails to implement one of the 15 methods, the build fails with the constraint that was violated; the cross-validation test then catches numerical disagreements within tolerance.

## Scenario 5 — Python `import tensor` initialisation

**Trigger.** A user runs `import tensor` in a Python REPL or notebook after `pip install tensor-named-axis`.

**Walk-through.**

```text
1. Python's import system locates the `tensor/` directory inside the installed wheel.
2. `tensor/__init__.py` executes:
   - `from ._tensor_native import __version__, autograd, tex, Axis, DynamicShape,
      DynamicTensor, DynamicTensorF32, contract, from_numpy`
     → CPython loads the nanobind extension module `_tensor_native.so`.
   - nanobind runs the module init code in `python/src/_tensor_native.cpp`:
     * Registers `Axis` (`nb::class_` + placement-new `__init__` form per the
       ADR-0018 R-P3-style boundary papercut workaround).
     * Registers `DynamicShape` / `DynamicTensor<double>` / `DynamicTensorF32`
       (`DynamicTensor<float>`) — same surface, two type-erased Python classes.
     * Registers `contract` / `from_numpy` free functions.
     * Builds the `autograd` submodule (DynamicVariable + ops + activations).
     * Builds the `tex` submodule (parse / to_latex / Evaluator).
3. `tensor/__init__.py` runs two `sys.modules` registrations:
     sys.modules[__name__ + ".autograd"] = autograd
     sys.modules[__name__ + ".tex"] = tex
   — needed because nanobind submodules are exposed as attributes of the parent
   module, not as Python submodules. Without this, `import tensor.autograd as ag`
   would raise `ModuleNotFoundError` (caught the hard way in PR #104).
4. The user can now write:
     >>> import tensor
     >>> import tensor.autograd as ag    # works thanks to step 3
     >>> import tensor.tex as tex        # works thanks to step 3
```

**Phase 6.5 extension.** Once `set_backend()` ships per [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md), step 2 becomes a probe: `tensor/__init__.py` imports whichever of `_tensor_native_{reference,eigen,webgpu}` are present (PEP-420 namespace package), the first successful import wins as default, and `tensor.set_backend(name)` rebinds the top-level exports against the matching module. Missing backends raise on `set_backend()`-call, not on `import tensor`.

**Why this is interesting.** Step 3 (sys.modules registration) is the nanobind ↔ Python-import-system boundary papercut that the [Phase 6 retrospective](../../reports/2026-05-13_phase-6-python-sdk-retrospective.md) flags as one of four conventions worth recording. Documenting it in §6 keeps it discoverable for the next nanobind binding work.

## Scenario 6 — Phase 6.5 `tensor.set_backend("eigen")` runtime switch

**Trigger.** A learner has `pip install tensor-named-axis[eigen]` on a clean venv and calls `tensor.set_backend("eigen")` from a notebook cell, intending to compare Eigen SIMD vs the default reference adapter on a 1000×1000 matmul.

**Walk-through (forward-doc; lands in `0.3.0` per [ADR-0019](../09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)).**

```text
1. The wheel install put two `.so` files under tensor/:
     site-packages/tensor/_tensor_native_reference.so   (from tensor-named-axis)
     site-packages/tensor/_tensor_native_eigen.so       (from tensor-named-axis-eigen)
2. `import tensor` runs `python/tensor/__init__.py`:
   - For each backend in {reference, eigen, webgpu}:
       try:
           mod = importlib.import_module(f"._tensor_native_{backend}", __name__)
           AVAILABLE[backend] = mod
       except ImportError:
           pass
   - DEFAULT = first available backend (reference is always available).
3. `tensor.list_available_backends()` returns ['reference', 'eigen'].
4. `tensor.current_backend()` returns 'reference' (the default).
5. `tensor.set_backend("eigen")`:
   - Look up AVAILABLE["eigen"] → mod object.
   - Rebind top-level exports:
       tensor.DynamicTensor = mod.DynamicTensor
       tensor.contract      = mod.contract
       tensor.from_numpy    = mod.from_numpy
       tensor.autograd      = mod.autograd
       tensor.tex           = mod.tex
       sys.modules["tensor.autograd"] = mod.autograd  # (the sys.modules trick from Scenario 5)
       sys.modules["tensor.tex"]      = mod.tex
   - Update `_CURRENT_BACKEND = "eigen"`.
6. Subsequent `tensor.DynamicTensor(...)` constructions resolve to `mod.DynamicTensor`
   — the new tensors carry an Eigen-backed C++ implementation.
7. Tensors constructed *before* step 6 continue to use whatever backend was active at
   their construction time. Mid-computation rebind is undefined; document this clearly
   in the how-to (see `../../user-manual/how-to/use-set-backend.md`).
```

**Missing-backend path**:

```text
1. tensor.set_backend("webgpu") when webgpu is not in AVAILABLE.
2. Raises:
     RuntimeError: webgpu backend is not installed.
     Install with:  pip install tensor-named-axis[webgpu]
     Or install all backends:  pip install tensor-named-axis[all]
     Currently available: ['reference', 'eigen']
3. No silent fallback to reference. Explicit-request semantics: fail loud.
```

**Why this is interesting.** Steps 5–6 are runtime rebinding of `tensor.DynamicTensor` — looking up `tensor.DynamicTensor` after the rebind gives a different C++ type than before. This is fine because the wrapper class on the Python side is identical across backends (only the underlying `_tensor_native_*.so` differs), but it means any user-side `assert isinstance(x, tensor.DynamicTensor)` could fail if `x` was constructed before the rebind. The how-to (`use-set-backend.md`) documents the constraint.

## Scenario 7 — Notebook execute-then-deploy pipeline (docs-system runtime)

**Trigger.** A maintainer merges PR #117 (the executed-notebooks PR). `deploy-book.yml` fires on the merge-to-develop push.

**Walk-through.** This is the docs-system's own runtime — included so contributors understand the pipeline they're editing.

```text
1. `.github/workflows/deploy-book.yml` triggers on push to develop:
   - actions/checkout@v4 pulls the develop HEAD.
   - actions/setup-python@v5 installs CPython 3.11.
   - pip install jupyter-book>=0.15,<1.
2. bash book/stage.sh:
   - ln -sfn ../tutorials book/tutorials
   - ln -sfn ../python    book/python
   - ln -sfn ../docs      book/docs
   (Per PR #116: Sphinx's source root is book/, so external content needs
    to be reachable from inside book/.)
3. jupyter-book build book:
   - Sphinx reads book/_config.yml and book/_toc.yml.
   - For each chapter in _toc.yml, Sphinx reads the .ipynb/.md/.rst file
     and produces HTML under book/_build/html/.
   - `execute_notebooks: "off"` in _config.yml — Sphinx renders the
     EXISTING outputs in the committed JSON. (If commit was un-executed,
     output cells are empty. The PR #118 gate prevents that.)
   - The plotly `<script src="https://cdn.plot.ly/plotly-3.0.1.min.js">`
     reference in committed-executed cells loads at browser-render time.
     The MathJax v3 `<script defer src="...mathjax@3/...">` from the
     book theme typesets $…$ math. (PR #120 ensured the two don't
     collide.)
4. actions/upload-pages-artifact@v3 uploads book/_build/html/.
5. The `deploy` job (after `build`):
   - actions/deploy-pages@v4 pushes to the gh-pages target.
   - GitHub Pages serves the artifact at https://uyuutosa.github.io/tensor/.
   - Cache TTL: max-age=600 (10 min).
```

**Why this is interesting.** The four PRs that closed the publish gap (#116 stage.sh, #117 executed-notebooks, #118 gate, #120 MathJax) each touched a different layer of this pipeline. Anchoring the layers here keeps the next docs-system regression from hiding in the gaps between layers.

## Why these seven scenarios

The §1 §G-1..§G-9 goals each show up here:

- Scenario 1 → G-1 (named axes), G-2 (modern C++).
- Scenario 2 → G-3 (*the formula is the program*).
- Scenario 3 → G-4 (end-to-end teaching arc), G-2.
- Scenario 4 → G-1 (Hexagonal payoff), G-5 (zero-friction install / configure).
- Scenario 5 → G-9 (Python is a first-class entry to the same Domain).
- Scenario 6 → G-9 (Phase 6.5 forward — runtime backend selection).
- Scenario 7 → G-4 + G-6 + G-8 (the docs-system pipeline that delivers all of the above).

## Cross-references

- §5 building blocks the scenarios traverse: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- detailed-design backing each scenario: [`../../detailed-design/tensor-core.md`](../../detailed-design/tensor-core.md)
- ADRs that shaped the scenarios' design: [ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0011](../09-decisions/0011-kernel-backend-port-api.md).
