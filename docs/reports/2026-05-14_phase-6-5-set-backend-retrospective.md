---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Phase 6.5 retrospective — `set_backend()` via PEP-508 extras, 2026-05-14

| Metadata        | Value                                                                                  |
| --------------- | -------------------------------------------------------------------------------------- |
| Status          | Stable                                                                                 |
| Type            | Layer B — retrospective report (dated, append-only)                                    |
| Owner           | uyuutosa                                                                               |
| Anchor ADR      | [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) |
| Related plan    | [`../impl-plans/2026-05-13_phase-6-5-set-backend.md`](../impl-plans/2026-05-13_phase-6-5-set-backend.md) |
| Predecessor     | [`./2026-05-13_phase-6-python-sdk-retrospective.md`](./2026-05-13_phase-6-python-sdk-retrospective.md) (Phase 6 close) |
| Successor (planned) | Phase 7 retrospective when the next major surface lands                            |

## Executive summary

Phase 6.5 (runtime backend selection via PEP-508 extras) **shipped its implementation in two days** — M1 (multi-backend build pipeline) and M2 (`set_backend()` Python surface) on 2026-05-14 morning, M3 (cibuildwheel extras + companion projects) on 2026-05-14 afternoon, M4 prep (`tools/release.sh` lockstep bumper + ceremony refresh) on 2026-05-14 evening. The actual `0.3.0` release ceremony is queued behind `0.2.0` per the maintainer's 2026-05-14 sequencing decision; PR #115 (`release/0.2.0` draft) needs to merge first.

One major design discovery during M3 implementation: **nanobind 2.x's type registry is process-global** ([R-P6.5.5](../arc42/11-risks/overview.md)). [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)'s "three `.so` files coexist under PEP-420 namespace + runtime rebind" design assumed pybind11-style `module_local` semantics that nanobind 2.x doesn't have. The recovery: ship the build infrastructure (companion projects + cibuildwheel matrix + extras) intact, and adjust the M2 `set_backend()` semantics to single-backend-per-process with `TENSOR_BACKEND` env-var workaround. Mid-process backend switching is now an explicit known limitation rather than a feature.

The shape of the Phase 6.5 deliverable is what ADR-0019 specified at the *packaging* level (extras over fat wheel, per-companion distributions, lockstep version pins, ~55-wheel matrix) but with reduced runtime flexibility. The trade-off is honest: most users want one backend per environment anyway; the rare "switch within a process" case now needs a fresh Python process.

## What shipped — milestones

| Milestone | PRs  | Deliverable                                                                                       |
| --------- | ---- | ------------------------------------------------------------------------------------------------- |
| —         | #119 | ADR-0019 (extras-not-fat-wheel) + Phase 6.5 impl-plan; roadmap entry.                              |
| **P6.5.M1** | #123 | `python/CMakeLists.txt` parameterised by `TENSOR_KERNEL_BACKEND` env var; module name becomes `_tensor_native_{eigen,webgpu}.so` for non-reference builds; new `m.attr("__backend__")` for Python-side introspection; Eigen / Dawn find_package + link_libraries on the matching backend. |
| **P6.5.M2** | #123 (same PR) | `tensor.set_backend()` / `current_backend()` / `list_available_backends()` Python surface; lazy `importlib` discovery; rebinding of top-level exports; `sys.modules` re-registration trick from PR #104 re-applied; missing-backend error includes the `pip install` command. 11 new pytest cases (`test_set_backend.py`). |
| **P6.5.M3** | #124 | cibuildwheel matrix grows from `4 OS/arch × 5 CPython = 20 wheels` to `3 backends × {4, 4, 3} OS/arch × 5 CPython = ~55 wheels` (Windows webgpu scoped out). Two companion projects under `python/extras/` (`tensor-named-axis-eigen`, `tensor-named-axis-webgpu`) sharing the `tensor/` namespace package; lockstep version pin on the base distribution; backend-specific apt / brew / vcpkg bootstrap steps in the workflow. **R-P6.5.5 discovered + recovered**: single-backend-per-process semantics; `TENSOR_BACKEND` env var as the cross-process switch mechanism. |
| **P6.5.M4 prep** | #125 | `tools/release.sh` lockstep version bumper (5 files in one pass — `vcpkg.json`, `CMakeLists.txt`, base `pyproject.toml`, two companion `pyproject.toml`s); `docs/design-guide/release-ceremony.md` §2 step 2 refreshed to invoke the helper; roadmap reflects M1–M3 shipped + M4 queued behind `0.2.0`. |
| **P6.5.M4 actual** | (pending — maintainer-side) | `release/0.3.0` branch cut from develop, PyPI registration of `tensor-named-axis-eigen` + `tensor-named-axis-webgpu`, trusted-publisher policies on all three projects, tag + back-merge. Queued behind `0.2.0` per the 2026-05-14 sequencing decision. |

