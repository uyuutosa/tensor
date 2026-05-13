---
status: Accepted
owner: tensor
last-reviewed: 2026-05-13
---

# ADR-0019: Phase 6.5 — runtime backend selection via PEP-508 extras, not a fat wheel

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-13                                                     |
| Deciders  | uyuutosa (maintainer)                                          |
| Consulted | —                                                              |
| Informed  | Phase 6.5 contributors                                         |

---

## Context and Problem Statement

[ADR-0018 §F](./0018-phase-6-python-sdk-entry-via-nanobind.md) named "backend selection" as part of the minimum-viable Python surface. The Phase 6 impl-plan ([`docs/impl-plans/2026-05-12_phase-6-python-sdk.md`](../../impl-plans/2026-05-12_phase-6-python-sdk.md) §P6.M6) proposed a single fat wheel bundling all three pre-built `_tensor_native_{reference,eigen,webgpu}.so` adapters, with `tensor.set_backend("reference" | "eigen" | "webgpu")` selecting between them at runtime.

The [Phase 6 retrospective](../../reports/2026-05-13_phase-6-python-sdk-retrospective.md) deferred this to **Phase 6.5** because the wheel-packaging question is orthogonal to the release-publish infrastructure that M6 actually delivered (cibuildwheel + PyPA trusted publishing). With M6 shipped and `0.2.0` ready for tag, this ADR resolves the deferred packaging question so Phase 6.5 implementation can start.

The C++ side already supports backend selection at configure time via `-DTENSOR_KERNEL_BACKEND={reference,eigen,webgpu}` ([ADR-0011](./0011-kernel-backend-port-api.md)). A Python wheel is built with a fixed `TENSOR_KERNEL_BACKEND` at scikit-build-core configure time; the question is **how many such wheels we ship per release, and how a user picks between them**.

---

## Decision Drivers

In priority order:

