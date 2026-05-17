---
status: Proposed
owner: tensor
last-reviewed: 2026-05-14
---

# ADR-0020: Phase 6.6 — multi-backend runtime dispatch via type-owner-separation

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Proposed**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-14                                                     |
| Deciders  | uyuutosa (maintainer)                                          |
| Consulted | nanobind upstream (discussion #1067 — type-owner pattern)      |
| Informed  | Phase 6.6 contributors; ADR-0019 supersession scope            |

---

## Context and Problem Statement

[ADR-0019](./0019-phase-6-5-runtime-backend-selection-via-extras.md) chose PEP-508 extras + a `set_backend()` runtime rebind to switch between reference / Eigen / WebGPU adapters from Python. The Phase 6.5 implementation ([P6.5.M2](../../impl-plans/2026-05-13_phase-6-5-set-backend.md)) discovered **R-P6.5.5** during M3 wiring: nanobind 2.x's C++ type registry is **process-global** — unlike pybind11, nanobind 2.12 has no `module_local()` flag, so two `_tensor_native_*.so` files in one Python process produce partial bindings on the second `import`. The Phase 6.5 retrospective ([`../../reports/2026-05-14_phase-6-5-set-backend-retrospective.md`](../../reports/2026-05-14_phase-6-5-set-backend-retrospective.md)) shipped the constraint as documented: single-backend-per-process, switch via `TENSOR_BACKEND=<name> python …`.

The R-P6.5.5 lift exploration ([`../../reports/2026-05-14_r-p6-5-5-lift-exploration.md`](../../reports/2026-05-14_r-p6-5-5-lift-exploration.md)) confirms via nanobind upstream discussion #1067 that the canonical fix is the **type-owner-separation pattern**: one module owns the `nb::class_<...>` registrations for the shared types (`Axis`, `DynamicShape`, `DynamicTensor`, autograd `Variable`, `tex::Expression`); per-backend modules import that owner module's bindings and only register their backend-specific kernel functions. nanobind 2.x supports this via `nb::module_::import_("_tensor_types")` from the kernel modules.

This ADR records the architectural choice for Phase 6.6: adopt the type-owner-separation pattern, enabling true single-process multi-backend dispatch and lifting R-P6.5.5.

---

## Decision Drivers

In priority order:

- **DD-1 — close the swappable-backends teaching surface gap**. The original Phase 6.5 framing in ADR-0019 §DD-2 envisioned `python/notebooks/05_swappable-backends.ipynb` as "now switch to WebGPU and measure the speedup" in one process. R-P6.5.5 forced the workaround `TENSOR_BACKEND=webgpu python …`, which is a documentation-grade fix but a teaching-surface regression. Phase 6.6 closes the gap.
- **DD-2 — minimise architectural blast radius**. The fix should change *how the C++ side is sliced into modules*, not the public Python API (`tensor.set_backend()` / `tensor.current_backend()` / `tensor.list_available_backends()`). The existing notebook + how-to docs must not break.
- **DD-3 — preserve ADR-0011 KernelBackend port boundary**. The C++ `KernelBackend` interface stays unchanged; only its Python-side packaging shifts. The C++ tutorials (`tutorials/08_swappable-backends.ipynb`) are unaffected.
- **DD-4 — avoid switching binding tool**. pybind11's `py::module_local()` would solve R-P6.5.5 trivially, but ADR-0018 chose nanobind for clear architectural reasons (lower overhead, modern C++20, fewer macros). Switching binding tool to revert one decision is a regression on the decision discipline.

---

## Considered Options

1. **Status quo (Phase 6.5 ship state)** — keep single-backend-per-process; `TENSOR_BACKEND` env-var workaround documented in §6 of the swappable-backends notebook + how-to.
2. **Type-owner-separation pattern (nanobind upstream-recommended)** — split `python/src/_tensor_native.cpp` into:
   - `_tensor_types.cpp` → built once as `_tensor_types.so` (no backend specialisation); owns all `nb::class_<...>` registrations for shared C++ types.
   - `_tensor_native_{reference,eigen,webgpu}.cpp` → each imports `_tensor_types` via `nb::module_::import_("tensor._tensor_types")` and registers only the kernel-call `m.def()`s for that backend.
3. **Switch from nanobind to pybind11** — use `py::module_local()` to give each backend module its own type registry; standard pybind11 idiom.
4. **Per-backend Python subpackage** — ship `tensor.reference`, `tensor.eigen`, `tensor.webgpu` as fully independent Python modules with their own type universes; user explicitly imports `from tensor.eigen import contract`. No `set_backend()` indirection.

---

## Decision Outcome

**Chosen option: 2 — type-owner-separation pattern.**

This matches nanobind upstream's recommended pattern for the exact scenario (multiple modules sharing C++ types). It closes R-P6.5.5 within the existing architecture: ADR-0011's KernelBackend port boundary is untouched, ADR-0018's nanobind choice is preserved, and the public Python API from ADR-0019 / Phase 6.5 stays bit-identical from the user's perspective. The cost is a modest C++ refactor (~3-5 days per [`../../reports/2026-05-14_r-p6-5-5-lift-exploration.md`](../../reports/2026-05-14_r-p6-5-5-lift-exploration.md) §3) and one new ABI compatibility constraint between `_tensor_types.so` and each `_tensor_native_<backend>.so` (they must be built against the same `tensor::core` headers — already true in our monorepo build).

Option 1 (status quo) is correct for `0.3.0` but fails DD-1 going forward. Option 3 (switch to pybind11) fails DD-4. Option 4 (per-backend subpackage) fails DD-2 — every existing notebook + tutorial + Python-side how-to would need rewriting.

### Y-statement summary

> In the context of **R-P6.5.5 — nanobind 2.x's process-global C++ type registry blocking single-process multi-backend switching**, facing **the educational-frame regression of forcing `TENSOR_BACKEND=…` restart between backend comparisons**, we decided for **the type-owner-separation pattern (one `_tensor_types.so` owns nb::class_ registrations; per-backend modules import-and-use)** to achieve **true `tensor.set_backend("webgpu")` within one Python process, no API surface change**, accepting **a 3-5 day C++ refactor + a new ABI co-build constraint between the types module and the backend modules**.

### Supersession scope

This ADR **partially supersedes** [ADR-0019](./0019-phase-6-5-runtime-backend-selection-via-extras.md) on the narrow question of "how `set_backend()` works internally". ADR-0019's primary decision (PEP-508 extras vs fat wheel) is **unchanged and still Accepted**. The R-P6.5.5 workaround documented in ADR-0019 §Consequences-Negative ("`set_backend()` is a runtime-detection API, not a compile-time choice … missing backends raise on `set_backend()`-call, not on `import tensor`") gets a stronger guarantee under ADR-0020: missing backends still raise as before, but *installed* backends become switchable in-process.

---

## Pros and Cons of the Options

### Option 1: Status quo (Phase 6.5 ship state)

- Pros:
  - Zero implementation cost.
  - Already documented + tested.
- Cons:
  - **`TENSOR_BACKEND=…` restart between backend comparisons** — fails DD-1.
  - The swappable-backends notebook's "now switch to WebGPU" cell can't actually demonstrate switching; it can only show error messages or the env-var workaround text.

### Option 2: Type-owner-separation pattern (chosen)

- Pros:
  - **Single-process multi-backend dispatch** — `tensor.set_backend("eigen")` then `set_backend("webgpu")` in the same script Just Works.
  - **No public-API change** — `pyproject.toml` extras, `set_backend()` / `current_backend()` / `list_available_backends()` surface, error messages — all stay identical.
  - **Nanobind upstream-blessed pattern** — discussion #1067 documents it as the canonical fix.
  - **ABI co-build constraint is already met** — monorepo CI builds all four `.so` files from the same `tensor::core` headers at every tag cut.
- Cons:
  - **C++ refactor effort** — `python/src/_tensor_native.cpp` (~568 lines) splits into ~250-line type-owner + ~3 × ~150-line kernel modules. 3-5 days of focused work.
  - **CMake gets one more target** — `_tensor_types` extension module joins the existing `_tensor_native_{reference,eigen,webgpu}` set; the build matrix grows by 1 ABI variant per `(OS × arch × CPython)` slot.
  - **One new failure mode**: if `_tensor_types.so` is missing (corrupt install), `import tensor` fails with a less-helpful error than today's "no backend installed". Mitigate via `python/tensor/__init__.py`'s `_load_types_module()` raising a specific `RuntimeError` pointing at `pip install --force-reinstall tensor-named-axis`.

### Option 3: Switch from nanobind to pybind11

- Pros:
  - `py::module_local()` solves R-P6.5.5 in one line.
  - pybind11 is older + more idiom-stable.
- Cons:
  - **Reverts ADR-0018**. nanobind was chosen for lower overhead, fewer macros, modern C++20 idioms.
  - The entire `python/src/` rewrites against a different binding API surface — far larger than the type-owner-separation refactor.
  - Two PR cycles to re-cross-validate the existing `python/tests/test_*.py` suite under pybind11.

### Option 4: Per-backend Python subpackage

- Pros:
  - Each backend's module is fully self-contained; no shared-type ABI concern.
  - PyPI-style "namespace-per-feature" idiom.
- Cons:
  - **Every existing user import breaks** — `from tensor import contract` becomes `from tensor.eigen import contract`.
  - The Diátaxis how-to + 6 Python notebooks all need rewriting.
  - Fails DD-2 (architectural blast radius).

---

## Consequences

### Positive

- **R-P6.5.5 lifted**. The arc42 §11 §5 risk register entry transitions from "Mitigated via env-var workaround" to "Resolved by ADR-0020 / Phase 6.6".
- **Swappable-backends teaching surface restored**. `python/notebooks/05_swappable-backends.ipynb` §6's "Known limitation" section becomes "Historical — see ADR-0020"; the notebook can demonstrate live in-process switching.
- **Bench-time backend comparison becomes ergonomic**. `bench/bench_python_backends.py` can loop over `[reference, eigen, webgpu]` and produce a single comparison plot in one process — no subprocess orchestration.

### Negative

- **One new ABI co-build constraint**. `_tensor_types.so` and each `_tensor_native_<backend>.so` must be built against the same `tensor::core` headers + the same nanobind ABI. Solved structurally by the monorepo + cibuildwheel matrix building all four together at every tag; documented in [`../detailed-design/`](../detailed-design/) as a new "build-time ABI invariant" subsection.
- **CMake adds one extension-module target**. `python/CMakeLists.txt` grows by ~30 lines. Build time per slot increases ~10% (one extra small `.so` compile).
- **PyPI per-extras-distribution size grows ~5 MB** (the type-owner module ships with each `tensor-named-axis-*` companion to ensure a usable install even if the user pins versions oddly). Mitigation: investigate whether the type-owner module can ship as a dedicated `tensor-named-axis-types` distribution that all backend extras depend on — defer to M2 of Phase 6.6.

### Neutral

- **Python API surface bit-identical**. No `pyproject.toml` change beyond bumping internal version pins. No notebook diff except the §6 "Known limitation" section update.
- **C++ Domain unchanged**. ADR-0011 KernelBackend port is unaffected. `tutorials/08_swappable-backends.ipynb` (the C++ side) is unaffected.

### Follow-ups

- [ ] Phase 6.6 impl-plan: [`../../impl-plans/2026-05-14_phase-6-6-r-p6-5-5-lift.md`](../../impl-plans/2026-05-14_phase-6-6-r-p6-5-5-lift.md) with milestones M1 (types-owner split + parity), M2 (kernel modules import-and-use), M3 (set_backend rebind), M4 (`0.4.0` release).
- [ ] Update [`../11-risks/overview.md`](../11-risks/overview.md): R-P6.5.5 status → "Resolved by ADR-0020 / Phase 6.6".
- [ ] Update [`python/notebooks/05_swappable-backends.ipynb`](../../../python/notebooks/05_swappable-backends.ipynb) §6 once Phase 6.6 ships.
- [ ] Investigate: should the type-owner module ship as a dedicated `tensor-named-axis-types` distribution that all extras depend on, vs being duplicated in each? Decide in Phase 6.6 M2 review.

---

## Compliance / Validation

- **Verification**: `python/tests/test_set_backend.py` gains a new case `test_in_process_switch_between_installed_backends` that runs `set_backend("reference")` → `contract(...)` → `set_backend("eigen")` → `contract(...)` in one process and asserts both calls return numerically agreeing results within tolerance.
- **Frequency**: every Phase 6.6 milestone PR + every release.

---

## More Information

### Related ADRs

- Partially supersedes: [ADR-0019](./0019-phase-6-5-runtime-backend-selection-via-extras.md) — on the internal `set_backend` rebind point. The PEP-508 extras choice is preserved.
- Architectural baseline: [ADR-0018](./0018-phase-6-python-sdk-entry-via-nanobind.md) — Python SDK entry via nanobind. Confirms nanobind choice; option 3 (pybind11 switch) is rejected on this basis.
- Port boundary: [ADR-0011](./0011-kernel-backend-port-api.md) — KernelBackend interface; unaffected by Phase 6.6.
- Positioning: [ADR-0010](./0010-refine-positioning-to-educational-first-production-capable.md) — educational-first framing; DD-1 of this ADR cashes the educational-frame motivation.

### References

- nanobind discussion #1067 (type-owner pattern): <https://github.com/wjakob/nanobind/discussions/1067>
- nanobind 2.x documentation §10 (type registration): <https://nanobind.readthedocs.io/en/latest/exchanging.html>
- R-P6.5.5 lift exploration report: [`../../reports/2026-05-14_r-p6-5-5-lift-exploration.md`](../../reports/2026-05-14_r-p6-5-5-lift-exploration.md)
- Landscape recheck + Phase 7+ roadmap (Phase 7b sequencing motivation): [`../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](../../reports/2026-05-14_landscape-recheck-and-phase-7-roadmap.md)
- Phase 6.5 retrospective (where R-P6.5.5 was first documented): [`../../reports/2026-05-14_phase-6-5-set-backend-retrospective.md`](../../reports/2026-05-14_phase-6-5-set-backend-retrospective.md)
