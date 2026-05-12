---
status: Stable
owner: tensor
last-reviewed: 2026-05-12
---

# Backend performance comparison — reference vs Eigen vs WebGPU

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable; numbers from a desktop machine with NVIDIA RTX 3090 + Dawn 2026-04 (Vulkan) — see §2026-05-12 below. |
| Type         | Layer B — performance report (dated, append-only)              |
| Owner        | uyuutosa                                                       |
| Related      | [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md), [Phase 2.5 plan P2.5.M4](../impl-plans/2026-05-11_phase-2-5-backend-port-and-eigen.md), [`bench/bench.cpp`](../../bench/bench.cpp) |

## TL;DR

The reference adapter is correct, simple, and **slow on matmul**: 512³ GEMM takes ~570 ms on a stock developer laptop. That number is the headline reason the Phase 2.5 design committed to swappable backends — Eigen routes matmul through a BLAS-flavoured GEMM and is expected to be 10–100× faster on the same case. The Phase 3 WebGPU adapter will fast-path the same case on a desktop GPU.

This report is a **snapshot, not a goal**. It exists so future PRs that change kernels can be sanity-checked against a known baseline.

## How to reproduce

```bash
cmake --preset=default \
      -DTENSOR_BUILD_BENCH=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DTENSOR_KERNEL_BACKEND=eigen      # or reference
cmake --build --preset=default --target tensor_bench
./build/default/bench/tensor_bench 5      # 5 iterations per case
```

The bench prints a text table. Paste it into the *Numbers* section below when new measurements are collected.

## Benchmark cases

| Case                          | Description                                                                       |
| ----------------------------- | --------------------------------------------------------------------------------- |
| `add (1M elements)`           | Element-wise binary add over a flat 1,000,000-element `DynamicTensor<double>`.    |
| `matvec (1024 × 1024 × 1024)` | Rank-2 × rank-1 contraction (`y_i = Σ_j W_{ij} x_j`); W is 1024×1024, x is 1024.  |
| `matmul (512 × 512 × 512)`    | Rank-2 × rank-2 contraction (`C_{ik} = Σ_j A_{ij} B_{jk}`); all dims 512.         |

Random tensors seeded with `std::mt19937(42)` so different backends measure on byte-for-byte identical inputs.

## Numbers

### 2026-05-11 — reference baseline (g++ 12.3, -O2, Linux laptop)

```text
tensor benchmark — 5 iterations per case

  case                        backend       average
----------------------------------------------------------------
  add (1M elements)           reference        1.542 ms      (snapshot — 2-iter quick smoke)
  matvec (1024 × 1024 × 1024) reference        3.089 ms
  matmul (512 × 512 × 512)    reference      573.881 ms
```

> **Note**: this row was taken with `iters=2` for quick smoke. Re-running with `iters=5` and on quiescent hardware will tighten the numbers. The qualitative gap (single-threaded scalar loop vs. SIMD vs. GEMM) is what matters here.

### TBD — Eigen adapter (superseded by 2026-05-12 below)

The initial 2026-05-11 entry promised Eigen numbers TBD; those numbers were measured on 2026-05-12 — see the dated subsection below.

### TBD — WebGPU adapter (superseded by 2026-05-12 below)

The initial entry expected WebGPU numbers only "once Phase 3 P3.M3+ ships and a self-hosted GPU runner is available". P3.M3.2 / P3.M4.2 / P3.M5 shipped via PRs #60 / #61 / #62; the maintainer's RTX 3090 served as the verification machine. The numbers below.

### 2026-05-12 — three-backend comparison on RTX 3090 + Dawn 2026-04

Bench run on the maintainer's desktop after Phase 3 dispatch wiring landed. The same `bench/bench.cpp` runs all three backends; the WebGPU section adds a parallel f32 reference column so the GPU comparison is apples-to-apples (WebGPU's f32-only MVP per [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md) means the f64 baseline isn't a direct comparator).

**Hardware**: NVIDIA GeForce RTX 3090, driver 560.28.03, Vulkan via Dawn `20260410.140140`; CPU side g++ 12.3, -O3, Linux. 5 iterations per case after a single warm-up call.

