---
status: Stable
owner: tensor
last-reviewed: 2026-05-12
---

# gpu.cpp@0.2.0 vs Dawn@20260410.140140 ABI drift — and raw-Dawn smoke success on RTX 3090

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable — discovery captured, raw-Dawn substrate verified working. |
| Type         | Layer B — discovery / verification report (dated, append-only)  |
| Owner        | uyuutosa                                                       |
| Triggered by | maintainer authorised GPU testing on the local RTX 3090 (2026-05-11 / 2026-05-12). |
| Triggers     | [ADR-0016](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) — refinement of ADR-0014's gpu.cpp choice. |
| Related      | [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md), [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md), [Phase 3 impl-plan](../impl-plans/2026-05-11_phase-3-webgpu.md). |

## TL;DR

- **gpu.cpp@0.2.0 (the version we vendored under ADR-0014) is incompatible with Dawn@20260410.140140 (the version in the current vcpkg port).** Dawn migrated all async callbacks to a `*CallbackInfo` struct pattern around 2025-Q3-Q4; gpu.cpp@0.2.0 (Aug 2024 / Feb 2025) predates that migration. The vendored `gpu.hpp` has at least six call sites (`wgpuInstanceRequestAdapter`, `wgpuAdapterRequestDevice`, `wgpuDeviceSetUncapturedErrorCallback`, `wgpuShaderModuleGetCompilationInfo`, `wgpuQueueOnSubmittedWorkDone`, plus a `WGPUStringView` argument in `WGPUQueueWorkDoneCallback`) that fail to compile against current Dawn.
- **Raw-Dawn (talking to `webgpu.h` directly) works perfectly.** A ~280-line standalone smoke executes the project's [`kAddF32`](../../include/tensor/core/backend/webgpu_wgsl.hpp) WGSL source (PR #43) on the local NVIDIA GeForce RTX 3090 via Vulkan and produces **0 mismatches, max |err| = 0** against the CPU reference for N=1024 element-wise add.
- **Project-level conclusion**: the gpu.cpp wrapper layer is not load-bearing for our use case. We can talk to Dawn directly via the C ABI (or Dawn's own `webgpu_cpp.h` RAII wrapper, maintained by Google as part of the Dawn release) and skip gpu.cpp entirely. [ADR-0016](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) proposes this substrate refinement.

## 1. What we did

The maintainer authorised GPU testing on the local environment (RTX 3090, NVIDIA driver 560.28.03, CUDA 12.6). Per ADR-0014 the canonical path is **Dawn via vcpkg + gpu.cpp wrapper**. We followed it:

1. Installed vcpkg locally at `~/vcpkg` (commit `2026-04-08`).
2. Ran `~/vcpkg/vcpkg install 'dawn[core,vulkan]' --triplet x64-linux`. First attempt (`dawn` default features) failed because `dawn[x11]` requires `Xlib-xcb.h` from `libx11-xcb-dev` which isn't installed system-wide — for compute-only we don't need X11/GL/GLES, so the second attempt with `[core,vulkan]` succeeded in 4.2 minutes (with binary cache).
3. Wrote a standalone smoke `build/gpu-smoke/smoke_add.cpp` that uses the **vendored** `third_party/gpu_cpp/gpu.hpp` to dispatch [`webgpu_wgsl.hpp::kAddF32`](../../include/tensor/core/backend/webgpu_wgsl.hpp) on the GPU.
4. Found that `gpu.hpp` requires additional headers we hadn't vendored (`numeric_types/half.hpp`, `utils/logging.hpp`, `utils/array_utils.hpp`). Vendored those at upstream tag `0.2.0`.
5. Re-tried the build. It failed with the compiler complaining that gpu.cpp's lambda function pointers don't convert to Dawn's struct types — the API drift described in §2.
6. Wrote a **second** smoke `build/gpu-smoke/smoke_dawn_instance.cpp` that bypasses gpu.cpp and talks to `webgpu.h` directly. It succeeded in detecting the RTX 3090 via Vulkan.
7. Extended to a full compute smoke `build/gpu-smoke/smoke_dawn_add.cpp` that allocates buffers, uploads inputs, compiles `kAddF32` (with the `{{precision}}` / `{{workgroupSize}}` placeholders substituted by the smoke), dispatches with the canonical 256-thread workgroup, copies the result back via a staging buffer, and verifies element-wise.
8. **`./smoke_dawn_add` on the RTX 3090** prints:

```
kAddF32 dispatched on RTX 3090 via Dawn:
  N=1024  workgroup=256  totalGroups=4
  mismatches: 0 / 1024
  max |err|:  0
  result:     PASS
```

Bit-exact match against `a[i] + b[i]` for all 1024 elements.

## 2. The gpu.cpp ABI drift in detail

gpu.cpp@0.2.0 was tagged 2025-02-10. Dawn release `20260410.140140` is from 2026-04-10 — fourteen months newer. In that interval Dawn migrated *all* of its async-callback surface to the **callback-info struct** pattern. Specifically:

- **Before** (gpu.cpp@0.2.0's mental model):
  ```c
  typedef void (*WGPUQueueWorkDoneCallback)(WGPUQueueWorkDoneStatus, void* userdata);
  void wgpuQueueOnSubmittedWorkDone(WGPUQueue, WGPUQueueWorkDoneCallback, void* userdata);
  ```
- **After** (current Dawn 2026-04):
  ```c
  typedef void (*WGPUQueueWorkDoneCallback)(
      WGPUQueueWorkDoneStatus status,
      WGPUStringView message,                      // <-- new
      void* userdata1, void* userdata2);           // <-- now two userdata
  typedef struct WGPUQueueWorkDoneCallbackInfo {
      WGPUChainedStruct* nextInChain;
      WGPUCallbackMode mode;                        // <-- new: WaitAnyOnly /
      WGPUQueueWorkDoneCallback callback;           //         AllowProcessEvents /
      void* userdata1;                              //         AllowSpontaneous
      void* userdata2;
  } WGPUQueueWorkDoneCallbackInfo;
  WGPUFuture wgpuQueueOnSubmittedWorkDone(WGPUQueue, WGPUQueueWorkDoneCallbackInfo);
  ```

Same shape change for `WGPUCompilationInfoCallback`, `WGPURequestAdapterCallback`, `WGPURequestDeviceCallback`, `WGPUBufferMapCallback`. Six call sites in gpu.cpp@0.2.0's `gpu.hpp` need patching (`wgpuInstanceRequestAdapter` ×1, `wgpuAdapterRequestDevice` ×2 — there's a duplicate site, `wgpuDeviceSetUncapturedErrorCallback` ×2, `wgpuShaderModuleGetCompilationInfo` ×1, `wgpuQueueOnSubmittedWorkDone` ×1) plus the buffer-map flow.

This is **non-trivial** patching — not a verbatim rename but an API-shape rewrite. And once we maintain a patched fork, ADR-0014's claim that gpu.cpp gives us "a thin, readable, ~1000-LoC wrapper" weakens: we'd be maintaining ~1000 LoC + ~100 LoC of patches, against the alternative of writing our own thin wrapper that we know fits.

## 3. Why raw Dawn works fine for our use case

The total amount of Dawn code our project needs to drive a compute kernel is ~280 lines (the `smoke_dawn_add.cpp` body, which already includes verbose ceremony for proper RAII-style cleanup with no helpers). For the project's `webgpu::Backend` adapter, the corresponding logic in `include/tensor/core/backend/webgpu.hpp` would be perhaps ~200 lines plus a `WebGPUContext` helper class managing instance/adapter/device/queue lifetime — comparable to gpu.cpp's surface but **shaped to our specific needs** and **synchronised with Dawn's release cadence by construction** (we never lag because we link directly to the Dawn-version-paired `webgpu.h`).

Dawn also ships an official C++ wrapper `webgpu_cpp.h` (we have `/home/yu/vcpkg/installed/x64-linux/include/webgpu/webgpu_cpp.h` from the same vcpkg install). It is Google-maintained, RAII-flavored, and **always** synchronised with Dawn. For our `webgpu::Backend` adapter implementation, this is the natural choice — we get the ergonomics without the bus factor.

## 4. Implications for ADR-0014

ADR-0014 §Decision Outcome point 2 says:

> **Vendor gpu.cpp under `third_party/gpu_cpp/`.** Copy `gpu.hpp` from `AnswerDotAI/gpu.cpp` at a pinned commit; record the commit SHA and date in `third_party/gpu_cpp/VENDORED_FROM`.

The vendoring discipline was sound (canonical-reference-quality forcing function: don't depend on bus-factor-1 substrates). But the **specific dependency** turned out to be undriveable against the **specific Dawn version** ADR-0014 also chose.

This is a known failure mode the canonical-reference disciplines (per ADR-0015) are designed to surface, not prevent. The fact that the failure was caught the first time we tried to actually use gpu.cpp against current Dawn validates the disciplines: we discovered the problem before it shipped to users.

[ADR-0016](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) refines ADR-0014's substrate decision point 2 in light of this finding: **drop the gpu.cpp wrapper layer; talk to Dawn directly via the C ABI or `webgpu_cpp.h`**. The vendored `third_party/gpu_cpp/` can either be removed (post-ADR-0016 approval) or kept as a historical artifact pointing to ADR-0016 for the actual approach.

## 5. What's now unblocked

With the Stage 2 GPU smoke verified, the following Phase 3 milestones become directly actionable:

- **P3.M3.2 — element-wise dispatch wiring**: replace the stub `webgpu::Backend::{add,sub,mul,div,exp,log,relu,neg}` delegations with real GPU dispatch using the Dawn `webgpu_cpp.h` API. The `smoke_dawn_add.cpp` is the template; each adapter method follows the same allocate-upload-shader-pipeline-dispatch-readback pattern.
- **P3.M4.2 — GEMM dispatch wiring**: same template, with the addition of a uniform buffer for `Params{M,N,K}` and a 2-D dispatch.
- **CI**: an 11th CI job on a self-hosted runner with a Dawn-compatible GPU runs the cross-backend numerical-agreement suite. The current 10-job matrix continues unchanged on GitHub-hosted runners.

## 6. Cross-references

- Standalone smoke: [`build/gpu-smoke/smoke_dawn_add.cpp`](../../build/gpu-smoke/smoke_dawn_add.cpp), [`build/gpu-smoke/smoke_dawn_instance.cpp`](../../build/gpu-smoke/smoke_dawn_instance.cpp), [`build/gpu-smoke/build_dawn_only.sh`](../../build/gpu-smoke/build_dawn_only.sh).
- WGSL source verified: [`include/tensor/core/backend/webgpu_wgsl.hpp::kAddF32`](../../include/tensor/core/backend/webgpu_wgsl.hpp), shipped in PR #43 with text-level validation in [`tests/test_webgpu_wgsl.cpp`](../../tests/test_webgpu_wgsl.cpp).
- ADR-0014 (substrate strategy that this report refines): [`docs/arc42/09-decisions/0014-external-substrate-strategy.md`](../arc42/09-decisions/0014-external-substrate-strategy.md).
- ADR-0016 (the resulting refinement): [`docs/arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md`](../arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md).
- Dawn callback-info struct API: [`/home/yu/vcpkg/installed/x64-linux/include/dawn/webgpu.h`](file:///home/yu/vcpkg/installed/x64-linux/include/dawn/webgpu.h) lines 1285, 1297, 1326, 1420.