- **DD-1 — pip-install bandwidth + cold-start cost**. The Dawn runtime is large (~50 MB compiled; the `dawn::webgpu_dawn` link target is the dominant wheel-size contributor). A user on the educational-default path shouldn't pay GPU runtime download cost just to read the README.
- **DD-2 — runtime switching as a teaching surface**. The `tutorials/08_swappable-backends.ipynb` and the upcoming Python equivalent (`python/notebooks/05_swappable-backends.ipynb`, planned) want to demonstrate live backend comparison: "now switch to WebGPU and measure the speedup". Runtime switching is genuinely valuable for the educational frame.
- **DD-3 — packaging idiom alignment**. PyTorch, JAX, TensorFlow all settled on multi-wheel-via-extras (or PyTorch's own index) rather than fat wheels for the GPU-runtime trade-off. Following the established idiom reduces onboarding friction.
- **DD-4 — CI wheel-build matrix size**. Each backend × OS × arch × CPython version multiplies the cibuildwheel matrix. The M6 matrix is already 20 wheels per release (1 backend × 4 OS/arch × 5 CPython); a fat wheel keeps that at 20; per-backend extras pushes it to 60. Manageable but not free.

---

## Considered Options

1. **Fat wheel** — single `tensor-named-axis` wheel bundles all three pre-built `_tensor_native_{reference,eigen,webgpu}.so`. `set_backend()` switches between them at runtime. (Original P6.M6 proposal.)
2. **PEP-508 extras** — `pip install tensor-named-axis` ships reference only; `pip install tensor-named-axis[eigen]` adds Eigen; `pip install tensor-named-axis[webgpu]` adds WebGPU; `pip install tensor-named-axis[all]` adds both. `set_backend()` switches between *installed* backends only; unselected backends raise `RuntimeError` with install instructions.
3. **Separate distributions per backend** — publish `tensor-named-axis`, `tensor-named-axis-eigen`, `tensor-named-axis-webgpu` as three independent PyPI projects. User picks one. No `set_backend()`.
4. **PyTorch-style external index** — host a `https://uyuutosa.github.io/tensor/simple/` index serving backend-suffixed wheels (`tensor-named-axis-0.2.0-cp311-cp311-linux_x86_64+webgpu.whl`). User opts in with `pip install --extra-index-url …`. Maintains compatibility with the standard `pip install tensor-named-axis` path.

---

## Decision Outcome

**Chosen option: 2 — PEP-508 extras.**

The fat-wheel option (1) bakes in the Dawn runtime cost for every user, regardless of whether they ever touch the GPU path — a 100 MB+ wheel for what is, on the educational default path, a 5 MB header-only library. **DD-1 dominates**: most users are on the reference path most of the time, and the extras pattern keeps that path light.

The extras pattern (2) preserves runtime switching as a teaching surface within whatever subset the user installed (**DD-2**), which is sufficient for the swappable-backends notebook — readers who want all three install `tensor-named-axis[all]` once and switch freely. Users who only need Eigen install `[eigen]` and never download Dawn.

Per-backend separate distributions (3) sacrifices runtime switching entirely (would require uninstall + reinstall to compare backends), which fails **DD-2**.

External-index (4) is what PyTorch does and works at scale, but adds maintenance burden (we'd host the index) and breaks the `pip install tensor-named-axis` flow that ADR-0018 R-P3 settled on. The benefit over option 2 is marginal for a library this size.

### Y-statement summary

> In the context of **packaging the three `KernelBackend` adapters for the Python SDK**, facing **the Dawn runtime's 50 MB+ link cost vs the need for runtime backend switching in the educational frame**, we decided for **PEP-508 extras (`tensor-named-axis[eigen]` / `[webgpu]` / `[all]`)** to achieve **a 5 MB default install path with opt-in GPU expansion**, accepting **a 3× larger cibuildwheel matrix (60 wheels/release) and the need for `set_backend()` to detect installed backends at import time**.

---

## Pros and Cons of the Options

### Option 1: Fat wheel

- Pros:
  - Single `pip install tensor-named-axis` covers every demo path.
  - `set_backend()` always works for all three backends.
  - cibuildwheel matrix stays at 20 wheels per release.
- Cons:
  - **Every user pays the Dawn runtime download cost** — ~100 MB wheel vs ~5 MB for reference-only. Bad for the educational-default path.
  - Inflates conda-forge channel storage proportionally.
  - PyPI per-project storage soft cap (10 GB) hits sooner — 20 × 100 MB = 2 GB per release; 5 releases = cap reached.

### Option 2: PEP-508 extras (chosen)

- Pros:
  - **Default `pip install` is ~5 MB** — fast cold-start, fast Colab boot.
  - Users opt into Eigen / WebGPU explicitly when they need the perf or GPU path.
  - `set_backend()` works at runtime between whatever subset is installed — the swappable-backends notebook still demos comparisons, just within the installed set.
  - Aligns with JAX's `jax[cuda12]` idiom; familiar to Python ML users.
  - Each backend's wheel stays inside PyPI's per-file size limit (60 MB default, requestable to 100 MB).
- Cons:
  - cibuildwheel matrix grows 3× (60 wheels/release). Still bounded; CI time scales linearly.
  - Users wanting all three need `[all]` or three explicit extras; one more cognitive step.
  - `set_backend("webgpu")` on a `[eigen]`-only install raises at runtime — has to be a clear error with install instructions.

### Option 3: Separate distributions per backend

- Pros:
  - Each distribution is fully self-contained — no extras magic.
  - PyPI metrics distinguish reference vs Eigen vs WebGPU adoption.
- Cons:
  - **No runtime switching** — fails the swappable-backends teaching surface.
  - Three projects to register + three trusted-publisher policies + three CHANGELOGs.
  - Users who want both `eigen` and `webgpu` install two projects that conflict in `sys.modules`.

### Option 4: PyTorch-style external index

- Pros:
  - Keeps `pip install tensor-named-axis` as the canonical path.
  - Allows GPU-runtime-suffixed wheels (`+cu118`-style).
- Cons:
  - We have to host + maintain the index (a static GitHub Pages directory could work but it's still infrastructure).
  - `pip install --extra-index-url ...` is a friction step PyTorch users have learned to tolerate but is non-trivial for newcomers.
  - Doesn't actually solve runtime switching (each wheel still ships one backend).

---

## Consequences

### Positive

- **Educational default stays light**. `pip install tensor-named-axis` on Colab / Binder / a reader's laptop is a 5 MB download. The README demos all run with the default install.
- **Runtime switching is genuinely useful in the educational frame**. The swappable-backends notebook can demonstrate `set_backend("reference")` vs `set_backend("eigen")` on a `[eigen]` install, or all three on `[all]`.
- **WebGPU adopters pay for what they use**. The 50 MB+ Dawn runtime download is opt-in via `[webgpu]`.

### Negative

- **cibuildwheel matrix 3×**. 60 wheels per tag instead of 20. CI time per release grows from ~30 min to ~90 min (sequential matrix; parallel jobs cap at GitHub's plan limit). Acceptable: releases are infrequent.
- **`set_backend()` is a runtime-detection API, not a compile-time choice**. The Python module imports all three `_tensor_native_*.so` lazily; missing backends raise on `set_backend()`-call, not on `import tensor`.
- **Three sets of build deps per release**. CI needs vcpkg ports for Eigen3 *and* Dawn during the `[all]` wheel build. Dawn vendoring stays per [ADR-0014 §1](./0014-external-substrate-strategy.md) (consumer-overridden vcpkg baseline).

### Neutral

- **`tensor-named-axis` (no extras) ships reference only**. Users who want explicit "I'm on the educational default" can say `tensor-named-axis[reference]` (a no-op alias).
- **conda-forge feedstock** stays single-recipe; `eigen` and `webgpu` become optional dependencies in the recipe's `outputs:` section.

### Follow-ups

- [ ] Phase 6.5 impl-plan: `docs/impl-plans/2026-05-13_phase-6-5-set-backend.md` with milestones P6.5.M1 (multi-backend build pipeline), P6.5.M2 (`tensor.set_backend()` + parity tests), P6.5.M3 (cibuildwheel-extras pipeline), P6.5.M4 (`0.3.0` release).
- [ ] Update `pyproject.toml`'s `[project.optional-dependencies]` once the C++ multi-backend build pipeline is in place.
- [ ] Update `huggingface/space/requirements.txt` to use `tensor-named-axis[all]` (or `[webgpu]` if the Space gets GPU hardware) once `0.3.0` ships.

---

## Compliance / Validation

- **Verification**: `tensor.set_backend("webgpu")` on a `[reference]`-only install must raise `RuntimeError` with the exact `pip install tensor-named-axis[webgpu]` install instruction in the message. Add to `python/tests/test_set_backend.py` once the surface lands.
- **Frequency**: every release. Phase 6.5 impl-plan's M2 exit criterion.

---

## More Information

### Related ADRs

- Prerequisite: [ADR-0018](./0018-phase-6-python-sdk-entry-via-nanobind.md) — Phase 6 entry decisions; §F flagged backend-selection as deferred.
- Architectural baseline: [ADR-0011](./0011-kernel-backend-port-api.md) — `KernelBackend` port API surface (the C++ side that the three wheels link against).
- Substrate strategy: [ADR-0014 §1](./0014-external-substrate-strategy.md) — Dawn as opt-in dependency via vcpkg manifest feature.
- Related: [ADR-0016](./0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) — talks to Dawn directly via `webgpu_cpp.h`; the WebGPU wheel's link target.

### References

- PEP-508 dependency specification: <https://peps.python.org/pep-0508/>
- PyPI per-project + per-file size limits: <https://pypi.org/help/#file-size-limit>
- JAX install matrix (idiom reference): <https://jax.readthedocs.io/en/latest/installation.html>
- PyTorch install matrix (external-index reference): <https://pytorch.org/get-started/locally/>
- Phase 6 retrospective: [`docs/reports/2026-05-13_phase-6-python-sdk-retrospective.md`](../../reports/2026-05-13_phase-6-python-sdk-retrospective.md) — `set_backend()` deferral rationale.
