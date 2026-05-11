# third_party/gpu_cpp

> **2026-05-12 — Superseded by [ADR-0016](../../docs/arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md).** This vendored snapshot is no longer load-bearing. gpu.cpp@0.2.0's async-callback API is 14 months behind Dawn 2026-04 and does not compile against the current vcpkg Dawn port (see the [discovery report](../../docs/reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md)). The project now talks to Dawn directly via Dawn's own `webgpu_cpp.h` (Google-maintained, always synchronised with the linked Dawn binary). The directory is targeted for removal once Stage 3 (P3.M3.2 + P3.M4.2 dispatch wiring) lands.

Vendored snapshot of [AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) at tag `0.2.0` (commit `9a42592a0b`, 2025-02-10).

This directory contains the single public header `gpu.hpp` and the upstream `LICENSE` (Apache-2.0). Vendoring instead of consuming upstream as a dependency follows [ADR-0014](../../docs/arc42/09-decisions/0014-external-substrate-strategy.md) — gpu.cpp is bus-factor 1 (one primary maintainer, no release between 2024-08 and 2025-02) and has no vcpkg port, so we hold our own pinned copy under our own commit discipline.

See [`VENDORED_FROM`](./VENDORED_FROM) for the upstream pin, the re-vendor procedure, and the list of modifications (none).

## What this header wraps

`gpu.hpp` is a thin C++ wrapper around the WebGPU C API (`webgpu/webgpu.h`). At runtime the WebGPU implementation comes from Dawn (Google's WebGPU) — pulled in by vcpkg as `dawn` per ADR-0014. The header itself is implementation-free; it expects `webgpu/webgpu.h` to be on the include path and a Dawn-compatible implementation to be linked.

## Where this header is used

`include/tensor/core/backend/webgpu.hpp` (the third `KernelBackend` adapter, per ADR-0012). As of Phase 3 P3.M2, the WebGPU adapter is a stub that delegates to `reference::Backend` and does not yet include `gpu.hpp`. P3.M3 (element-wise WGSL kernels) is the milestone that activates this header — element-wise kernel bodies are emitted as WGSL strings and dispatched through gpu.cpp's `createKernel` / `dispatch` API.

## License

This directory's content is Apache-2.0 licensed (upstream's terms). The rest of the `tensor` repository is MIT-licensed — Apache-2.0 grants are compatible with MIT consumers.
