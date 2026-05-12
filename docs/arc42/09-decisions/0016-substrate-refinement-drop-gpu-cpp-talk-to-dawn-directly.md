---
status: Accepted
owner: tensor
last-reviewed: 2026-05-12
---

# ADR-0016: Drop the gpu.cpp wrapper layer; talk to Dawn directly via the C ABI (`webgpu.h`) or Dawn's own C++ wrapper (`webgpu_cpp.h`)

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-12                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-12) + the smoke-test discovery report.      |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Refines   | [ADR-0014 §Decision Outcome point 2](0014-external-substrate-strategy.md) — gpu.cpp is no longer the WebGPU surface; Dawn directly is. ADR-0014 remains in force; only point 2 is rewritten. |
| Triggered by | [Stage 2 GPU smoke + ABI drift discovery report (2026-05-12)](../../reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md). |

---

## Context and Problem Statement

[ADR-0014 §Decision Outcome point 2](0014-external-substrate-strategy.md) chose to vendor [AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) under `third_party/gpu_cpp/` as the project's WebGPU surface. The chosen pin was tag `0.2.0` (commit `9a42592a0b`, 2025-02-10). The rationale was: gpu.cpp provides a thin, readable, ~1000-LoC C++ wrapper around the WebGPU C API; bus-factor 1 risk is mitigated by vendoring.

