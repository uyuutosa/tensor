---
status: Active
owner: tensor
last-reviewed: 2026-05-11
---

# Phase 3 — WebGPU adapter (third `KernelBackend` slot)

| Metadata       | Value                                                                  |
| -------------- | ---------------------------------------------------------------------- |
| Status         | Active (planned; no code shipped)                                      |
| Type           | Layer B — implementation plan (dated, append-only)                     |
| Owner          | uyuutosa                                                               |
| Predecessor    | [`2026-05-11_phase-2-5-backend-port-and-eigen.md`](./2026-05-11_phase-2-5-backend-port-and-eigen.md) (Phase 2.5; first non-reference Backend) |
| Related ADRs   | [ADR-0006 (WebGPU strategic)](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0009 (Hexagonal lite)](../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), [ADR-0010 (production-capable refine)](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0011 (KernelBackend port API)](../arc42/09-decisions/0011-kernel-backend-port-api.md) |
| Target window  | 2026-06-01 → 2026-08-31                                                |
| Exit version   | `0.0.4-alpha`                                                          |

## TL;DR

The third concrete `KernelBackend` adapter: WebGPU. Phase 2.5 shipped reference + Eigen; Phase 3 adds GPU. The architecture (Hexagonal lite + `KernelBackend` port) makes this a focused integration: implement the same 15 methods, but in WGSL kernels dispatched through Dawn or wgpu-native.

**Headline pedagogy** (ADR-0006): the same `dot(W, x)` runs on the learner's laptop GPU without a proprietary toolchain install. The same Domain code, the same tests, just `cmake -DTENSOR_KERNEL_BACKEND=webgpu`.

## Phase 3 milestones

### P3.M1 — ADR-0012: WebGPU adapter implementation design (week 1)

> Concrete adapter design, mirroring ADR-0011's structure for the port itself.

