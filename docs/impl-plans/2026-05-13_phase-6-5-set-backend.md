---
status: Active
owner: tensor
date: 2026-05-13
type: Layer B implementation plan
---

# Phase 6.5 — runtime backend selection via PEP-508 extras (2026-05-13)

> Builds on [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md). Closes the `set_backend()` deferral from the Phase 6 retrospective. Target exit: `0.3.0` tag.

| Metadata     | Value                                                                            |
| ------------ | -------------------------------------------------------------------------------- |
| Layer        | B (volatile working material — dated, append-only)                              |
| Predecessor  | [`2026-05-12_phase-6-python-sdk.md`](./2026-05-12_phase-6-python-sdk.md) (Phase 6 plan, closed) |
| Anchor ADR   | [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) |
| Target window | 2026-05-14 → 2026-06-15 (~one month; four milestones, ~one PR each)             |
| Exit         | `0.3.0` tag — `pip install tensor-named-axis[eigen]` / `[webgpu]` / `[all]` + `tensor.set_backend()` |

---

## Milestones

The order is dependency-driven; each milestone exits when its tests pass on Linux + macOS + Windows (where the backend is available — Eigen everywhere; WebGPU on linux + macOS arm64 only for the M3 cut, Windows WebGPU is a P6.5 follow-up).

### P6.5.M1 — Multi-backend `_tensor_native_*.so` build pipeline

**Goal**: prove that scikit-build-core + nanobind can produce three differently-suffixed extension modules from a single `pyproject.toml` invocation, parameterised by a build-time environment variable.

**Deliverables**:

- `python/CMakeLists.txt` reads `TENSOR_KERNEL_BACKEND` env var; the produced extension module is named `_tensor_native_{reference,eigen,webgpu}` correspondingly.
- `pyproject.toml` `[tool.scikit-build]` gets a `cmake.define` map sourced from the build env so `CIBW_ENVIRONMENT='TENSOR_KERNEL_BACKEND=eigen'` reaches CMake.
- `python/tensor/__init__.py` learns to import all `_tensor_native_*` modules that are present, and falls back to `_tensor_native` (the legacy unsuffixed name) for the reference adapter. This keeps the existing `0.2.0` wheels importable.
- `python/tests/test_multi_backend_build.py` — verifies that a build with `TENSOR_KERNEL_BACKEND=eigen` produces `_tensor_native_eigen.so` and `import tensor` still works.

**Exit criteria**: three local `pip install -e .` invocations with `TENSOR_KERNEL_BACKEND={reference,eigen,webgpu}` produce three coexistent `.so` files; `python -c "import tensor; print(tensor.list_available_backends())"` lists exactly the ones installed.

**Effort**: M (≤ 1 day).

### P6.5.M2 — `tensor.set_backend()` Python surface

**Goal**: runtime backend selection works from Python, with clear error messages when an uninstalled backend is requested.

**Deliverables**:

- `tensor.set_backend(name: Literal["reference", "eigen", "webgpu"]) -> None`. Imports the matching `_tensor_native_<name>` and rebinds `tensor.DynamicTensor`, `tensor.contract`, `tensor.from_numpy`, `tensor.autograd`, `tensor.tex` to that module's exports.
- `tensor.current_backend() -> str` and `tensor.list_available_backends() -> list[str]`.
- `tensor.set_backend("webgpu")` on a non-`[webgpu]` install raises:
  ```
  RuntimeError: webgpu backend is not installed.
  Install with:  pip install tensor-named-axis[webgpu]
  Or install all backends:  pip install tensor-named-axis[all]
  Currently available: ['reference']
  ```
- `python/tests/test_set_backend.py` — 6 cases: default is `reference`; `set_backend("reference")` is a no-op; `set_backend("eigen")` succeeds when installed; `set_backend("webgpu")` raises with the expected message when not installed; round-trip `set_backend` + arithmetic + parity vs reference within `1e-12` (`double`) / `1e-5` (`float`).

