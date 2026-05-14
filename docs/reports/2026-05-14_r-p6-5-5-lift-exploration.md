---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# R-P6.5.5 lift exploration — nanobind multi-module type registry

| Metadata        | Value                                                                                  |
| --------------- | -------------------------------------------------------------------------------------- |
| Status          | Stable                                                                                 |
| Type            | Layer B — research / spike report (dated, append-only)                                 |
| Owner           | uyuutosa                                                                               |
| Anchor risk     | [arc42 §11 R-P6.5.5](../arc42/11-risks/overview.md)                                    |
| Trigger         | Phase 6.5 retrospective deferred item — explore whether the single-backend-per-process constraint can be lifted |
| Predecessor     | [`./2026-05-14_phase-6-5-set-backend-retrospective.md`](./2026-05-14_phase-6-5-set-backend-retrospective.md) |

## The constraint, restated

Phase 6.5 ([ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)) originally assumed three `_tensor_native_*.so` files could coexist in a single Python process under the `tensor/` PEP-420 namespace package, with `set_backend()` switching between them at runtime. M3 implementation surfaced the technical reality:

```
RuntimeWarning: nanobind: type 'Axis' was already registered!
AttributeError: module 'tensor._tensor_native_eigen' has no attribute 'Axis'
```

nanobind 2.x's type registry is **process-global per C++ type**. The second module's `nb::class_<Axis>(...)` collision is detected, the registration is silently rejected (with a warning), and the second module's `.Axis` attribute ends up unset. Subsequent `set_backend("eigen")` → `mod.Axis` lookup → AttributeError.

The recovery shipped in PR #124 + #131: **single-backend-per-process** semantics with `TENSOR_BACKEND` env-var workaround for cross-process switching. The constraint is honest and the user-facing API works; what's lost is the original "switch backends in one REPL session" UX from ADR-0019's first draft.

## Why nanobind doesn't have `module_local`

pybind11 ships `py::module_local()` precisely for this case: each module gets its own type registry slice; the same C++ type can be registered N times in N modules, and downcasts work per-module. nanobind 2.12 does NOT have this; the maintainer's stated design ([discussion #1067](https://github.com/wjakob/nanobind/discussions/1067)) is:

> "Library A exposes `A::obj` via `nb::class_<A::obj>(ma, "obj")`. Library B receives or returns `A::obj` without re-registering it. This should just work fine."

In other words: **one module owns the type registration; other modules use the type without re-registering**. The nanobind maintainer treats this as the natural design, not a missing feature.

This reframes R-P6.5.5: the constraint isn't "nanobind is missing `module_local`"; it's "the original Phase 6.5 design had every module re-register every type, which nanobind's design prevents on purpose".

## The path forward — type-owner separation

The fix isn't a nanobind workaround; it's a binding-layer refactor:

1. **One module owns the type registrations.** Call it `_tensor_native_types.so` (or `_tensor_native_core.so`). It registers `Axis`, `DynamicShape`, `DynamicTensor`, `DynamicTensorF32`, `DynamicVariable`, `Expression`, `Evaluator` — every public C++ type the Python adapter exposes. **It exports no backend-specific kernel function.**
2. **Per-backend modules import the types from the owner module.** `_tensor_native_reference.so`, `_tensor_native_eigen.so`, `_tensor_native_webgpu.so` each does `nb::module_::import_("tensor._tensor_native_types")` at module-init time, then registers ONLY its backend-specific kernel functions (`contract`, `from_numpy`, autograd ops, tex Evaluator) that operate on the shared types.
3. **Python adapter rebinds the kernel surface on `set_backend()`.** The types stay constant; the operations rebind to the active backend's free functions. `DynamicTensor + DynamicTensor` dispatches via Python `__add__` which calls the active backend's `add` free function. Concretely:

   ```python
   # tensor/__init__.py (sketch)
   from ._tensor_native_types import Axis, DynamicShape, DynamicTensor, ...
   
   _AVAILABLE = {}
   for name in ("reference", "eigen", "webgpu"):
       try:
           _AVAILABLE[name] = importlib.import_module(f"._tensor_native_{name}", __name__)
       except ImportError:
           pass
   
   _current = _AVAILABLE.get("reference") or next(iter(_AVAILABLE.values()))
   
   def set_backend(name):
       global _current, contract, from_numpy, autograd, tex
       _current = _AVAILABLE[name]
       contract = _current.contract
       from_numpy = _current.from_numpy
       autograd = _current.autograd
       tex = _current.tex
   ```