- [ ] ADR-0012 in `docs/arc42/09-decisions/0012-webgpu-adapter-implementation-design.md`.
- [ ] Decisions to capture:
  - Dawn (Chrome's WebGPU) vs wgpu-native (Rust-implemented WebGPU); vcpkg availability per platform.
  - WGSL kernel emission strategy: textual emission per op (simple, slow) vs templated kernels per shape (faster, more code).
  - Buffer ownership: GPU buffers as opaque handles inside `gpu::Backend`; never escape the adapter.
  - Synchronisation model: blocking `submit + wait` per call (MVP); async batching in a follow-up.
  - Element-type support: `float` only at MVP (WebGPU doesn't ship `f64` shaders broadly); `double` falls through to reference.

### P3.M2 — CMake plumbing + stub adapter (week 1–2)

> The integration point. Domain code stays unchanged.

- [ ] `vcpkg.json`: add `dawn` (or `wgpu-native`, per ADR-0012).
- [ ] `CMakeLists.txt`: `TENSOR_KERNEL_BACKEND` cache var gains `webgpu` value; conditional `find_package` + `target_link`.
- [ ] `include/tensor/core/backend/webgpu.hpp`: `Backend` class with the 15 port methods; initial implementation delegates *every* method to `reference::Backend` so the test suite passes the moment the build links.
- [ ] `static_assert(KernelBackend<Backend>)` at file scope verifies the port.
- [ ] CI gains a `backend=webgpu` job (or annotation that GPU tests are local-only).

### P3.M3 — Element-wise WGSL kernels (week 2–3)

> First operations to run on the GPU.

- [ ] WGSL shader template for unary element-wise (`exp`, `log`, `relu`, `neg`).
- [ ] WGSL shader template for binary same-shape element-wise (`add`, `sub`, `mul`, `div`).
- [ ] Buffer round-trip: CPU `DynamicTensor<float>` → GPU buffer → dispatch → GPU buffer → CPU `DynamicTensor<float>`.
- [ ] Cross-validation against reference: outputs match within 1e-5 for `float`.

### P3.M4 — Contraction (matmul) WGSL kernel (week 3–4)

> The op a real workload spends time on.

- [ ] WGSL tiled GEMM for (rank 2, rank 2) with single shared axis.
- [ ] (rank 2, rank 1) matvec.
- [ ] Higher-rank / multi-shared cases delegate to reference (consistent with Eigen adapter scope).
- [ ] Performance comparison vs reference and Eigen (single matmul, 1024 × 1024 × 1024).

### P3.M5 — Broadcast and reduction WGSL kernels (weeks 4–5)

> Round out the port surface.

- [ ] `broadcast_add` / `broadcast_sub` / `broadcast_mul` via a WGSL kernel parameterised by the BroadcastPlan's `a_source` / `b_source` (passed as a uniform buffer).
- [ ] `reduce_sum` via a tree-reduction WGSL kernel.
- [ ] `unbroadcast` via a kernel parameterised by `source_map` (uniform) — performs scatter-add along reduced axes.

### P3.M6 — Browser tutorial `tutorials/06_webgpu-acceleration.ipynb` (weeks 5–6)

> The pedagogical close.

- [ ] Notebook walks through:
  - Section 1 — why WebGPU (the "zero proprietary toolchain" pitch from ADR-0006).
  - Section 2 — backend swap demo: same expression on reference, Eigen, WebGPU.
  - Section 3 — perf comparison on matmul scaling 256² → 2048².
  - Section 4 — peek into a WGSL kernel.
- [ ] Optionally, a browser-deployed companion (Phase 4 territory, see exit criteria below).

## Exit criteria for `0.0.4-alpha`

- [ ] All 15 port methods implemented in WGSL (with reference fallbacks documented per ADR-0012).
- [ ] Cross-backend test agreement within 1e-5 for `float`.
- [ ] CI green (reference + eigen + webgpu where the runner has GPU access).
- [ ] tutorial 06 ships and is executable via xeus-cling.

## Out of scope for Phase 3

- WebGPU autograd-aware kernels (Phase 4): the existing autograd composes port primitives, so backward already works through the WebGPU adapter; what's missing is a GPU-resident `GradAccum` that avoids round-tripping each step. Deferred.
- Pinned host memory / mapped buffers for zero-copy. Deferred.
- `double` precision via shader extensions. Deferred.
- Browser-deployed Pyodide / wgpu-web companion. Phase 4.

## Risks and mitigations

| Risk                                                                          | Likelihood | Impact | Mitigation                                                                  |
| ----------------------------------------------------------------------------- | ---------- | ------ | --------------------------------------------------------------------------- |
| Dawn or wgpu-native vcpkg port instability                                   | Medium     | High   | ADR-0012 evaluates both and picks based on current port stability; the choice is reversible. |
| CI runners without GPUs                                                       | High       | Medium | Treat GPU tests as local-or-self-hosted; CI verifies compile + reference cross-check on a stub host. |
| WebGPU adoption in the C++ ecosystem still maturing                          | Medium     | Medium | Adapter delegates anything it can't yet handle to reference; nothing in the Domain knows or cares whether GPU is reachable. |
| Buffer round-trip overhead dominates for small operations                     | High       | Low    | Document explicitly in tutorial 06; perf comparison in M4 shows the crossover point. |

## Cut lines (drop top first if behind schedule)

1. P3.M5 (broadcast / unbroadcast / reduce_sum on GPU); delegate to reference; ship element-wise + matmul only.
2. P3.M6 perf section; ship tutorial through Section 2 (backend swap).
3. P3.M4 higher-rank matmul cases; element-wise + matvec only.

P3.M1, P3.M2, P3.M3 minimum, and P3.M6 minimum (sections 1–2) are non-negotiable for `0.0.4-alpha`.

## Status addendum — 2026-05-11 (post-research)

P3.M1 (ADR-0012) and P3.M2 (stub adapter + CMake plumbing) shipped (PRs #32 and #38 respectively).

After the [external-substrate research](../reports/2026-05-11_external-substrate-research.md) was completed on the same date, the next-step substrate picture clarifies as captured in [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md):

- **Dawn is now in vcpkg** (port `20260410.140140`, refreshed 2026-04-20). The original P3.M2 `vcpkg.json` entry can finally land — previously this step was blocked on a missing port and was deferred behind the stub. The Phase 3 build path becomes `find_package(dawn CONFIG REQUIRED)` gated on `TENSOR_KERNEL_BACKEND=webgpu`.
- **gpu.cpp is bus-factor 1 and has no vcpkg port.** Per [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md), `gpu.hpp` is vendored under `third_party/gpu_cpp/` with a `VENDORED_FROM` record (PR #41), not consumed as an upstream dependency.
- **The risks-and-mitigations table line "Dawn or wgpu-native vcpkg port instability" downgrades** from `Likelihood: Medium, Impact: High` to `Likelihood: Low, Impact: Medium`. Mitigation remains the same (the choice is one CMake variable away from being reversed).
- The `0.0.4-alpha` exit criteria still hold; the change is in *how* P3.M3 will be built, not *whether* it will ship.

### P3.M3 sub-split

To keep CI tractable while no self-hosted GPU runner exists, P3.M3 splits:

- **P3.M3.1 — Binary WGSL kernel sources (shipped, PR #43).** `include/tensor/core/backend/webgpu_wgsl.hpp` ships the four element-wise binary kernels (`add` / `sub` / `mul` / `div`) as `constexpr std::string_view` constants in gpu.cpp's `{{workgroupSize}}` / `{{precision}}` templated form. `tests/test_webgpu_wgsl.cpp` text-validates them (no shader compilation). The dispatch design is in `docs/detailed-design/webgpu-element-wise-kernels.md`.
- **P3.M3.3 — Unary WGSL kernel sources (shipped, this PR).** Same header, four more constants: `kExpF32` / `kLogF32` / `kReluF32` / `kNegF32`. Two-binding template (input + output) instead of three. ReLU expressed via `max(a, 0.0)` so it maps to a single GPU instruction. The WebGPU adapter's element-wise *source* surface is now feature-complete: 8 ops covered (4 binary + 4 unary).
- **P3.M3.2 — Dispatch wiring (deferred).** Replaces each `webgpu::Backend::{add,sub,mul,div,exp,log,relu,neg}` stub delegation with the gpu.cpp dispatch sequence described in the detailed-design doc §3 (binary) + §3.1 (unary). Preconditions: vcpkg baseline bumped to include `dawn@20260410.140140`+; self-hosted GitHub Actions runner with a Dawn-compatible GPU exists; ADR-0012's compile-only-CI policy upgrades to "compile-only on generic CI + numerical-agreement on self-hosted runner".

### P3.M4 sub-split

Mirrors P3.M3 split:

- **P3.M4.1 — Tiled GEMM WGSL source (shipped, this PR).** `webgpu_wgsl.hpp::kGemmF32` is a single readable tiled-GEMM kernel covering both matvec (rank-2 × rank-1) and matmul (rank-2 × rank-2) with one shared axis. 16 × 16 workgroup tile; 16-deep K slab; workgroup-shared `shA` / `shB`; two `workgroupBarrier()` per outer iteration. `tests/test_webgpu_wgsl.cpp` text-validates. `docs/detailed-design/webgpu-gemm-kernel.md` is the design (with P3.M4.2 dispatch-sequence pseudo-code).
- **P3.M4.2 — Dispatch wiring (deferred).** Replaces `webgpu::Backend::contract` delegation; gates on simple matvec / matmul case detection and delegates the rest. Same preconditions as P3.M3.2.

## Follow-ups beyond Phase 3

- Plan Phase 4 (`0.1.0` public release): Jupyter Book site on GitHub Pages, full tutorial corpus, LyX export module, release tag. Per [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md), Phase 4 also gains a canonical-reference framing pass (CITATION.cff; per-container "why this exists" comments; Jupyter Book "How to cite" section).
- Phase 1.5 remaining items: notebook-CI execute job now switches to **xeus-cpp** per [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md), replacing the xeus-cling target that was the long-standing backlog item here.

## References

- [ADR-0006 — WebGPU strategic choice](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md)
- [ADR-0011 — KernelBackend port API](../arc42/09-decisions/0011-kernel-backend-port-api.md)
- [Phase 2.5 plan — predecessor](./2026-05-11_phase-2-5-backend-port-and-eigen.md)
- [gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) — single-source C++ WebGPU wrapper
- [Dawn](https://dawn.googlesource.com/dawn) — Google's WebGPU implementation
- [wgpu-native](https://github.com/gfx-rs/wgpu-native) — Rust-based WebGPU implementation
- [WebGPU specification](https://www.w3.org/TR/webgpu/)
