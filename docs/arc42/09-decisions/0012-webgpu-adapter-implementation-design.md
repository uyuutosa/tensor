---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0012: WebGPU `KernelBackend` adapter — implementation design

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors                                                    |
| Refines   | [ADR-0006 (WebGPU strategic choice)](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0011 (KernelBackend port API)](0011-kernel-backend-port-api.md) |

---

## Context and Problem Statement

[ADR-0006](0006-adopt-webgpu-as-gpu-backend.md) selected WebGPU as the GPU path on educational-cost-of-entry grounds. [ADR-0011](0011-kernel-backend-port-api.md) fixed the `KernelBackend` port API. Phase 2.5 shipped two concrete adapters (`reference`, `eigen`) — the third slot is WebGPU.

This ADR resolves the *implementation* choices the Phase 3 plan flagged for M1: which WebGPU runtime, how to emit WGSL kernels, how to handle buffer lifecycle, how to synchronise, and how CI verifies a backend whose default host environment has no GPU.

The decisions here are constrained — they have to fit ADR-0010's "educational-first, production-capable via backend adapters" stance: the adapter is real (production users can use it) but explicit about its limitations.

---

## Decision Drivers

- **DD-1**: Cost of entry on the learner's laptop. WebGPU was chosen over CUDA precisely because the runtime is the GPU driver the user already has. The adapter must not require a proprietary toolchain.
- **DD-2**: Toolchain stability. The vcpkg ports for WebGPU runtimes are still in flux (2026); the adapter must survive a port being unavailable or broken on one platform without blocking the rest of the project.
- **DD-3**: Educational legibility. WGSL kernels are part of the teaching artifact. They must be short, readable, and one-kernel-per-operation.
- **DD-4**: Correctness. Outputs must agree with the reference adapter within a documented tolerance for `float`. Cross-validation is non-negotiable.
- **DD-5**: Solo-maintainer bandwidth. The adapter must be deliverable in six weeks of evening work (Phase 3's window).

---

## Considered Options

For each decision below we record the alternatives considered and the choice.

### O1 — WebGPU runtime

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **Dawn (Google)** | C++ API matures fastest; runs on every desktop GPU; powers Chrome's WebGPU implementation. | Source build is heavy; integrating requires CMake gymnastics. |
| **wgpu-native (gfx-rs)** | Rust-implemented; ships a C ABI; vcpkg port exists; matches Firefox's WebGPU implementation. | C ABI adds an extra integration layer for C++. |
| **gpu.cpp (Answer.AI)** | Single-header C++ wrapper around Dawn; explicitly educational. | Thinner ecosystem; depends on Dawn underneath; less battle-tested. |

**Chosen: gpu.cpp + Dawn.** gpu.cpp gives the educational-legibility win, Dawn is the production-grade engine underneath. wgpu-native remains a fallback if the Dawn build pipeline proves too painful on one platform — the adapter is structured so the runtime swap is a CMake option, not a code change.

### O2 — WGSL kernel emission strategy

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **Textual emission per operation, statically defined** | Simplest; each operator gets a fixed WGSL string; easy to read. | A new operator requires a hand-written WGSL kernel; can't fuse. |
| **Templated kernels parameterised by shape / extent uniforms** | Fewer distinct kernels; one matmul template covers many shapes. | More complex pipeline-state caching. |
| **JIT compilation from the named-axis expression graph** | Maximum specialisation; could outperform Eigen on small shapes. | Out of scope for an educational artifact; deferred to Phase 5+. |

**Chosen: textual emission per operation (one fixed kernel per port method).** The 15 port methods become 15 fixed WGSL strings + a handful of templated GEMM variants. Pipeline-state caching is keyed by `(kernel_name, dtype, shape_hint)` so we don't rebuild the pipeline on every call. The templated path can supersede this when measurement shows the dispatch overhead matters.

### O3 — Buffer ownership and lifecycle

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **GPU buffers as opaque handles inside `gpu::Backend` (chosen)** | The Domain never sees a GPU buffer; the adapter copies in / out per call. | Round-trip cost for small operations. |
| Persistent GPU buffers reachable from the Domain | Zero-copy across multiple operations. | Domain becomes GPU-aware; violates Hexagonal discipline. |
| Pinned host memory for fast transfer | Faster transfers; harder portability. | Phase 5+ optimisation. |

**Chosen: opaque GPU buffers inside the adapter.** Every `Backend` method receives `DynamicTensor<float>` on input, allocates a GPU buffer, dispatches, reads back, and returns a fresh `DynamicTensor<float>`. The transfer overhead is documented prominently in `tutorials/06_webgpu-acceleration.ipynb`. A future PR can introduce a `TensorView` GPU-resident wrapper; that is explicitly out of scope for Phase 3.

### O4 — Synchronisation model

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **Blocking submit + wait per call (chosen)** | Simplest; matches the synchronous `KernelBackend` port API. | No overlap between kernels. |
| Async batching via a queue inside the adapter | Better wall-clock for multi-step expressions. | Changes the port API; deferred. |

**Chosen: blocking submit + wait.** Matches what the port returns by-value already commits to.

### O5 — Element type support

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **`float` only at MVP (chosen)** | WGSL ships `f32` everywhere; `f64` requires shader extensions still patchy in 2026. | `double` calls fall back to reference (CPU). |
| Both `f32` and `f64` from day one | Symmetry with Eigen adapter. | Shader extension dependency cost. |

**Chosen: `float` only.** `double` is `if constexpr (std::is_same_v<T, double>)` → delegate to reference. When WebGPU `f64` extensions stabilise, this is a one-method-per-op addition.

### O6 — CI strategy

| Option | Pros | Cons |
| ------ | ---- | ---- |
| **Compile-only CI on stock runners + numerical agreement on a self-hosted GPU runner (chosen)** | Standard runners stay fast; GPU testing happens where it can. | Self-hosted runner is a maintenance burden. |
| Skip GPU testing entirely on CI | Cheapest. | Cross-backend agreement claim becomes aspirational. |
| Lavapipe / SwiftShader software rasteriser on standard runners | No special infra. | Slow; not all WebGPU features are supported. |

**Chosen: compile-only on standard CI + numerical agreement on a self-hosted GPU runner.** The compile-only step (which still runs all the *reference* tests) catches integration errors. Numerical agreement runs on the self-hosted runner per release tag rather than per PR. If self-hosted infra is unavailable, this falls back to "local-only" and the perf comparison notebook becomes the authoritative cross-check.

---

## Decision Outcome

The combined choices form a coherent adapter:

- **Runtime**: gpu.cpp + Dawn behind a CMake option `TENSOR_KERNEL_BACKEND=webgpu`; wgpu-native fallback reserved.
- **Kernels**: textual WGSL, one per port method, plus templated GEMM variants for the matmul fast paths from the Eigen adapter.
- **Buffers**: GPU-only, opaque, per-call; the Domain never sees them.
- **Sync**: blocking submit + wait.
- **Element types**: `float` only; `double` delegates to reference.
- **CI**: compile-only on standard runners; numerical agreement on a self-hosted GPU runner at release-tag time.

### Y-statement summary

> In the context of **shipping the third `KernelBackend` adapter (WebGPU)**, facing **the implementation choices that ADR-0011 left to the adapter**, we decided for **gpu.cpp on Dawn, textual WGSL per port method, opaque GPU buffers per call, blocking submit-and-wait, `float`-only at MVP, and a compile-only-on-CI / numerical-agreement-on-self-hosted strategy**, to achieve **a WebGPU adapter a learner can run on a stock laptop GPU without proprietary toolchains, with the same Domain code working everywhere**, accepting **higher per-call overhead for small ops and `double` falling back to reference**.

---

## Pros and Cons recap

See the per-decision tables above. The aggregate trade-off is:

- (+) Educational legibility preserved.
- (+) No proprietary toolchain required.
- (+) Adapter conforms to ADR-0011's port; same Domain code runs unchanged.
- (+) Reference / Eigen / WebGPU agree numerically by construction.
- (−) Small ops eat their savings on PCIe transfer.
- (−) `double` precision unavailable on GPU.
- (−) CI for numerical agreement requires self-hosted infra.

---

## Consequences

### Positive

- Phase 3 implementation has a frozen design surface; M2 onward can proceed without per-PR design debates.
- The MVP plays well with the existing `KernelBackend` port; no port changes required.
- Future GPU work (`double` precision, persistent GPU buffers, kernel fusion) becomes additive — each is a follow-up ADR that supersedes one of this one's choices.

### Negative

- Self-hosted CI runner is a real operational obligation.
- Users running Apple Silicon will need Metal (Dawn's backend) — the path works in 2026 but is less battle-tested than CUDA-backed paths.

### Neutral

- The Eigen adapter remains the recommended choice for CPU-bound workloads; WebGPU only wins for medium-to-large matmuls.

### Follow-ups

- [ ] **P3.M2**: CMake plumbing + stub adapter delegating to reference. vcpkg.json gains `dawn` and `gpu.cpp` (latter likely vendored). `TENSOR_KERNEL_BACKEND` gains `webgpu`.
- [ ] **P3.M3**: Element-wise WGSL kernels (unary + same-shape binary).
- [ ] **P3.M4**: Templated GEMM kernels for matvec and matmul with single shared axis.
- [ ] **P3.M5**: Broadcast + reduction WGSL kernels parameterised by uniforms.
- [ ] **P3.M6**: `tutorials/06_webgpu-acceleration.ipynb` Phase 3 close.

---

## Compliance / Validation

- **Verification**: every `webgpu::Backend` method's output agrees with `reference::Backend` to absolute tolerance 1e-5 for `float` on the self-hosted GPU runner.
- **Frequency**: per release tag.

---

## More Information

### Related ADRs

- [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md) — strategic choice of WebGPU.
- [ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) — Hexagonal lite that places this adapter in the DrivenAdapter row.
- [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) — refined positioning that admits production-grade adapters.
- [ADR-0011](0011-kernel-backend-port-api.md) — the port API this adapter implements.

### References

- [gpu.cpp / Answer.AI launch](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html)
- [Dawn (Google's WebGPU implementation)](https://dawn.googlesource.com/dawn)
- [wgpu-native](https://github.com/gfx-rs/wgpu-native) — fallback runtime
- [WebGPU specification](https://www.w3.org/TR/webgpu/)
- [WGSL specification](https://www.w3.org/TR/WGSL/)
- Phase 3 implementation plan: [`../../impl-plans/2026-05-11_phase-3-webgpu.md`](../../impl-plans/2026-05-11_phase-3-webgpu.md)