4. **The `+` / `*` / `/` arithmetic operators** become slightly tricky — they're class methods (`DynamicTensor.__add__`), defined on the type which lives in the *owner* module. The owner module's class definition would call out to a backend dispatcher (e.g., a `tensor::core::active_backend()` runtime global), which the per-backend modules set on import. This is the substantial C++ refactor.

This is roughly the **GraphQL-style "frontend types + backend-specific resolvers"** pattern, applied to a tensor library.

## Effort estimate

| Component | Effort | Reason |
| --------- | ------ | ------ |
| Split `python/src/_tensor_native.cpp` into types + 3 backend modules | M-L (1-2 days) | Mechanical but touches every `nb::class_` and every `m.def`; tests need updating; bindings now span 4 sources. |
| C++ runtime-active-backend dispatcher under `tensor::core` | L (1-2 days) | Currently the `KernelBackend` adapter is a configure-time choice via `static_assert(KernelBackend<Backend>)`. Runtime selection means a vtable or function-pointer table per `DynamicTensor` operation. Affects performance — the indirection cost must be measured. |
| `pyproject.toml` for the new types module | S (1 hr) | New base distribution `tensor-named-axis-types` (or rename the base). |
| Python `set_backend()` semantics refactor | S (2 hr) | Switch from rebind-module to rebind-function-pointer pattern. |
| Tests | M (4 hr) | New parity tests across all 3 backends in the SAME process. |
| Documentation | S-M | ADR-0020 superseding ADR-0019 §G; retrospective; design-guide update. |

Total estimate: **3–5 days** of dedicated work. A Phase 6.6 milestone.

## Why this exploration didn't ship as code

The path is clear but the implementation is substantial enough that it needs:

1. **A new ADR (ADR-0020)** to ratify the type-owner separation pattern + supersede ADR-0019's "PEP-420 + co-load + rebind" assumption.
2. **A perf measurement** to confirm the runtime-dispatch indirection isn't a regression beyond the QF-1 envelope.
3. **The 0.2.0 release** to ship first so the existing Phase 6 surface is published before being refactored.

Each of those is its own slice. A 2-hour POC would either build a toy verification (not load-bearing) or stub out the real work (not useful). Better to record the path here and pick it up as a planned phase.

## What this report decides

1. **R-P6.5.5 is liftable**, not a hard nanobind limitation. The fix is a binding-layer refactor following nanobind's intended design.
2. **The fix becomes Phase 6.6** — a dedicated milestone with its own impl-plan, ADR, and 3-5 day effort budget.
3. **Phase 6.5 ships as-is** with the documented `TENSOR_BACKEND` env-var workaround. No retroactive change to PR #123 / #124 / #131 / the Phase 6.5 retrospective.
4. **The arc42 §11 R-P6.5.5 row** stays 🟡 Medium until Phase 6.6 lands — the constraint is real, the workaround is documented, the lift path is known.

## What needs to happen before Phase 6.6 can start

- `0.2.0` tag ships (PR #115).
- Maintainer decides Phase 6.6 priority vs other Phase 7+ candidates.
- A new impl-plan: `docs/impl-plans/YYYY-MM-DD_phase-6-6-multi-backend-runtime-dispatch.md`.
- A new ADR: `docs/arc42/09-decisions/0020-multi-backend-runtime-dispatch-via-type-owner-module.md` (or similar), explicitly superseding ADR-0019 §"Decision Outcome" point about `set_backend()` runtime rebinding.

## References

- nanobind discussion #1067 — "Interop between 2 nanobind modules" — establishes the type-owner-separation pattern.
- [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) — the Phase 6.5 packaging decision this refactor would refine.
- [arc42 §11 R-P6.5.5](../arc42/11-risks/overview.md) — the risk row this exploration informs.
- [Phase 6.5 retrospective](./2026-05-14_phase-6-5-set-backend-retrospective.md) — the deferral that motivated this report.
- pybind11's `py::module_local` documentation: <https://pybind11.readthedocs.io/en/stable/advanced/classes.html#module-local-class-bindings> — the analogous feature in the sibling library, which this report explicitly does NOT propose adopting (nanobind's design choice is different).
