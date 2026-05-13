---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `webgpu::Backend` element-wise kernels (P3.M3) — detailed design

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Draft (P3.M3.1 ships the WGSL sources; P3.M3.2 ships the dispatch wiring). |
| Type         | Detailed Design (Template 3, arc42 §5 zoom-in for `tensor::core::backend::webgpu`) |
| Owner        | uyuutosa                                                       |
| Realises ADR | [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md), [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md), [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md) |
| Related plan | [Phase 3 impl-plan](../impl-plans/2026-05-11_phase-3-webgpu.md) |
| Building block | [tensor::core::backend::webgpu](../arc42/05-building-blocks/overview.md) — third `KernelBackend` adapter |
| Cited from   | [book/intro.md](../../book/intro.md) "How to cite this work" (ADR-0013) |

## Purpose

This document specifies how the four element-wise binary operators (`add`, `sub`, `mul`, `div`) on `webgpu::Backend` will dispatch to GPU compute kernels via [gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) on top of Dawn. It is the design that fills in the `TODO (P3.M3)` markers currently in [`include/tensor/core/backend/webgpu.hpp`](../../include/tensor/core/backend/webgpu.hpp).

The design is split into two slices to keep CI tractable while no self-hosted GPU runner exists:

- **P3.M3.1 (shipped, PR #43)** — Binary WGSL kernel sources (`kAddF32` / `kSubF32` / `kMulF32` / `kDivF32`) committed under [`include/tensor/core/backend/webgpu_wgsl.hpp`](../../include/tensor/core/backend/webgpu_wgsl.hpp) as `constexpr std::string_view` constants. Inert code — citable, reviewable, well-formedness-tested in [`tests/test_webgpu_wgsl.cpp`](../../tests/test_webgpu_wgsl.cpp). The WebGPU stub still delegates to `reference::Backend`.
- **P3.M3.3 (shipped, this PR)** — Unary WGSL kernel sources (`kExpF32` / `kLogF32` / `kReluF32` / `kNegF32`) — same template as P3.M3.1 but with two storage bindings (input + output) instead of three. Brings the WebGPU adapter's element-wise *source* surface to feature parity with the reference + Eigen adapters (8 ops: 4 binary + 4 unary).
- **P3.M3.2 (deferred)** — Dispatch wiring (this document's §3 binary + §3.1 unary) replaces each delegation with a real `gpu.cpp` call sequence. Precondition: vcpkg manifest baseline bumped to one that contains the `dawn` port (per ADR-0014); self-hosted GitHub Actions runner with a Dawn-compatible GPU; ADR-0012's compile-only-CI policy upgraded with a runner that can run the cross-validation test against real silicon.

## 1. Scope

In scope:

- Four element-wise binary operators that take two same-shape `DynamicTensor<T>` and return a same-shape `DynamicTensor<T>`: `add`, `sub`, `mul`, `div`.
- Four element-wise unary operators: `exp`, `log`, `relu`, `neg` (P3.M3.3).
- `f32` precision only (per ADR-0012). Other precisions delegate to `reference::Backend`.
- Inputs and outputs are flat (rank does not matter for element-wise) — the kernel iterates over `arrayLength(&out)`.

Out of scope (deferred to P3.M4 / P3.M5):

- Broadcast element-wise (`broadcast_add`, etc.) — same flat dispatch model but with a uniform buffer carrying the `BroadcastPlan`.
- Tiled GEMM (`contract`) — P3.M4. Separate design.
- Tree-reduction (`reduce_sum`) and scatter-add (`unbroadcast`) — P3.M5. Separate designs.

## 2. WGSL kernel shape

Every element-wise binary kernel under [`include/tensor/core/backend/webgpu_wgsl.hpp`](../../include/tensor/core/backend/webgpu_wgsl.hpp) follows the same structure:

```wgsl
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read>       b   : array<{{precision}}>;
@group(0) @binding(2) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = a[i] <OP> b[i];
}
```

Placeholders use the `{{workgroupSize}}` / `{{precision}}` convention originally from gpu.cpp's `KernelCode` class. As of ADR-0016 (2026-05-12) the project does not use gpu.cpp itself; the substitution is now done by [`tensor::core::backend::webgpu::detail::substitute_wgsl`](../../include/tensor/core/backend/webgpu_detail/dispatch.hpp) which performs the placeholder replacement at dispatch time before handing the source to `wgpu::Device::CreateShaderModule`.

`{{workgroupSize}}` defaults to 256 (the canonical Dawn choice — fits in a single subgroup on most modern GPUs and balances occupancy against shared-memory pressure). `{{precision}}` is `f32` in Phase 3 MVP; `f16` is a Phase 4 follow-up.

### 2.1 Unary kernel shape (P3.M3.3)

Unary kernels follow the same template with one fewer binding:

```wgsl
@group(0) @binding(0) var<storage, read>       a   : array<{{precision}}>;
@group(0) @binding(1) var<storage, read_write> out : array<{{precision}}>;

@compute @workgroup_size({{workgroupSize}})
fn main(@builtin(global_invocation_id) gid: vec3<u32>) {
    let i = gid.x;
    if (i >= arrayLength(&out)) { return; }
    out[i] = <OP(a[i])>;
}
```

Where `<OP(a[i])>` is:

| Operator | Body | WGSL built-in |
| -------- | ---- | ------------- |
| `exp`    | `exp(a[i])`        | [`exp`](https://www.w3.org/TR/WGSL/#exp-builtin) |
| `log`    | `log(a[i])`        | [`log`](https://www.w3.org/TR/WGSL/#log-builtin) (natural log) |
| `relu`   | `max(a[i], 0.0)`   | [`max`](https://www.w3.org/TR/WGSL/#max-float-builtin) — single GPU instruction on Vulkan / Metal / D3D12 |
| `neg`    | `-a[i]`            | unary minus |

ReLU is expressed via `max` rather than a branch because branching prevents some optimisations on uniformly-shaped data. The literal `0.0` is implicitly typed by the operand precision.

## 3. Dispatch wiring (P3.M3.2 — design)

The implementation strategy for `Backend::add` is the template every other element-wise binary follows by `sed` substitution. The pseudo-code below references gpu.cpp APIs by file location so reviewers can verify each call against vendored upstream:

```cpp
// in tensor::core::backend::webgpu

template <class T>
[[nodiscard]] DynamicTensor<T> Backend::add(DynamicTensor<T> const& a,
                                            DynamicTensor<T> const& b) const {
    if constexpr (!std::is_same_v<T, float>) {
        // f32-only MVP; delegate non-float to reference (ADR-0012).
        return ref_.add(a, b);
    } else {
        assert(a.shape() == b.shape() && "add: shape mismatch");
        auto const n = a.size();

        // 1. Lazily build the gpu.cpp Context the first time any GPU op runs.
        //    `ctx_` is a mutable member of Backend (or static-thread-local;
        //    TBD by P3.M3.2 PR review). Created via gpu::createContext()
        //    (third_party/gpu_cpp/gpu.hpp:774).
        auto& ctx = ensure_context();

        // 2. Allocate GPU tensors for inputs + output. shape() is the
        //    gpu::Shape (third_party/gpu_cpp/gpu.hpp:55), unrelated to our
        //    tensor::core::DynamicShape — it's just a fixed-rank array of
        //    extents that gpu.cpp uses for buffer sizing.
        auto gA   = gpu::createTensor(ctx, {n}, gpu::kf32);  // gpu.hpp:615
        auto gB   = gpu::createTensor(ctx, {n}, gpu::kf32);
        auto gOut = gpu::createTensor(ctx, {n}, gpu::kf32);

        // 3. Upload inputs (host → GPU).
        gpu::toGPU(ctx, a.data(), gA);  // gpu.hpp:1112
        gpu::toGPU(ctx, b.data(), gB);

        // 4. Build the kernel from the WGSL source + workgroup size.
        gpu::KernelCode code{
            std::string{webgpu::wgsl::kAddF32},
            webgpu::wgsl::kDefaultWorkgroupSize,
            gpu::kf32
        };

        // 5. Create the kernel with the three bindings (a, b, out) and
        //    the total-workgroups shape = ceil(n / workgroupSize).
        gpu::Bindings bindings{gA, gB, gOut};
        gpu::Shape totalWorkgroups{gpu::cdiv(n, webgpu::wgsl::kDefaultWorkgroupSize), 1, 1};
        auto kernel = gpu::createKernel(ctx, code, bindings, totalWorkgroups);
        // gpu.hpp:1392

        // 6. Dispatch + wait. P3.M3.2 uses blocking sync per ADR-0012;
        //    async batching is a Phase 4+ optimisation.
        std::promise<void> p;
        auto f = p.get_future();
        gpu::dispatchKernel(ctx, kernel, p);  // gpu.hpp:1429
        gpu::wait(ctx, f);                     // gpu.hpp:978

        // 7. Download output (GPU → host) into a same-shape DynamicTensor.
        DynamicTensor<T> out{a.shape()};
        gpu::toCPU(ctx, gOut, out.data(), n * sizeof(T));  // gpu.hpp:997

        return out;
    }
}
```

Substitute `kAddF32` → `kSubF32` / `kMulF32` / `kDivF32` for the other three binary operators.

### 3.1 Unary dispatch sequence

The unary dispatch sequence is identical to §3 with two changes:

1. **One input tensor** — only `gA` is uploaded; no `gB`.
2. **Two-binding kernel** — `gpu::Bindings bindings{gA, gOut};` instead of three.

Replace `kAddF32` with `kExpF32` / `kLogF32` / `kReluF32` / `kNegF32` per operator. Everything else (Context, totalWorkgroups, blocking sync, download) is identical.

### Context lifetime

`ensure_context()` is the single design choice this document leaves intentionally open for the P3.M3.2 PR. Two options:

- **Per-Backend instance** — `Backend` holds a `mutable std::optional<gpu::Context> ctx_;` and `ensure_context()` initialises it on first call. Pro: deterministic lifetime tied to Backend lifetime. Con: each `Backend` allocates a Dawn context (heavy if many `Backend` instances exist).
- **Thread-local static** — `static thread_local gpu::Context ctx;` inside `ensure_context()`. Pro: one Dawn context per thread regardless of Backend count. Con: lifetime extends to thread teardown, which under xeus-cpp may surprise the user.

P3.M3.2 PR picks one; this design accepts either.

### Buffer ownership

ADR-0012 §Decision Outcome point 3 says GPU buffers stay opaque inside `gpu::Backend`. The design above honors that: `gA / gB / gOut` are local variables that go out of scope at function exit; gpu.cpp's `TensorPool` (gpu.hpp:190) reclaims them when the Context destructs.

### Synchronisation

ADR-0012 §Decision Outcome point 4 picked blocking-sync-per-call for MVP. Step 6 implements that with `std::promise` + `gpu::wait`. Async batching is deferred to Phase 4+ as an optimisation that does not change the public surface.

## 4. Test plan

### P3.M3.1 + P3.M3.3 (shipped)

- `tests/test_webgpu_wgsl.cpp` asserts the eight WGSL constants are non-empty.
- Asserts each binary kernel declares three storage bindings, each unary kernel declares exactly two.
- Asserts each contains the expected `@compute @workgroup_size(` declaration.
- Asserts each contains the expected `out[i] = ...` body matching the operator.
- No shader compilation; no Dawn linkage required.

### P3.M3.2 (deferred PR with GPU runner)

- The existing [`tests/test_webgpu_backend.cpp`](../../tests/test_webgpu_backend.cpp) cross-validation suite already iterates the four operators and asserts `webgpu` and `reference` outputs agree within `kTol = 1e-9`. When P3.M3.2 lands, the same suite verifies real GPU dispatch instead of CPU delegation.
- Tolerance softens from `1e-9` to `1e-5` for `float` per ADR-0012 §Decision Outcome point 5.
- A new self-hosted GitHub Actions runner with a Dawn-compatible GPU runs the cross-validation suite on every PR that touches `include/tensor/core/backend/webgpu.hpp` or `include/tensor/core/backend/webgpu_wgsl.hpp`.

## 5. Performance expectations

For a 1M-element add (`DynamicTensor<float>` with shape `{1024*1024}`) on a modern integrated GPU:

- Reference baseline (from [P2.5.M4 bench report](../reports/2026-05-11_backend-performance-comparison.md)): single-threaded CPU add ≈ ~1 ms.
- Expected WebGPU add: dominated by host↔device transfer (~10 ms upload + ~10 ms download for 4 MB each way over PCIe; numbers vary by integrated vs discrete). Actual compute is <0.1 ms.

So for **element-wise binary kernels at this scale, GPU loses to CPU on round-trip alone.** Tutorial 06 will document this honestly as the canonical-reference-quality lesson it is. GEMM (P3.M4) is where GPU wins by 50–100× over the same baseline, because compute amortises the transfer.

## 6. Cross-references

- WGSL sources: [`include/tensor/core/backend/webgpu_wgsl.hpp`](../../include/tensor/core/backend/webgpu_wgsl.hpp)
- Stub adapter (current `add/sub/mul/div` bodies): [`include/tensor/core/backend/webgpu.hpp`](../../include/tensor/core/backend/webgpu.hpp)
- Dawn C++ API: `<webgpu/webgpu_cpp.h>` (shipped by the `dawn` vcpkg port) — used directly by the project per [ADR-0016](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md). The original draft of this document referenced gpu.cpp's `gpu.hpp` by line number; those references are preserved historically in the §3 dispatch-sequence pseudo-code as a guide to the API surface gpu.cpp was wrapping (the underlying `wgpu*` C ABI is what we now use directly through `webgpu_cpp.h`'s RAII types).
- Phase 3 impl-plan: [`docs/impl-plans/2026-05-11_phase-3-webgpu.md`](../impl-plans/2026-05-11_phase-3-webgpu.md)
- [ADR-0012 — WebGPU adapter implementation design](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md)
- [ADR-0014 — External-substrate strategy](../arc42/09-decisions/0014-external-substrate-strategy.md)
- Python consumer (Phase 6.5): [`./python-sdk-binding-surface.md`](./python-sdk-binding-surface.md) — `pip install tensor-named-axis[webgpu]` brings this adapter's `_tensor_native_webgpu.so` flavour.

## 7. Future work

- **`reduce_sum` on WGSL** — currently delegated to reference. The subgroup-shuffle pattern from WGSL 1.0+ makes single-axis reduction efficient; add as the 13th method to dispatch on real GPU.
- **`f16` element-wise variants** — the existing kernel sources are `F32`-suffixed; adding `F16` siblings is mechanical once the Domain `DynamicTensor<half>` (or `_Float16`) lands. Currently out-of-scope per ADR-0012.
- **Multi-device dispatch** — the current adapter holds one `wgpu::Device` per `Backend` instance. Multi-GPU dispatch (e.g. an instance per GPU) would let `tensor.set_backend("webgpu", device_index=N)` route across GPUs; deferred until a measured signal asks.