**Exit**: backend parity test passes on a local `pip install -e .[all]` build (all three `.so` files coexist; switching is observable).

**Effort**: M (≤ 1 day).

### P6.5.M3 — cibuildwheel extras pipeline

**Goal**: the release pipeline builds one wheel per `(OS × arch × CPython × backend)` slot and tags each wheel with the backend in its filename or wheel metadata so `pip install tensor-named-axis[eigen]` picks the right one.

**Deliverables**:

- Strategy: ship `tensor-named-axis` (reference-only baseline) + `tensor-named-axis-eigen` + `tensor-named-axis-webgpu` as separate distribution-style packages where the *latter two* declare `tensor-named-axis` as a runtime dep and ship only their backend-specific `_tensor_native_<name>.so` extension module under the `tensor/` namespace package. The extras alias `tensor-named-axis[eigen]` resolves to `tensor-named-axis-eigen` (i.e., the extras pull in companion projects), so `pip install tensor-named-axis[eigen]` Just Works.
- `.github/workflows/cibuildwheel.yml` matrix grows: `backend: [reference, eigen, webgpu]` × the existing OS / arch matrix. The Windows WebGPU slot is **skipped** in M3 (Dawn vcpkg port on Windows requires the maintainer's verification — tracked as a M4 stretch).
- Per-backend `pyproject.toml`-equivalent for the companion projects (`tensor-named-axis-eigen/pyproject.toml`, `tensor-named-axis-webgpu/pyproject.toml`) — kept *in-tree* under `python/extras/` so they ship from the same repo, with their own scikit-build-core invocation pinning `TENSOR_KERNEL_BACKEND={eigen,webgpu}`.
- `tensor-named-axis`'s `[project.optional-dependencies]` declares:
  ```toml
  [project.optional-dependencies]
  reference = []  # explicit alias for the default
  eigen = ["tensor-named-axis-eigen==<same-version>"]
  webgpu = ["tensor-named-axis-webgpu==<same-version>"]
  all = ["tensor-named-axis[eigen,webgpu]"]
  ```

**Exit**: `pip install tensor-named-axis[all]` on a fresh ubuntu-latest container installs three wheels, `python -c "import tensor; tensor.set_backend('webgpu'); print(tensor.contract(...))"` works.

**Effort**: L (2 days).

### P6.5.M4 — `0.3.0` release

**Goal**: ship the extras to PyPI as `0.3.0`.

**Deliverables**:

- Maintainer-side: register `tensor-named-axis-eigen` + `tensor-named-axis-webgpu` on PyPI; configure trusted-publisher policy for both (same workflow / environment as the base project).
- `release/0.3.0` branch from `develop`: version bumps + CHANGELOG `[0.3.0]` section + roadmap update (Phase 6.5 → Shipped).
- `python/notebooks/05_swappable-backends.ipynb` — Python-side equivalent of `tutorials/08_swappable-backends.ipynb`: demonstrate `set_backend("reference")` vs `set_backend("eigen")` on a small matmul, with timing.
- HuggingFace Space `requirements.txt` switches to `tensor-named-axis[eigen]` (Eigen is free-tier-CPU-appropriate; WebGPU on HF Space requires GPU hardware which is paid).

**Exit**: `0.3.0` tag on `main`; PyPI shows three project entries (`tensor-named-axis`, `tensor-named-axis-eigen`, `tensor-named-axis-webgpu`); `pip install tensor-named-axis[all]` works in a fresh venv on at least one CI matrix slot per platform.

**Effort**: M (1 day).

---

## Out of scope

- **Windows WebGPU wheel** in M3. Dawn vcpkg port on Windows needs maintainer verification; Windows WebGPU is a P6.5 follow-up tracked separately.
- **conda-forge multi-output recipe**. The `tensor-feedstock` recipe stays single-output (reference only) until a Phase 6.5.1 follow-up; conda-forge users get the extras via `pip install` inside a conda env until then.
- **`set_backend()` switching mid-computation**. `set_backend()` after the first arithmetic op is undefined — the rebind only takes effect for *subsequently constructed* tensors. Document this constraint; do not implement live migration.
- **Backend-specific Python wheel optimisations**. The `eigen` wheel could ship Eigen-specialised pyi stubs for IDE completion; deferred until measured developer-friction signal.

---

## CI matrix changes

Existing M6 matrix (20 wheels per release):
- Linux x86_64 × CPython 3.9–3.13 = 5
- macOS x86_64 × CPython 3.9–3.13 = 5
- macOS arm64 × CPython 3.9–3.13 = 5
- Windows x86_64 × CPython 3.9–3.13 = 5

Phase 6.5 matrix (~52 wheels per release):
- Reference (base): 20 wheels (unchanged)
- Eigen: 20 wheels (Linux + macOS x2 + Windows)
- WebGPU: 12 wheels (Linux + macOS x2 only; Windows skipped per M3 scope)

CI time scales linearly; one full release matrix moves from ~30 min to ~80 min. PR-time CI is unaffected (cibuildwheel still only runs on `workflow_dispatch` + tag push).

---

## Risks

- **R-P6.5.1 — Companion-project versioning drift**. `tensor-named-axis-eigen==0.3.0` depending on `tensor-named-axis==0.3.0` means a `0.3.0` patch release needs to bump all three in lockstep. Mitigation: a single `release.sh` helper bumps all three `pyproject.toml`s + tags them together.
- **R-P6.5.2 — Namespace-package conflict on `tensor/`**. Both `tensor-named-axis` and `tensor-named-axis-eigen` install under `tensor/`. Using a [PEP-420 implicit namespace package](https://peps.python.org/pep-0420/) (no `__init__.py` in the shared namespace dir) is the canonical fix; verified in M1 via a smoke import. Falling back to non-namespace requires moving `__init__.py` into a separate sub-package, which would change the import path — avoid.
- **R-P6.5.3 — `pip install` resolver pinning**. If `tensor-named-axis==0.3.0` is installed but `tensor-named-axis-eigen==0.2.9` is too, the extension module ABI may mismatch. Mitigation: each companion project pins `tensor-named-axis==<exact-version>` (not `>=`) so `pip` either upgrades the base or refuses.
- **R-P6.5.4 — Dawn vcpkg port instability**. The Dawn version + Windows toolchain combination is known-fragile per [ADR-0014 §1](../arc42/09-decisions/0014-external-substrate-strategy.md). M3 scopes Windows WebGPU out for this reason; M4 stretch goal if maintainer's local Windows verification succeeds before the `0.3.0` tag.

---

## Sequencing relative to other work

| Phase | Status (2026-05-13) | Relation to Phase 6.5 |
| ----- | -------------------- | --------------------- |
| Phase 6 | ✅ closed (retrospective shipped, `0.2.0` release branch ready) | predecessor |
| **Phase 6.5** | **this plan** | next |
| Phase 5 (`tensor::linalg` shim) | paused — `<linalg>` not shipped in libc++/libstdc++/MSVC STL | parallel-able; no dependency |
| Bibliography audit | first audit 2026-11-11 | independent |

The `0.2.0` release ceremony (Phase 6 close) is the gate. Phase 6.5 work can branch from `develop` once `0.2.0` is tagged, OR can start now on top of pre-`0.2.0` develop with the understanding that the first Phase 6.5 PRs ride on top of the `release/0.2.0` back-merge.

---

## Cross-references

- Anchor ADR: [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md).
- Phase 6 retrospective (deferral rationale): [`../reports/2026-05-13_phase-6-python-sdk-retrospective.md`](../reports/2026-05-13_phase-6-python-sdk-retrospective.md).
- `KernelBackend` port (what each backend module links against): [`../detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md).
- M3 release infrastructure (cibuildwheel matrix to extend): [`.github/workflows/cibuildwheel.yml`](../../.github/workflows/cibuildwheel.yml).
