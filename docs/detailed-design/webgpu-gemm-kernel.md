---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `webgpu::Backend` GEMM kernel (P3.M4) — detailed design

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Draft (P3.M4.1 ships the WGSL source; P3.M4.2 ships the dispatch wiring). |
| Type         | Detailed Design (Template 3, arc42 §5 zoom-in for `tensor::core::backend::webgpu`) |
| Owner        | uyuutosa                                                       |
| Realises ADR | [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md), [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md), [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md) |
| Related plan | [Phase 3 impl-plan](../impl-plans/2026-05-11_phase-3-webgpu.md) |
| Sibling design | [`./webgpu-element-wise-kernels.md`](./webgpu-element-wise-kernels.md) (P3.M3) |
| Cited from   | [book/intro.md](../../book/intro.md) "How to cite this work" (ADR-0013) |

## Purpose

This document specifies how the GEMM-family `contract` method on `webgpu::Backend` will dispatch to a tiled WGSL kernel via [gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) on top of Dawn. It is the design that fills in the `TODO (P3.M4)` marker currently at [`include/tensor/core/backend/webgpu.hpp`](../../include/tensor/core/backend/webgpu.hpp).

Like the element-wise design, this splits to keep CI tractable while no self-hosted GPU runner exists:

- **P3.M4.1 (shipped, this PR)** — Tiled GEMM WGSL kernel source committed at [`include/tensor/core/backend/webgpu_wgsl.hpp`](../../include/tensor/core/backend/webgpu_wgsl.hpp) as the constant `kGemmF32`, with tile dimensions (`kGemmTileM` / `kGemmTileN` / `kGemmTileK` = 16/16/16) exposed alongside. Inert code, citable, text-validated in [`tests/test_webgpu_wgsl.cpp`](../../tests/test_webgpu_wgsl.cpp).
- **P3.M4.2 (deferred)** — Dispatch wiring (this document's §3) replaces the `contract` delegation with a real `gpu.cpp` call sequence. Same preconditions as P3.M3.2: vcpkg baseline includes `dawn`, self-hosted GPU runner exists.

## 1. Scope

In scope:

- **Matrix–matrix product (matmul)**: rank-2 × rank-2 with one shared axis, producing rank-2. Inputs in row-major flat layout (matches `DynamicTensor<T>` semantics).
- **Matrix–vector product (matvec)**: rank-2 × rank-1 with one shared axis, producing rank-1. Handled by the same kernel with N = 1 — see §4 for the inefficiency note.
- `f32` precision only (ADR-0012). Other precisions delegate to `reference::Backend`.
- Single shared axis. The Eigen adapter already constrains itself this way ([include/tensor/core/backend/eigen.hpp](../../include/tensor/core/backend/eigen.hpp)); the WebGPU adapter mirrors that scope.

Out of scope:

- **Higher-rank contraction** (e.g. rank-3 × rank-3 with one shared axis) — delegates to reference. A general-rank GPU kernel is a Phase 4+ research item.
- **Multiple shared axes** (full Einstein-convention contraction) — delegates to reference.
- **Batched GEMM** (rank-3 × rank-3 where one axis batches) — Phase 4+.
- **Asymmetric tile sizes** (e.g. TILE_M ≠ TILE_N). The 16 × 16 square tile is the canonical educational choice; per-architecture tuning is Phase 5+.

## 2. WGSL kernel shape

The committed source ([`webgpu_wgsl.hpp::kGemmF32`](../../include/tensor/core/backend/webgpu_wgsl.hpp)) implements:

```
out[m, n] = Σ_k a[m, k] * b[k, n]
```

with `m ∈ [0, M)`, `k ∈ [0, K)`, `n ∈ [0, N)` and row-major flat indexing.

### Tile layout

The workgroup is 2-D, 16 × 16 = 256 threads (matching the canonical Dawn workgroup size used by the element-wise kernels per `kDefaultWorkgroupSize`). Each thread computes **one output cell** `out[row, col]` where `row = global_invocation_id.y` and `col = global_invocation_id.x`.

Two workgroup-shared arrays cache the A and B tiles for the current K slab:

- `shA[TILE_M][TILE_K]` — a 16 × 16 tile of A.
- `shB[TILE_K][TILE_N]` — a 16 × 16 tile of B.

The outer loop iterates `nTiles = ceil(K / TILE_K)` times. Each iteration:

1. Each thread cooperatively loads exactly one cell of `shA` and one cell of `shB`. Out-of-range cells become `0.0` so the inner-product loop runs without per-cell bounds checks.
2. `workgroupBarrier()` — wait for all loads to complete.
3. Inner loop accumulates `TILE_K` partial products into a per-thread `acc`.
4. `workgroupBarrier()` — wait for all threads to finish using the current tile before the next iteration overwrites it.

After all tiles, threads whose `(row, col)` is in-bounds write `out[row * N + col] = acc`.

### Why this tile shape

- **16 × 16 = 256 threads** matches the existing `kDefaultWorkgroupSize` for the element-wise kernels, so all of the WebGPU adapter's kernels run with a consistent workgroup-size budget.
- **TILE_K = 16** balances shared-memory pressure against re-load count: 16 × 16 × 4 bytes per tile × 2 tiles = 2 KB shared memory — well within every Dawn-supported backend's per-workgroup budget (Vulkan/Metal/D3D12 all guarantee ≥ 16 KB).
- **Square tiles** are the standard pedagogical choice; tuning is a Phase 5+ research item once a real GPU runner exists to measure on.

### Why one kernel for matvec and matmul

The canonical-reference framing (ADR-0013) prefers one readable kernel over two specialised ones. A dedicated matvec kernel with a 1-D workgroup would extract more throughput for tall-skinny problems (N = 1) but doubles the surface area to read, test, and re-derive. The matvec inefficiency (15 / 16 of the threads in a 16-wide workgroup are idle when N = 1) is documented in §4 rather than engineered away; a P3.M4.3 PR can specialise later if a real benchmark demands it.

## 3. Dispatch wiring (P3.M4.2 — design)

The implementation strategy for `Backend::contract` follows the same shape as the element-wise design's §3, with two structural differences: (a) a uniform `Params` buffer carries M / N / K, and (b) the workgroup is 2-D so `totalWorkgroups` is computed in both X and Y.

```cpp
// in tensor::core::backend::webgpu

template <class T>
[[nodiscard]] DynamicTensor<T> Backend::contract(DynamicTensor<T> const& a,
                                                 DynamicTensor<T> const& b,
                                                 ContractPlan const& plan) const {
    if constexpr (!std::is_same_v<T, float>) {
        return ref_.contract(a, b, plan);
    } else {
        // Only the single-shared-axis matvec / matmul cases run on GPU;
        // anything else delegates to keep this design tractable (ADR-0012).
        if (!is_simple_matmul_or_matvec(a, b, plan)) {
            return ref_.contract(a, b, plan);
        }

        auto const M = extent_M(a, plan);  // helper resolves M from plan
        auto const K = extent_K(a, b, plan);
        auto const N = extent_N(b, plan);  // 1 in the matvec case

        // Same Context lifetime story as P3.M3.2 — see
        // docs/detailed-design/webgpu-element-wise-kernels.md §3.
        auto& ctx = ensure_context();

        // gpu::Shape values for buffer sizing only; not related to
        // tensor::core::Shape.
        auto gA   = gpu::createTensor(ctx, {M * K}, gpu::kf32);
        auto gB   = gpu::createTensor(ctx, {K * N}, gpu::kf32);
        auto gOut = gpu::createTensor(ctx, {M * N}, gpu::kf32);

        gpu::toGPU(ctx, a.data(), gA);
        gpu::toGPU(ctx, b.data(), gB);

        // Uniform `Params` mirrors the struct in kGemmF32:
        struct Params {
            std::uint32_t M_, N_, K_;
        } params{
            static_cast<std::uint32_t>(M),
            static_cast<std::uint32_t>(N),
            static_cast<std::uint32_t>(K),
        };

        gpu::KernelCode code{
            std::string{webgpu::wgsl::kGemmF32},
            // 2-D workgroup; gpu::KernelCode's `Shape` overload at
            // third_party/gpu_cpp/gpu.hpp:333 takes the 3D shape.
            gpu::Shape{webgpu::wgsl::kGemmTileN, webgpu::wgsl::kGemmTileM, 1},
            gpu::kf32
        };

        // totalWorkgroups is 2-D: X = ceil(N / TILE_N), Y = ceil(M / TILE_M).
        gpu::Bindings bindings{gA, gB, gOut};
        gpu::Shape totalWorkgroups{
            gpu::cdiv(N, webgpu::wgsl::kGemmTileN),
            gpu::cdiv(M, webgpu::wgsl::kGemmTileM),
            1
        };
        auto kernel = gpu::createKernel(ctx, code, bindings, totalWorkgroups, params);
        // gpu.hpp:1392 (ParamsType overload)

        std::promise<void> p;
        auto f = p.get_future();
        gpu::dispatchKernel(ctx, kernel, p);
        gpu::wait(ctx, f);

        DynamicTensor<T> out{output_shape(a, b, plan)};
        gpu::toCPU(ctx, gOut, out.data(), M * N * sizeof(T));

        return out;
    }
}
```

Two small helpers (`is_simple_matmul_or_matvec`, `extent_M / K / N`, `output_shape`) inspect the `ContractPlan` to decide whether the call is in scope and to resolve the M / N / K extents. They mirror the equivalent logic the Eigen adapter already uses.

## 4. Performance expectations

### Educational benchmark target

For a 512 × 512 × 512 matmul on a modern integrated GPU:

- Reference baseline ([P2.5.M4 perf report](../reports/2026-05-11_backend-performance-comparison.md)): ~574 ms.
- Expected WebGPU: ~5-15 ms (compute-bound; transfer round-trip is ~12 ms each way for 1 MB, but the GEMM compute is ~50× more than transfer at this size).
- Expected Eigen GEMM on a modern CPU: ~5-20 ms (BLAS-quality matmul; varies by core count).

So at the 512³ scale, WebGPU is **competitive with optimised CPU BLAS** while costing the user no proprietary toolchain. This is the canonical Phase 3 pitch — a learner who runs `cmake --preset=default -DTENSOR_KERNEL_BACKEND=webgpu` sees the same matmul go from ~600 ms (reference) to ~10 ms.

### Matvec inefficiency

When N = 1, a 16-wide workgroup runs 15 idle threads per row. The matvec at M = 1024, K = 1024 thus achieves roughly 1/16 of peak. Real numbers will land in the [Phase 3 perf report](../reports/) once a self-hosted GPU runner exists; until then, the inefficiency is documented honestly here rather than engineered away.

## 5. Test plan

### P3.M4.1 (this PR)

- `tests/test_webgpu_wgsl.cpp` asserts:
  - `kGemmTileM` / `kGemmTileN` / `kGemmTileK` = 16, and the WGSL source's `const TILE_*` lines agree.
  - The kernel declares four bindings (a / b / out / Params) and `var<uniform>`.
  - The compute entry point is `@compute @workgroup_size(TILE_N, TILE_M, 1)`.
  - Both `var<workgroup> shA / shB` declarations are present.
  - Two `workgroupBarrier()` calls are present (load fence + compute fence).
  - The inner-product line and the bounded write line appear.

### P3.M4.2 (deferred PR with GPU runner)

- The existing [`tests/test_webgpu_backend.cpp::matvec`](../../tests/test_webgpu_backend.cpp) cross-validation already covers the matvec case. Extend it with a matmul case (3 × 3 × 3, then 64 × 64 × 64) once dispatch is wired.
- Tolerance softens from `1e-9` (CPU vs CPU) to `1e-3` for `float` matmul (per ADR-0012, allowing for reduced-precision accumulation).
- A new bench scenario lands under `bench/` covering 512 × 512 × 512 matmul on reference / Eigen / WebGPU.

## 6. Cross-references

- WGSL source: [`include/tensor/core/backend/webgpu_wgsl.hpp::kGemmF32`](../../include/tensor/core/backend/webgpu_wgsl.hpp)
- Sibling design (element-wise): [`./webgpu-element-wise-kernels.md`](./webgpu-element-wise-kernels.md)
- gpu.cpp vendored header: [`third_party/gpu_cpp/gpu.hpp`](../../third_party/gpu_cpp/gpu.hpp) — line refs above
- Phase 3 impl-plan: [`../impl-plans/2026-05-11_phase-3-webgpu.md`](../impl-plans/2026-05-11_phase-3-webgpu.md)
- [ADR-0011 — KernelBackend port API](../arc42/09-decisions/0011-kernel-backend-port-api.md)
- [ADR-0012 — WebGPU adapter implementation design](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md)
- [ADR-0014 — External-substrate strategy](../arc42/09-decisions/0014-external-substrate-strategy.md)
- [ADR-0016 — Substrate refinement: drop gpu.cpp, talk to Dawn directly](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) — the gpu.cpp references above are now historical; Dawn `webgpu_cpp.h` is the in-use API surface.

## 7. Future work

- **Non-simple-GEMM `contract` on WGSL** — current path handles 2D-matmul; multi-axis `contract` (the `tensor::core::contract` with arbitrary shared-label sets) still routes to reference. Once the broadcast kernels stabilise, the same dispatch pattern can lift `contract`'s general case.
- **Tile-size tuning** — the current tile-size constants (16 × 16) were chosen for RTX 3090 + Vulkan; auto-tuning per device would tighten the QF-2 envelope.
- **`f16` GEMM** — Mixed-precision GEMM with `f16` inputs and `f32` accumulator is the production-shape ML pattern; reachable from the existing `F32` kernel by parameterising the input type. Defer until `DynamicTensor<half>` lands in the Domain.
- **Subgroup-shuffle reductions** — modern GPUs support warp-level shuffle; the GEMM kernel's k-loop reduction can use subgroup intrinsics for a measurable speedup on contemporary hardware (Ampere+, RDNA2+). Track at the next perf-comparison report cycle.
