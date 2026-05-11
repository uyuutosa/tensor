---
status: Stable (reference baseline; Eigen numbers TBD)
owner: tensor
last-reviewed: 2026-05-11
---

# Backend performance comparison — reference vs Eigen (vs WebGPU TBD)

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable for the reference baseline; Eigen and WebGPU numbers TBD on a real machine. |
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

### TBD — Eigen adapter

To be filled in after a clean local run with `-DTENSOR_KERNEL_BACKEND=eigen`:

```text
  case                        backend       average
----------------------------------------------------------------
  add (1M elements)           eigen          TBD ms
  matvec (1024 × 1024 × 1024) eigen          TBD ms
  matmul (512 × 512 × 512)    eigen          TBD ms
```

Expected qualitative result based on Eigen's documented behaviour:

- `add` — comparable to reference (memory-bandwidth-bound; the SIMD win on a single core is modest).
- `matvec` — modest speedup (~2–5×) from SIMD; the loop is cache-friendly already.
- `matmul` — **large** speedup (~50–100×) because Eigen routes through a tiled BLAS-flavoured GEMM.

### TBD — WebGPU adapter (Phase 3)

To be filled in once Phase 3 P3.M3+ ships and a self-hosted GPU runner is available. The shape of the result is also expected qualitatively:

- Small ops: **slower** than reference (PCIe round trip dominates).
- Medium / large matmul: **fast** (a desktop GPU should beat Eigen on 1024+ shapes).

## How this report is supposed to evolve

This is a Layer B report. The "Stable (reference baseline)" status is interim; once Eigen and WebGPU columns are filled in, the file moves to fully Stable. When the kernels change in a way that should move numbers (a new GEMM tiling, a Eigen version bump, a WebGPU shader rewrite), write a **new dated report** that links back to this one rather than editing the body — Layer B is append-only.

## References

- [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md) — port API the bench measures.
- [Phase 2.5 impl-plan](../impl-plans/2026-05-11_phase-2-5-backend-port-and-eigen.md) — P2.5.M4 is the planning anchor for this report.
- [bench/bench.cpp](../../bench/bench.cpp) — the bench source.
- [tutorials/08_swappable-backends.ipynb](../../tutorials/08_swappable-backends.ipynb) — narrative companion for the same comparison.
