---
status: Draft
owner: tensor
last-reviewed: 2026-05-10
---

# C++ Tensor Library Landscape (2025–2026): A Brief for Reviving `tensor`

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | Layer B — research report (one-shot, dated, append-only)       |
| Owner         | uyuutosa                                                       |
| Related       | `feature/pentaglyph-docs` PR #1 (initial scaffold)             |
| Last Updated  | 2026-05-10                                                     |

## Executive Summary

- The "production dense linear algebra" niche is now thoroughly saturated: [Eigen 5.0](https://libeigen.gitlab.io/releases/5.0/) (Sept 2025), [Armadillo 15.2](https://sourceforge.net/p/arma/news/2025/10/armadillo-c-linear-algebra-lib-1521-maintenance-release/) (Oct 2025), [xtensor 0.27](https://github.com/xtensor-stack/xtensor), and [Kokkos 5.0/5.1](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/) all dropped major C++17/20-modernized releases in 2025. Trying to win on raw performance or coverage is no longer realistic for a one-person revival.
- C++26 will ship [`std::linalg` (P1673)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p1673r12.html) on top of C++23 [`std::mdspan`](https://github.com/kokkos/mdspan). Anything that is "just BLAS over multidim arrays" gets standardized — so a hand-rolled tensor lib must justify its existence above that baseline.
- The killer feature of `tensor` — named axes / Einstein-style broadcasting — is *underserved* in C++ and is in flux even in Python: [PyTorch named tensors](https://docs.pytorch.org/docs/stable/named_tensor.html) remain a "prototype" since 2019, and [JAX `xmap` was deleted](https://github.com/jax-ml/jax/discussions/20312) in favor of `shard_map`. There is a real gap, and small C++ entrants like [Einsums](https://einsums.github.io/Einsums/), [Fastor](https://github.com/romeric/Fastor), and [Tenseur](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/) attack it from different angles.
- The most realistic GPU path for a small project in 2026 is *not* hand-written CUDA but riding on top of an abstraction: [Kokkos](https://github.com/kokkos/kokkos), [SYCL/oneAPI 2025.3](https://www.intel.com/content/www/us/en/developer/articles/release-notes/oneapi-dpcpp/2025.html), or [WebGPU via gpu.cpp](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html). CUDA-direct is still best-in-class for performance but worst-in-class for personal-project cost-of-entry.
- The library's strongest revival narrative is *educational/DSL-research*, not production. The genre exists — [tinygrad](https://github.com/tinygrad/tinygrad), [micrograd](https://github.com/karpathy/micrograd), [TACO](http://tensor-compiler.org/), [COMET](https://github.com/pnnl/COMET) — and is healthy in 2024–2026. Eclipse CDT tooling and pre-C++17 idioms must be replaced regardless of the chosen path.

---

## 1. Competitive / Inspirational Landscape (C++)

| Library | Latest stable | Positioning (1-line) | Emphasis | Gap vs. `tensor` |
|---|---|---|---|---|
| **Eigen** | [5.0.0, 30 Sep 2025](https://libeigen.gitlab.io/releases/5.0/) | de-facto C++ dense linear algebra; expression-template bible | Header-only, MPL2, last C++14 release; master moving to C++17 | No named axes, no first-class N-D tensor (only `Tensor` module, unsupported), no GPU |
| **xtensor** | [0.27.x, C++20](https://github.com/xtensor-stack/xtensor) | "NumPy for C++"; lazy broadcasting on multidim arrays | SIMD via xsimd, NumPy-compatible API | No named axes; broadcasting is positional, not labeled |
| **Blaze** | [3.8 (Aug 2020), still alive](https://bitbucket.org/blaze-lib/blaze) | High-performance HPC math; smart expression templates | Best-in-class dense BLAS perf; CUDA fork via [STEllAR-GROUP](https://github.com/STEllAR-GROUP/blaze) | Mostly matrix/vector, not N-D tensor; no named axes |
| **Armadillo** | [15.2.1, 28 Oct 2025](https://sourceforge.net/p/arma/news/2025/10/armadillo-c-linear-algebra-lib-1521-maintenance-release/) | MATLAB-flavored ergonomics on top of LAPACK | Templated expressions, sparse + dense | Cube is the highest order; no Einstein notation |
| **libtorch** | [PyTorch 2.x, Stable ABI Mar 2025](https://docs.pytorch.org/docs/stable/notes/libtorch_stable_abi.html) | C++ frontend to PyTorch core | Full autograd, CUDA, distributed; huge binary | Not a "library" in the small sense — it's a runtime |
| **ArrayFire** | [3.10 (Oct 2025)](https://arrayfire.com/blog/the-torch-by-arrayfire-summer-2024-gpu-library-updates/) | GPU-first array lib (CUDA/OpenCL) | Now adding Python with Array API conformance | Closed-ish governance; not focused on named-axis semantics |
| **Kokkos** | [5.0/5.1 (2025), C++20](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/) | Performance-portable parallel programming model | Backends: CUDA, HIP, SYCL, OpenMP, threads; built on `mdspan` | Not a tensor algebra lib — a parallel execution + view framework |
| **C++23 `mdspan` / C++26 `std::linalg`** | [P1673R13 in C++26](https://isocpp.org/files/papers/P1673R13.html); [feature freeze Hagenberg Feb 2025](https://herbsutter.com/2025/02/17/trip-report-february-2025-iso-c-standards-meeting-hagenberg-austria/) | Standardized multidim view + BLAS-flavored ops | Standardization, not novelty | Index-based, not named; rank-2 BLAS, not arbitrary-order tensor algebra |
| **Halide** | Active in 2025–2026; recent [tensor-core scheduling work](https://arxiv.org/abs/2512.02371) | Image-processing/tensor DSL with separated algorithm + schedule | Schedule-as-first-class-citizen | DSL, not a value-type tensor lib; learning curve high |
| **TACO** | [Active, GitHub issues into 2025](https://github.com/tensor-compiler/taco) | Sparse/dense tensor algebra compiler | Compile-time codegen for arbitrary index expressions; CUDA backend | Compiler, not a library; sparse-first |
| **Taichi** | [v1.7.4 (Jul 2025)](https://pypi.org/project/taichi/) | Python-embedded DSL for parallel/sparse computation | Spatially sparse data, GPU JIT | Python-front; not a C++ library you link against |

The closest "named-axis-shaped" entrants are small/recent projects:

- [Einsums](https://einsums.github.io/Einsums/) — explicit Einstein-notation C++ tensor library with compile-time pattern analysis.
- [Fastor](https://github.com/romeric/Fastor) — high-perf small tensor algebra with `einsum` template.
- [Tenseur (2024)](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/) — header-only lazy-eval over BLAS/SIMD.
- [TCL — Tensor Contraction Library](https://github.com/springer13/tcl) — `numpy.einsum`-style contractions.

## 2. Named-Axis / Einsum-Style APIs

The original `tensor` library's most distinctive feature is named axes. Status of named-axis ideas across the ecosystem:

- **PyTorch named tensors** are still labelled "prototype, subject to change" in the [Named Tensors operator coverage docs](https://docs.pytorch.org/docs/stable/name_inference.html) as of mid-2025, six years after introduction in [PyTorch 1.3](https://pytorch.org/blog/pytorch-1-dot-3-adds-mobile-privacy-quantization-and-named-tensors/). The community ["State of Torch Named Tensors" issue](https://github.com/pytorch/pytorch/issues/60832) confirms development has stalled. Meta's own follow-up [`torchdim`](https://github.com/facebookresearch/torchdim) is a different, more functional design ("first-class dimensions") and has eclipsed named tensors as the spiritual successor.
- **JAX `xmap`** was deprecated and [deleted from JAX](https://github.com/jax-ml/jax/discussions/20312) in 2024 in favor of `shard_map` + `vmap`. The named-axis programming model in JAX has effectively been retired.
- **Einops + einsum** is the *de facto* winner for named-axis ergonomics in Python ([einops docs](https://einops.rocks/api/einsum/)). It's library-level, not type-system-level.
- **xtensor / Eigen / Blaze / Armadillo** have *no* named-axis support. Broadcasting is positional.
- **Academic/compiler stack**: [TACO](http://tensor-compiler.org/), [COMET](https://github.com/pnnl/COMET) (MLIR-based, 2025 LLM-serving paper), and the [MLIR `linalg` and `tensor` dialects](https://mlir.llvm.org/docs/Dialects/Linalg/) all express index-name semantics formally — but at the compiler IR level, not at user-API level in a header-only C++ lib.
- **Named Tensor Notation** as a research formalism is alive ([namedtensor.github.io](https://namedtensor.github.io/), [Tensor Considered Harmful](https://nlp.seas.harvard.edu/NamedTensor)) — the design space remains open and academically interesting.

This means there is genuine *whitespace*: a small, modern C++ library that takes named axes seriously at compile time (via concepts + NTTPs / string-literal types) is not a duplicate of any of the above.

## 3. GPU Acceleration Paths

Ranked by realistic barrier-to-entry for a personal C++ project in 2026:

1. **WebGPU via [gpu.cpp](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html) / [WebGPU-Cpp](https://github.com/eliemichel/WebGPU-Cpp)** — single-source C++, Dawn or [wgpu-native](https://github.com/gfx-rs/wgpu-native) backend, runs on every desktop GPU and the browser. Lowest install friction; weakest perf ceiling. The 2025 [ONNX Runtime WebGPU EP](https://onnxruntime.ai/docs/tutorials/web/ep-webgpu.html) shows it's production-viable for inference.
2. **Kokkos** — write once, ride [CUDA/HIP/SYCL/OpenMP](https://github.com/kokkos/kokkos) backends. With `mdspan`-based views in [Kokkos 5.0](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/), it's the most natural "I already have multidim views" upgrade path.
3. **SYCL / oneAPI** — [Intel oneAPI 2025.3](https://www.intel.com/content/www/us/en/developer/articles/release-notes/oneapi-dpcpp/2025.html) compiler is mature; portable across NVIDIA/AMD/Intel via Codeplay plug-ins. C++23-friendly, heavyweight compiler.
4. **C++ standard parallelism** (`std::execution`, `par_unseq`) — works today on NVIDIA via nvc++, less mature on AMD/Intel; `std::linalg` GPU backends are nascent. Future-proof, present-limited.
5. **HIP/ROCm** — fine if you have AMD hardware, otherwise an AMD-only ghetto.
6. **CUDA direct** — best perf, worst portability, biggest maintenance debt for a solo project.

For a revival oriented around *educational clarity* and *named-axis semantics*, WebGPU (lowest friction, multi-platform, browser demos!) or Kokkos (cleanest abstraction) are the natural choices. CUDA-direct is a trap.

## 4. Modern C++ Relevance

**`std::mdspan` (C++23) and `std::linalg` (C++26).** These are *complements*, not replacements, for a tensor library. `mdspan` standardizes the view-into-multidim-buffer shape; `std::linalg` standardizes BLAS-1/2/3 free functions on top. They do *not* provide:

- value-type owning tensors with arithmetic operators,
- automatic differentiation,
- broadcasting beyond BLAS shapes,
- named axes / Einstein notation,
- expression-template fusion,
- non-BLAS contractions or higher-order tensor algebra.

So the standard library raises the floor (every tensor lib should now consume `mdspan` interop) but does not eat the ceiling. Eigen 5, Kokkos 5, and recent xtensor releases all explicitly position themselves with respect to `mdspan` rather than against it.

**Modern idioms for tensor expression building (2025–2026):**

- **C++20 concepts** replace SFINAE pyramids for constraining tensor-like types.
- **`constexpr` everything**: shape arithmetic, axis-label checks, Einstein-notation parsing — all at compile time.
- **NTTP string literals** (C++20) make compile-time named axes ergonomic (`"ij,jk->ik"_ein`).
- **CRTP + expression templates** still dominate, but [deducing-this (P0847)](https://en.cppreference.com/cpp/26) in C++23 makes them dramatically less ugly.
- **`mdspan` interop** is becoming the lingua franca for cross-library data exchange.
- **Modules** (used by Kokkos 5.0) help with the build-time tax that expression templates impose, but tooling support is still patchy.

A pre-C++17 codebase with heavy `new`/raw refs, like the existing `tensor`, is essentially incompatible with modern idioms — most of the value of a revival is rewriting the foundations.

## 5. Educational / Niche Positioning

There is a thriving 2024–2026 sub-genre of "small, hackable, didactic" tensor/ML projects:

- [tinygrad](https://github.com/tinygrad/tinygrad) — Python, ~12 primitive ops, runs LLaMA + Stable Diffusion; explicitly didactic-then-production ([2025 review](https://www.blog.brightcoding.dev/2025/09/08/tinygrad-the-ultra-minimal-deep-learning-library-that-runs-llama-and-stable-diffusion/)).
- [micrograd](https://github.com/karpathy/micrograd) — Karpathy's 100-line scalar autograd; canonical teaching artifact.
- [teenygrad](https://github.com/j4orz/teenygrad) — bridges micrograd → tinygrad.
- ["Recreating PyTorch from Scratch"](https://medium.com/data-science/recreating-pytorch-from-scratch-with-gpu-support-and-automatic-differentiation-8f565122a3cc) — popular tutorial article.
- [`dsc`](https://news.ycombinator.com/item?id=44310678) — 2025 Show-HN C++/CUDA tensor lib with stated focus on clarity and observability.
- [Tenseur (2024)](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/) — design write-up for a header-only lazy C++ tensor lib — emblematic of the "I'll show you how" genre.
- [Fastor](https://github.com/romeric/Fastor) — actively maintained, widely cited as a teaching example.

What's missing: a C++ project that *teaches named-axis tensor algebra and convolutions-as-inner-products* clearly. That is precisely the angle the existing `tensor` library was designed around.

## 6. Risks / Dead Ends

- **Reinvention risk vs. `mdspan` + `std::linalg`.** If the revival aims at "fast dense linear algebra in C++", it is competing with a moving standard library + Eigen 5. Don't fight that war.
- **Reinvention risk vs. libtorch.** If the goal is "C++ deep learning", libtorch wins unconditionally.
- **Eclipse CDT.** [Eclipse CDT is still maintained](https://projects.eclipse.org/projects/tools.cdt) but is widely seen as legacy in 2025 ([slant alternatives 2025](https://www.slant.co/options/2472/alternatives/~eclipse-cdt-alternatives)). The `.cproject` file is a portability and reproducibility liability. Replace with CMake (and optionally Bazel/Meson) — every modern C++ tensor lib uses CMake; vcpkg/Conan integration is essentially required.
- **Pre-C++17 idioms.** `new`/raw owning pointers, no concepts, no `mdspan` interop: these block adoption by anyone in 2026. The migration is non-trivial but unavoidable.
- **Single-threaded.** Acceptable for an educational artifact; deadly for any production claim.
- **Named-axis as type-state.** Tempting to encode axis labels in the type system. PyTorch's prototype-forever and JAX's `xmap` retirement suggest that aggressive type-level naming is hard to make ergonomic at scale. Consider hybrid (runtime-checked names, compile-time when possible) rather than pure type-level.

---

## Discussion points for the maintainer

- **D-1: Should the project pivot from production aspirations to explicit educational/DSL-research positioning, given that Eigen 5 + libtorch + `std::linalg` own the production niche?** A clear "this is a teaching library that explores named-axis tensor algebra in modern C++" framing aligns with where the gap actually is.
- **D-2: Is the named-axis API the headline feature, or is it the function-tensor / reference-tensor / convolution-as-inner-product axis?** The named-axis story is the most differentiated and sellable; the others are intellectually interesting but might dilute the pitch. Pick one anchor.
- **D-3: Type-level vs. runtime named axes — which model?** Type-level is elegant but PyTorch/JAX history shows it's painful at scale; runtime with compile-time fast-paths (via NTTP string literals) may be the pragmatic middle.
- **D-4: Should the codebase target C++20/23 from the rewrite, with `mdspan` interop as a first-class requirement?** That is the modern price of admission and a major break from the 2016 codebase.
- **D-5: GPU strategy — pick one of {none, WebGPU via gpu.cpp, Kokkos backend, SYCL}, and commit.** Each implies a very different code structure and audience. "None, single-thread, educational-only" is a perfectly defensible choice that matches the project's history.
- **D-6: Replace Eclipse CDT with CMake (+ vcpkg/Conan)?** This is essentially mandatory for any external contribution in 2026.
- **D-7: How does the library relate to `std::mdspan` and the upcoming C++26 `std::linalg`?** Define interop explicitly: e.g. construct from `mdspan`, expose `std::linalg`-compatible views. Don't compete; ride.
- **D-8: How does it relate to existing einsum-flavored C++ libs ([Einsums](https://einsums.github.io/Einsums/), [Fastor](https://github.com/romeric/Fastor), [Tenseur](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/), [TCL](https://github.com/springer13/tcl))?** Run a head-to-head feature/API table and explicitly state the differentiator (likely: function-tensors and reference-tensors as first-class, convolutions-as-tensor-products as a teaching narrative).
- **D-9: Autograd or no autograd?** Adding it makes the library a tinygrad-class teaching ML framework; not adding it keeps it a pure tensor algebra DSL. Both are defensible niches; pick deliberately.
- **D-10: Output medium — header-only library, library + companion blog/book, or library + Jupyter-notebook tutorials (via xeus-cling or similar)?** Educational positioning rewards bundled pedagogy. The original blog post on convolutions-as-inner-products is a strong seed.

---

## Sources

- [Eigen 5.0 release notes](https://libeigen.gitlab.io/releases/5.0/)
- [Armadillo 15.2.1 release announcement](https://sourceforge.net/p/arma/news/2025/10/armadillo-c-linear-algebra-lib-1521-maintenance-release/)
- [xtensor GitHub](https://github.com/xtensor-stack/xtensor)
- [Blaze on Bitbucket](https://bitbucket.org/blaze-lib/blaze)
- [STEllAR-GROUP/blaze (CUDA fork)](https://github.com/STEllAR-GROUP/blaze)
- [LibTorch Stable ABI docs](https://docs.pytorch.org/docs/stable/notes/libtorch_stable_abi.html)
- [ArrayFire 3.10 summer 2024 update](https://arrayfire.com/blog/the-torch-by-arrayfire-summer-2024-gpu-library-updates/)
- [Kokkos 5.0 announcement (HPSF, 2025)](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/)
- [Kokkos GitHub](https://github.com/kokkos/kokkos)
- [Kokkos `mdspan` reference impl](https://github.com/kokkos/mdspan)
- [P1673R13 — std::linalg](https://isocpp.org/files/papers/P1673R13.html)
- [Herb Sutter's Hagenberg 2025 trip report (C++26 feature freeze)](https://herbsutter.com/2025/02/17/trip-report-february-2025-iso-c-standards-meeting-hagenberg-austria/)
- [C++26 on cppreference](https://en.cppreference.com/cpp/26)
- [Halide language site](https://halide-lang.org/)
- [Pushing Tensor Accelerators Beyond MatMul (Halide, 2025/26)](https://arxiv.org/abs/2512.02371)
- [TACO project site](http://tensor-compiler.org/)
- [TACO GitHub](https://github.com/tensor-compiler/taco)
- [COMET GitHub (PNNL)](https://github.com/pnnl/COMET)
- [Taichi PyPI (v1.7.4, Jul 2025)](https://pypi.org/project/taichi/)
- [PyTorch Named Tensors docs](https://docs.pytorch.org/docs/stable/named_tensor.html)
- [PyTorch Named Tensors operator coverage](https://docs.pytorch.org/docs/stable/name_inference.html)
- ["State of Torch Named Tensors" issue](https://github.com/pytorch/pytorch/issues/60832)
- [PyTorch 1.3 named tensors blog](https://pytorch.org/blog/pytorch-1-dot-3-adds-mobile-privacy-quantization-and-named-tensors/)
- [facebookresearch/torchdim](https://github.com/facebookresearch/torchdim)
- [Named Tensor Notation site](https://namedtensor.github.io/)
- [Harvard NLP — Tensor Considered Harmful](https://nlp.seas.harvard.edu/NamedTensor)
- [JAX xmap → shard_map discussion](https://github.com/jax-ml/jax/discussions/20312)
- [einops einsum docs](https://einops.rocks/api/einsum/)
- [Einsums C++ library](https://einsums.github.io/Einsums/)
- [Fastor on GitHub](https://github.com/romeric/Fastor)
- [Tenseur design post (2024)](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/)
- [TCL — Tensor Contraction Library](https://github.com/springer13/tcl)
- [MLIR linalg dialect](https://mlir.llvm.org/docs/Dialects/Linalg/)
- [MLIR tensor dialect](https://mlir.llvm.org/docs/Dialects/TensorOps/)
- [Intel oneAPI 2025.3 release notes](https://www.intel.com/content/www/us/en/developer/articles/release-notes/oneapi-dpcpp/2025.html)
- [gpu.cpp / Answer.AI](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html)
- [WebGPU-Cpp wrapper](https://github.com/eliemichel/WebGPU-Cpp)
- [wgpu-native](https://github.com/gfx-rs/wgpu-native)
- [ONNX Runtime WebGPU EP](https://onnxruntime.ai/docs/tutorials/web/ep-webgpu.html)
- [tinygrad on GitHub](https://github.com/tinygrad/tinygrad)
- [tinygrad 2025 overview](https://www.blog.brightcoding.dev/2025/09/08/tinygrad-the-ultra-minimal-deep-learning-library-that-runs-llama-and-stable-diffusion/)
- [karpathy/micrograd](https://github.com/karpathy/micrograd)
- [teenygrad (micrograd → tinygrad bridge)](https://github.com/j4orz/teenygrad)
- ["Recreating PyTorch from Scratch" tutorial](https://medium.com/data-science/recreating-pytorch-from-scratch-with-gpu-support-and-automatic-differentiation-8f565122a3cc)
- [Show HN: dsc tensor lib in C++/CUDA (2025)](https://news.ycombinator.com/item?id=44310678)
- [Eclipse CDT project page](https://projects.eclipse.org/projects/tools.cdt)
- [Slant — Eclipse CDT alternatives 2025](https://www.slant.co/options/2472/alternatives/~eclipse-cdt-alternatives)