On 2026-05-12, while running Stage 2 of Phase 3 (the first real GPU smoke after vcpkg's Dawn port was installed locally), we discovered that **gpu.cpp@0.2.0 does not compile against the current vcpkg Dawn port `20260410.140140`**. Dawn migrated all of its async-callback surface to a `*CallbackInfo` struct pattern between gpu.cpp@0.2.0's tagging date (Feb 2025) and Dawn 2026-04 — six call sites in gpu.cpp's `gpu.hpp` need API-shape rewrites. See the [discovery report](../../reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md) for the full audit.

Concurrently we wrote a ~280-line standalone smoke that talks to Dawn's C ABI directly (via `webgpu.h`) and **successfully dispatched the project's `kAddF32` WGSL source on the local RTX 3090, producing bit-exact results vs CPU**. This established two facts:

1. Dawn-direct works on RTX 3090 + NVIDIA driver + Vulkan stack.
2. The amount of Dawn-direct code our `webgpu::Backend` would need (~200 lines + a `WebGPUContext` helper) is comparable to gpu.cpp's wrapper layer, but shaped to our needs and **synchronised with Dawn's release by construction**.

The question this ADR answers: keep patching gpu.cpp, or drop it and talk to Dawn directly?

---

## Decision Drivers

- **DD-1**: Substrate sturdiness. The whole point of ADR-0014 was to make substrate choices that don't break on upstream churn. gpu.cpp's bus factor materialised exactly as feared.
- **DD-2**: Maintenance load. A patched fork of gpu.cpp would be ~1000 LoC of upstream + ~100 LoC of our patches — a worse position than ~200 LoC of our own thin wrapper.
- **DD-3**: Synchronisation with Dawn. If we link the same `webgpu_dawn.a` we also rely on its `webgpu.h` shape; using gpu.cpp puts a lagging-by-14-months intermediate in between.
- **DD-4**: Canonical-reference discipline ([ADR-0015](0015-aspire-to-canonical-reference-quality-not-self-anoint.md)). Removing a non-load-bearing layer makes the implementation more legible, not less — exactly the kind of depth-over-breadth move CONTRIBUTING.md calls out.
- **DD-5**: Reversibility. If gpu.cpp ever ships a 0.3+ that tracks Dawn 2026+, we can re-evaluate. The decision today is operational, not strategic.

---

## Considered Options

1. **Keep gpu.cpp; patch the six API-drift sites locally** — maintain a fork inside `third_party/gpu_cpp/`. Document patches in `VENDORED_FROM`.
2. **Drop gpu.cpp; use Dawn's C ABI (`webgpu.h`) directly** — write a small `tensor::core::backend::webgpu::detail::WebGPUContext` helper that manages instance / adapter / device / queue lifetime; per-op methods use raw `webgpu.h` calls.
3. **Drop gpu.cpp; use Dawn's official C++ wrapper (`webgpu_cpp.h`)** (chosen) — Dawn ships its own RAII C++ wrapper as part of the same vcpkg install. It is Google-maintained, always synchronised with Dawn, and gives the ergonomics gpu.cpp tried to give.
4. **Switch GPU implementation off Dawn entirely** (e.g. to `wgpu-native` or raw Vulkan) — out of scope; Dawn is working for us and ADR-0012 / ADR-0014 chose it deliberately.

---

## Decision Outcome

**Chosen option: 3 — drop gpu.cpp; use Dawn's `webgpu_cpp.h` as the C++ surface for the `webgpu::Backend` adapter.**

Concretely:

1. **The vendored `third_party/gpu_cpp/` is no longer load-bearing.** It remains in the tree for the moment with a note in its README that ADR-0016 supersedes its role; a follow-up PR removes the directory once the dispatch-wiring PRs (P3.M3.2 / P3.M4.2) have committed against `webgpu_cpp.h` and the `VENDORED_FROM` record is no longer informative.
2. **`webgpu::Backend` uses `dawn::webgpu_dawn` (the existing vcpkg CMake target) + `<webgpu/webgpu_cpp.h>`.** The same Dawn library binary is linked; only the C++ surface above it changes.
3. **A small helper** under `include/tensor/core/backend/webgpu_detail/` manages instance / adapter / device / queue lifetime as singletons per `Backend` instance. This is the closest equivalent to gpu.cpp's `Context` for our needs.
4. **The shipped WGSL sources** (`include/tensor/core/backend/webgpu_wgsl.hpp` from PR #43 / #44 / #46) are **unchanged**. They are pure WGSL strings with the project's own `{{precision}}` / `{{workgroupSize}}` placeholders; the substitution that gpu.cpp used to provide is moved into our own helper (~10 lines, see `build/gpu-smoke/smoke_dawn_add.cpp` for the reference implementation).
5. **ADR-0014 §Decision Outcome point 2 ("vendor gpu.cpp")** is overridden by this ADR but ADR-0014 stays in force; only point 2 is replaced.
6. **CONTRIBUTING.md** and `third_party/gpu_cpp/README.md` get pointers to ADR-0016 so future contributors don't re-add gpu.cpp by accident.

### Y-statement summary

> In the context of **discovering on 2026-05-12 that gpu.cpp@0.2.0 (vendored per ADR-0014) does not compile against the current Dawn vcpkg port due to a 14-month-old async-callback API migration**, facing **the choice between maintaining a patched fork of gpu.cpp or talking to Dawn directly**, we decided for **dropping gpu.cpp and using Dawn's own `webgpu_cpp.h` (Google-maintained, always synchronised with the linked Dawn binary)**, to achieve **a substrate that does not lag the linked library and that our `webgpu::Backend` implementation can fit to its own needs without an intermediate translation layer**, accepting **the loss of gpu.cpp's `Bindings<N>` / `KernelCode` ergonomics, which we replace with ~10-line helpers (the smoke-test code in `build/gpu-smoke/smoke_dawn_add.cpp` is the worked reference)**.

---

## Pros and Cons of the Options

### Option 1: Keep gpu.cpp; patch the six API-drift sites locally

- Pros: respects ADR-0014's wording verbatim.
- Cons: turns the project into the upstream-of-itself for a wrapper layer we don't actually need; future Dawn releases will require more patches; the wrapper layer is not load-bearing for our use case.

### Option 2: Drop gpu.cpp; use `webgpu.h` (C ABI) directly

- Pros: zero new dependency surface; total control.
- Cons: verbose ceremony (RAII-less; explicit `wgpu*Release` calls everywhere); the smoke code is exactly this and at ~280 lines for a single kernel it's noticeably noisier than `webgpu_cpp.h` equivalent.

### Option 3: Drop gpu.cpp; use `webgpu_cpp.h` (Dawn's C++ wrapper) (chosen)

- Pros: RAII ergonomics; same vcpkg install; Google-maintained; tracks Dawn exactly; no bus-factor exposure; our wrapper code shrinks to ~200 lines for the `webgpu::Backend` implementation; we own the small slice that knows the project's `KernelBackend` port.
- Cons: ties us closer to Dawn-specific C++ idioms (acceptable — we already chose Dawn in ADR-0012; locking in deeper is consistent).

### Option 4: Off Dawn entirely

- Pros: removes the X11/depot_tools-style Chromium dependency from any future contributor.
- Cons: ADR-0012's WebGPU choice and ADR-0014 are working as designed; switching backends is a strategic move with much higher cost.

---

## Consequences

### Positive

- The `webgpu::Backend` adapter ships against a substrate that does not have a 14-month-lag layer in it.
- The vendoring discipline (ADR-0014) is *vindicated*, not abandoned: the discipline made us re-vendor gpu.cpp instead of consuming it from upstream, and the re-vendoring discipline made the ABI drift discoverable as a build error in a local smoke instead of a runtime issue in a user's hand.
- `webgpu_cpp.h` is the same surface every other Dawn consumer uses; community know-how transfers.

### Negative

- ADR-0014 §Decision Outcome point 2's specific wording is overridden — the gpu.cpp paragraph in the README's `third_party/gpu_cpp/README.md` will be reworded, and the directory is targeted for removal.
- ~200 lines of `webgpu_cpp.h` integration code in `include/tensor/core/backend/webgpu.hpp` instead of "delegates to gpu.cpp's `createKernel` / `dispatchKernel`".

### Neutral

- The shipped WGSL kernel sources (PR #43 / #44 / #46) are unchanged.
- The `KernelBackend` port (ADR-0011) is unchanged.
- ADR-0014's other substrate decisions (Dawn via vcpkg, xeus-cpp, `tensor::linalg` shim over kokkos/stdBLAS) remain in force.

### Follow-ups

- [ ] Stage 3 PR — replace `webgpu::Backend::{add,sub,mul,div,exp,log,relu,neg}` delegations with `webgpu_cpp.h` dispatch using the design from `build/gpu-smoke/smoke_dawn_add.cpp` as the template (P3.M3.2).
- [ ] Stage 3 follow-up — same template for `contract` (P3.M4.2) once GEMM dispatch is checked against `kGemmF32` on a real GPU.
- [ ] Update `third_party/gpu_cpp/README.md` with the ADR-0016 supersession pointer.
- [ ] Update `CONTRIBUTING.md` § Vendored third-party code with a note: gpu.cpp specifically is no longer load-bearing; future re-evaluations welcome.
- [ ] Delete `third_party/gpu_cpp/` after Stage 3 lands (separate PR).

---

## Compliance / Validation

- **Verification**: after Stage 3, no header under `include/tensor/core/backend/` includes from `third_party/gpu_cpp/`. CI's existing `vendored-check` job continues to enforce the `VENDORED_FROM` discipline for any *other* substrate that gets vendored.
- **Frequency**: per-PR.

---

## More Information

- Discovery report: [`docs/reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md`](../../reports/2026-05-12_gpu-cpp-dawn-abi-drift-and-raw-dawn-smoke.md).
- Working raw-Dawn smoke (the reference template for Stage 3): [`build/gpu-smoke/smoke_dawn_add.cpp`](../../../build/gpu-smoke/smoke_dawn_add.cpp).
- Dawn's `webgpu_cpp.h` (installed at `~/vcpkg/installed/x64-linux/include/webgpu/webgpu_cpp.h` per the vcpkg port).
- Refined: [ADR-0014 §Decision Outcome point 2](0014-external-substrate-strategy.md).
- Architectural foundation kept in force: [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0011](0011-kernel-backend-port-api.md), [ADR-0012](0012-webgpu-adapter-implementation-design.md).
