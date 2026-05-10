---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0006: Adopt WebGPU (gpu.cpp / Dawn or wgpu-native) as the GPU backend

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief §3)                                     |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The 2016 codebase is single-threaded with no GPU support. The research brief (§3) ranks GPU paths for a personal C++ project in 2026 by realistic barrier-to-entry: WebGPU > Kokkos > SYCL > C++ stdpar > HIP > CUDA-direct.

Per ADR-0001 (educational pivot) and ADR-0008 (Jupyter notebook tutorials), the GPU path must:

- Run on a learner's laptop without proprietary toolchain installs.
- Demo cleanly in a notebook (and ideally in a browser tab).
- Not blow up the maintenance budget.

CUDA-direct is therefore disqualified despite its raw-perf lead. The realistic candidates are WebGPU, Kokkos, and SYCL.

---

## Decision Drivers

- **DD-1**: Cost of entry for the learner. The learner should not need to install CUDA, ROCm, or Intel oneAPI.
- **DD-2**: Browser reach. Notebooks rendered through e.g. Pyodide / online sandboxes can demo GPU compute if the backend is WebGPU.
- **DD-3**: Multi-vendor coverage. NVIDIA + AMD + Intel + Apple Silicon + integrated GPUs.
- **DD-4**: Maintenance. Solo project; we ride someone else's portability layer.

---

## Considered Options

1. **None** — single-thread CPU only.
2. **WebGPU via [gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) / [Dawn](https://dawn.googlesource.com/dawn) / [wgpu-native](https://github.com/gfx-rs/wgpu-native)**.
3. **[Kokkos](https://github.com/kokkos/kokkos)**.
4. **SYCL via [Intel oneAPI](https://www.intel.com/content/www/us/en/developer/articles/release-notes/oneapi-dpcpp/2025.html) or [AdaptiveCpp](https://github.com/AdaptiveCpp/AdaptiveCpp)**.
5. **CUDA-direct** (rejected in research; included for record).

---

## Decision Outcome

**Chosen option: 2 — WebGPU via gpu.cpp + Dawn (with wgpu-native as a fallback impl).**

WebGPU uniquely satisfies DD-1 (zero proprietary install — the runtime is the GPU driver the laptop already has), DD-2 (browser-native), and DD-3 (works on every desktop GPU and Apple Silicon out of the box). [`gpu.cpp`](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html) (Answer.AI, 2024) provides an ergonomic single-source C++ wrapper that fits an educational artifact better than raw Dawn.

Kokkos and SYCL are technically stronger for HPC workloads but each one of them requires a substantial runtime install that defeats the educational pitch. Kokkos remains attractive *as a future complementary backend* if a contributor wants HPC reach; this ADR does not foreclose adding it later.

### Y-statement summary

> In the context of **adding a GPU compute path to the rewritten `tensor` library**, facing **the trade-off between perf ceiling and learner-side install friction**, we decided for **WebGPU via gpu.cpp on Dawn / wgpu-native**, to achieve **zero-install demos that run on the learner's laptop and in the browser**, accepting **a lower perf ceiling than CUDA / Kokkos / SYCL**.

---

## Pros and Cons of the Options

### Option 1: None

- Pros: simplest; matches the project's history.
- Cons: gives up the entire "watch tensor algebra accelerate on your GPU" pedagogical moment.

### Option 2: WebGPU via gpu.cpp / Dawn / wgpu-native (chosen)

- Pros: zero proprietary install; runs in browser; multi-vendor; gpu.cpp is single-source ergonomic; production credibility ([ONNX Runtime WebGPU EP](https://onnxruntime.ai/docs/tutorials/web/ep-webgpu.html), [Stable Diffusion in browser via wgpu](https://github.com/gfx-rs/wgpu)).
- Cons: lower perf ceiling than CUDA; WGSL shader language is one more thing to learn; Dawn embedding adds significant build-time cost.

### Option 3: Kokkos

- Pros: cleanest abstraction over CUDA/HIP/SYCL/OpenMP; shares `mdspan` with our core; HPC-grade.
- Cons: Kokkos itself + a vendor toolchain (CUDA / HIP / SYCL) is a substantial install for a learner.

### Option 4: SYCL

- Pros: portable across vendors via Codeplay plug-ins; mature compiler.
- Cons: oneAPI install is large; Codeplay plug-in for non-Intel hardware is an extra step.

### Option 5: CUDA-direct

- Pros: best perf ceiling.
- Cons: NVIDIA-only; CUDA toolkit install; antithesis of the educational pitch.

---

## Consequences

### Positive

- Notebooks ship with `tensor::gpu::matmul(...)` demos that work on any laptop.
- A future "tensor in the browser" demo (e.g. an interactive web playground) becomes feasible.
- The library can demonstrate the full pipeline: named-axis expression → WGSL kernel emission → execution.

### Negative

- Embedding Dawn or wgpu-native is non-trivial; `vcpkg` ports exist but build times are long.
- Performance is meaningfully below CUDA-direct; the README must say so.
- Two GPU runtimes (Dawn for desktop / wgpu-native for portability) double the test matrix.

### Neutral

- Kokkos remains reachable as a *secondary* backend if a contributor steps up.
- The autograd subsystem (ADR-0007) is GPU-agnostic; it composes with WebGPU kernels as it would any other.

### Follow-ups

- [ ] Spike: gpu.cpp build integration via vcpkg (Dawn port).
- [ ] Detailed design: WGSL kernel emission from named-axis expressions (`docs/detailed-design/02_webgpu-codegen.md`).
- [ ] CI: add a GPU job (or document that GPU tests are local-only initially).
- [ ] Notebook tutorial: "your first GPU tensor" using gpu.cpp.

---

## Compliance / Validation

- Verification: notebook smoke test that runs a named-axis matmul on the GPU backend and matches the CPU result.
- Frequency: every release.

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)
- Distribution: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)

### References

- [gpu.cpp / Answer.AI launch post](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html)
- [Dawn (WebGPU implementation)](https://dawn.googlesource.com/dawn)
- [wgpu-native](https://github.com/gfx-rs/wgpu-native)
- [WebGPU-Cpp wrapper](https://github.com/eliemichel/WebGPU-Cpp)
- [ONNX Runtime WebGPU EP](https://onnxruntime.ai/docs/tutorials/web/ep-webgpu.html)
- [WebGPU Specification](https://www.w3.org/TR/webgpu/)
- Research brief: `docs/reports/2026-05-10_tensor-revival-landscape.md` §3