```text
tensor benchmark — 5 iterations per case

  case                                backend     average
  ----------------------------------------------------------------
  add (1M elements)                   reference    1.100 ms    (f64)
  add (1M elements)                   eigen        0.721 ms    (f64)
  matvec (1024 × 1024 × 1024)         reference    3.075 ms    (f64)
  matvec (1024 × 1024 × 1024)         eigen        0.149 ms    (f64)
  matmul (512 × 512 × 512)            reference  571.986 ms    (f64)
  matmul (512 × 512 × 512)            eigen       13.638 ms    (f64)
  ────────────────────────────────────────────────────────────
  add (1M elements, f32)              reference    0.356 ms
  add (1M elements, f32)              webgpu       6.961 ms
  matvec (1024 × 1024 × 1024, f32)    reference    3.087 ms
  matvec (1024 × 1024 × 1024, f32)    webgpu       6.354 ms
  matmul (512 × 512 × 512, f32)       reference  469.940 ms
  matmul (512 × 512 × 512, f32)       webgpu       3.827 ms
```

Speedup table (reference = baseline 1.0×):

| Operation | reference (f64) | eigen (f64) | reference (f32) | **webgpu (f32, RTX 3090)** |
| --------- | --------------- | ----------- | --------------- | -------------------------- |
| `add (1M elements)` | 1.100 ms (1.0×) | 0.721 ms (**1.5×**) | 0.356 ms (3.1×) | 6.961 ms (**0.05×** — slower) |
| `matvec (1024²)` | 3.075 ms (1.0×) | 0.149 ms (**21×**) | 3.087 ms (1.0×) | 6.354 ms (**0.48×** — slower) |
| `matmul (512³)` | 571.986 ms (1.0×) | 13.638 ms (**42×**) | 469.940 ms (1.2×) | 3.827 ms (**149×**) |

Reading the numbers:

- **Eigen matmul wins by 42×** — that's the BLAS-flavoured tiled GEMM ADR-0011 anticipated, exactly as designed.
- **Eigen matvec wins by 21×** — same SIMD GEMV story.
- **Eigen add wins by 1.5×** — modest SIMD speedup over a scalar loop; memory bandwidth dominates.
- **WebGPU matmul wins by 149×** vs reference and **3.6× over Eigen** at this size. This is the canonical Phase 3 pitch from [tutorial 06](../../tutorials/06_webgpu-acceleration.ipynb) §5: at compute-bound sizes the RTX 3090 makes the host↔device round-trip irrelevant.
- **WebGPU add and matvec lose to CPU.** For 1M-element add (4 MB per operand, ~12 MB round-trip), PCIe transfer is ~5 ms each way — the GPU compute (<0.1 ms) is dwarfed. For matvec at 1024², it's the same story: 4 MB W, 4 KB x, 4 KB out, but the kernel only does 1M multiply-add — too little compute to amortise the transfer. Tutorial 06 §5 documents this honestly; the educational takeaway is "GPU wins when compute >> transfer", not "GPU is always faster".

Reproduction: `cmake --preset=default -DTENSOR_BUILD_BENCH=ON -DTENSOR_KERNEL_BACKEND=webgpu -DCMAKE_PREFIX_PATH=<vcpkg>/x64-linux && cmake --build --target tensor_bench && ./build/.../tensor_bench 5`.

**One caveat the numbers underline.** Reference f32 (0.356 ms add, 469.940 ms matmul) is significantly faster than reference f64 because the scalar loops are cache-friendly at half the byte volume. This is the same loop body — only the element type changes. It's a useful reminder that "reference" is not slow because it's reference; it's slow on matmul because it's a triple nested loop without tiling, and the type-related win comes for free. Future Phase 1.5+ profile work (discussion-points Axis C) could close some of the gap if a contributor felt motivated.

## How this report is supposed to evolve

This is a Layer B report. The "Stable (reference baseline)" status is interim; once Eigen and WebGPU columns are filled in, the file moves to fully Stable. When the kernels change in a way that should move numbers (a new GEMM tiling, a Eigen version bump, a WebGPU shader rewrite), write a **new dated report** that links back to this one rather than editing the body — Layer B is append-only.

## References

- [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md) — port API the bench measures.
- [Phase 2.5 impl-plan](../impl-plans/2026-05-11_phase-2-5-backend-port-and-eigen.md) — P2.5.M4 is the planning anchor for this report.
- [bench/bench.cpp](../../bench/bench.cpp) — the bench source.
- [tutorials/08_swappable-backends.ipynb](../../tutorials/08_swappable-backends.ipynb) — narrative companion for the same comparison.