## What was harder than planned

1. **nanobind 2.x lacks `module_local`** ([R-P6.5.5](../arc42/11-risks/overview.md)). The impl-plan §"R-P6.5.2" preregistered "Namespace-package conflict on `tensor/`" but classified it as 🟢 Low because PEP-420 was assumed to resolve it. The deeper issue — nanobind's type registry being process-global — only surfaced when M3 tried to import both `_tensor_native.so` and `_tensor_native_eigen.so` in the same Python process. Symptoms:

   ```
   RuntimeWarning: nanobind: type 'Axis' was already registered!
   AttributeError: module 'tensor._tensor_native_eigen' has no attribute 'Axis'
   ```

   The second `import` produces partial bindings — class attributes are missing on the second module. nanobind 2.12 has no equivalent of pybind11's `py::module_local()` flag. The fix would require either:
   - **Symbol-mangling** the C++ binding code per backend (each module uses unique C++ type names like `Reference::Axis`, `Eigen::Axis`).
   - **Pure-Python dispatch surface** on top of a single native module that contains all three KernelBackend adapters.
   - **Lazy module unload + reload** with `importlib.reload` (fragile for native modules).

   Phase 6.5 chose the pragmatic recovery: ship single-backend-per-process, document the limitation, defer the lifting to a follow-up exploration. The infrastructure (build pipeline + companion projects + cibuildwheel matrix + Python surface) all still ships; only the "switch within a process" use case is constrained.

2. **scikit-build-core's `[tool.scikit-build.cmake.define]` map**. The original M1 design used `CIBW_ENVIRONMENT='TENSOR_KERNEL_BACKEND=eigen'` to pass the env var through cibuildwheel. The cleaner pattern (per scikit-build-core docs) is `[tool.scikit-build.cmake.define] TENSOR_KERNEL_BACKEND = "eigen"` directly in the companion's `pyproject.toml`, which avoids the env-var-leak-into-other-builds problem. Switched mid-M3 once the env-var pattern caused matrix-cell cross-contamination in a local cibuildwheel rehearsal.

3. **Macro parametrisation of `NB_MODULE(...)`**. nanobind's `NB_MODULE(name, m) { ... }` takes a literal token. To switch the module name based on `TENSOR_KERNEL_BACKEND` at compile time, we needed a preprocessor macro: `#define TENSOR_NB_MODULE_NAME _tensor_native_eigen` then `NB_MODULE(TENSOR_NB_MODULE_NAME, m)`. nanobind's `NB_CONCAT` / `NB_TOSTRING` macros expand correctly when the argument is itself a macro (the standard indirect-concatenation idiom), so this works — but the GCC 11 / CTAD interaction made the diagnostic for failed expansions less helpful than ideal.

4. **Release branch sequencing**. Phase 6.5 implementation work landed on `develop` while `release/0.2.0` (PR #115) was still draft. The Phase 6.5 retrospective (this file) and any future Phase 6.5 work all sit on develop, ahead of the unmerged `0.2.0` release. M4 actual is therefore *gated* on `0.2.0` shipping. The maintainer's 2026-05-14 directive resolved the ordering (`0.2.0` first), but until PR #115 merges the Phase 6.5 surface is "shipped on develop, not yet packaged for PyPI". Acceptable; the [`design-guide/release-ceremony.md`](../design-guide/release-ceremony.md) §5 soft rule "release PRs stay draft until PyPI prerequisites land" handles the holding pattern.

## What was easier than planned

1. **The C++ Domain didn't need changes**. The `KernelBackend` port surface ([ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md)) was already a configure-time choice via `-DTENSOR_KERNEL_BACKEND={reference,eigen,webgpu}`. Phase 6.5's parameterisation of `python/CMakeLists.txt` reuses the same env-var + CMake-cache-variable pattern. Zero new C++ Domain surface; the entire Phase 6.5 implementation lives at the build-system + Python adapter boundary.

2. **The companion-project pattern is elegant once it clicks**. Each companion is *just* a `pyproject.toml` — 50 lines pinning `TENSOR_KERNEL_BACKEND` and the lockstep dep version. The same `python/CMakeLists.txt` + `python/src/_tensor_native.cpp` source tree feeds all three builds via `cmake.source-dir = "../../"`. No duplication, no per-backend source tree. The 2-line patch to the base `pyproject.toml`'s `[project.optional-dependencies]` plus the helper script (`tools/release.sh`) keep the three projects in lockstep at release time.

3. **`tools/release.sh` cycle time**. The lockstep bumper across 5 files turned out to be ~80 lines of bash + sed; semver validation + the friendly reminder + restore-on-no-CHANGELOG-promotion took most of the script-time effort. The 5-file update completes in <0.5 s; the maintainer's release-time cycle is now `bash tools/release.sh 0.3.0` instead of five separate `sed -i` invocations across files that might drift. Mitigates [R-P6.5.1](../arc42/11-risks/overview.md) directly.

4. **The cibuildwheel matrix expansion**. Going from 20 wheels per release to ~55 looked daunting, but the cibuildwheel `matrix.include` + per-cell `project_dir` parameter handle it cleanly. The workflow's `if: matrix.backend == 'eigen' && runner.os == 'Linux'` style conditionals keep backend-specific setup steps localised. The hardest part wasn't writing the matrix — it was deciding what to do for Windows WebGPU (M3-scoped out → R-P6.5.4 follow-up).

## What was new in scope vs the impl-plan

1. **R-P6.5.5 lessons-learned section** — wasn't on the impl-plan's risk register. Filed during M3 implementation; preserved in [arc42 §11 §5](../arc42/11-risks/overview.md) for the next reviewer.
2. **`tools/release.sh`** — the impl-plan §"Risks" R-P6.5.1 said "A `release.sh` helper bumps all three `pyproject.toml`s + tags them together"; the helper was originally a follow-up. PR #125 brought it in as M4 prep so the M4 actual ceremony is a one-liner when `0.2.0` ships.
3. **`m.attr("__backend__")` introspection on the native module** — wasn't in the impl-plan but felt natural to add during M2 implementation. Makes the `test_native_backend_attribute_matches_current_backend` test possible (it cross-validates that the loaded native module knows its own backend identity).

## Deferred / known-stale items

- **P6.5.M4 actual release** — queued behind `0.2.0` per the maintainer's 2026-05-14 sequencing decision. Once PR #115 (`release/0.2.0`) ships, the M4 ceremony is one `tools/release.sh 0.3.0` + CHANGELOG promotion + PR + tag.
- **Lifting R-P6.5.5** — Phase 6.5 follow-up exploration. Possible paths: symbol-mangled C++ types per backend, pure-Python dispatch surface, or `importlib.reload` of native modules. No follow-up impl-plan yet; investigation depth TBD.
- **Windows WebGPU wheel** — M3-scoped out (R-P6.5.4). Dawn vcpkg port on Windows needs maintainer's local verification before the workflow's Dawn-bootstrap step can be wired. M4 stretch goal.
- **conda-forge feedstock** — three outputs (`tensor-named-axis`, `tensor-named-axis-eigen`, `tensor-named-axis-webgpu`) once the PyPI publishes land. Tracked in arc42 §7 §4d as a planned follow-up.
- **`python/notebooks/05_swappable-backends.ipynb`** — Python-side equivalent of `tutorials/08_swappable-backends.ipynb`. Will demonstrate the `TENSOR_BACKEND` env-var pattern instead of mid-process rebind. Not yet authored; a Phase 6.5 follow-up.

## Numbers (Phase 6.5 cumulative, 2026-05-13 → 2026-05-14)

- **4 merged PRs** (`#119`, `#123`, `#124`, `#125`) directly implementing Phase 6.5. (Plus PRs #120–#122 in the same window addressed adjacent docs / bug-fix concerns.)
- **1 new ADR** ([ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)).
- **1 new impl-plan** (`docs/impl-plans/2026-05-13_phase-6-5-set-backend.md`).
- **2 new companion projects** under `python/extras/`.
- **1 new helper script** (`tools/release.sh`, ~80 lines bash).
- **1 new risk row** ([R-P6.5.5](../arc42/11-risks/overview.md)).
- **3 new Python public-surface symbols** (`set_backend`, `current_backend`, `list_available_backends`).
- **10 new pytest cases** (`python/tests/test_set_backend.py`).
- **cibuildwheel matrix**: 20 wheels per release → ~55 wheels per release (~2.75× growth; Windows webgpu out-of-scope reduces from ~60).
- **0 new C++ source files** (the C++ Domain side didn't need changes).
- **CI on develop**: still 100% green at every PR boundary in this phase.

## Phase 7 entry conditions

What's *ready* for whatever Phase 7 turns out to be:

- **Both Python distributions infra is in place** — the base + 2 companions. Adding a 4th backend (or a CUDA wheel variant) is now a recipe-follow exercise: copy a companion pyproject.toml, set `TENSOR_KERNEL_BACKEND`, add a matrix entry in cibuildwheel.yml, register the PyPI project, configure trusted publishing.
- **Release ceremony is two commands** — `bash tools/release.sh <semver>` + CHANGELOG promotion. The maintainer's manual surface is the PyPI / GitHub Release setup, not the file edits.
- **R-P6.5.5 is well-documented** — anyone arriving at "I want to switch backends mid-process" finds the constraint + workaround + follow-up direction in arc42 §11 + the design-guide.
- **The Phase 6.5 forward-doc** ([`docs/user-manual/how-to/use-set-backend.md`](../user-manual/how-to/use-set-backend.md)) — the user-facing surface is documented; needs only the "fresh process required" caveat added when M4 ships.

What's *not* ready (and what Phase 7 would need to address if it picks up `tensor`'s headline trajectory):

- **The 2-day-build cadence isn't sustainable** for indefinitely many phases. Phase 6 was 17 PRs in 2 days; Phase 6.5 was 4 PRs over 2 days. The maintainer's bandwidth at this rate is a finite resource.
- **R-P6.5.5 lift** is the biggest known technical debt going into whatever's next.
- **Conda-forge feedstock** is still planned, not shipped. Phase 7 might absorb it.

## References

- Anchor ADR: [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md).
- Phase 6.5 impl-plan: [`../impl-plans/2026-05-13_phase-6-5-set-backend.md`](../impl-plans/2026-05-13_phase-6-5-set-backend.md).
- Predecessor retrospective (Phase 6 close): [`./2026-05-13_phase-6-python-sdk-retrospective.md`](./2026-05-13_phase-6-python-sdk-retrospective.md).
- The `KernelBackend` port that Phase 6.5 wraps: [`../detailed-design/kernel-backend-port.md`](../detailed-design/kernel-backend-port.md).
- The Python binding surface: [`../detailed-design/python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) §3.5 has the Phase 6.5 forward-anchor notes; this retrospective adds the implementation-experience postscript.
- The lessons-learned arc42 §11 entries: R-P6.5.1 (lockstep drift; mitigated by `tools/release.sh`), R-P6.5.4 (Dawn vcpkg on Windows), R-P6.5.5 (nanobind type-registry — new risk discovered during this phase).
- The release ceremony updated for the lockstep helper: [`../design-guide/release-ceremony.md`](../design-guide/release-ceremony.md).
- User-facing forward-doc: [`../user-manual/how-to/use-set-backend.md`](../user-manual/how-to/use-set-backend.md).
