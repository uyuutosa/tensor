---
status: Stable
owner: tensor
last-reviewed: 2026-05-11
---

# External-substrate research: gpu.cpp, vcpkg, xeus-cling, `std::linalg` (May 2026)

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable; frozen snapshot of external-substrate signals as of 2026-05-11. |
| Type         | Layer B — research / due-diligence brief (dated, append-only)   |
| Owner        | uyuutosa                                                       |
| Triggered by | maintainer's "world-governing framework" north-star prompt (2026-05-11) + open Axis E in [`2026-05-11_open-discussion-points.md`](./2026-05-11_open-discussion-points.md). |
| Triggers     | [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) (canonical-reference framing), [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md) (vendor gpu.cpp + Dawn-via-vcpkg + xeus-cpp + stdBLAS shim). |

## Purpose

Before committing more code to Phase 3 (WebGPU kernels) or to Phase 4 (`0.1.0` release), the four external substrates the project leans on need to be audited:

1. **gpu.cpp** — chosen as the WebGPU surface in [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md); flagged as bus-factor 1.
2. **vcpkg ports** for `dawn` / `wgpu-native` / `gpu.cpp` — the integration path P3.M3 needs.
3. **xeus-cling** — the Jupyter kernel ADR-0008 picked; its C++20 coverage caps the notebook executable depth.
4. **C++26 `std::linalg` (P1673)** — the standard-library competitor the project will eventually live alongside.

The dated snapshot below is what determined ADR-0013 and ADR-0014.

---

## Executive summary

- **gpu.cpp is in soft-maintenance, not abandonment.** Last commit Feb 2026; no v0.2 release since v0.1.0 (Aug 2024); ~4k stars; two named contributors. Bus-factor 1 confirmed. ([AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp))
- **Dawn is now first-class in vcpkg** (port version `20260410.140140`, refreshed 2026-04-20). `wgpu-native` and `gpu.cpp` have **no** vcpkg ports as of the 2026.04.27 release. ([vcpkg dawn package](https://vcpkg.io/en/package/dawn.html), [vcpkg Release 2026.04.27](https://github.com/microsoft/vcpkg/releases/tag/2026.04.27))
- **xeus-cling is effectively frozen** at v0.15.3 (Jul 2023) on Cling/LLVM-13, locked to C++17; its successor **xeus-cpp** (Clang-Repl + CppInterOp) shipped v0.10.0 on 2026-04-02 and is the modern path. ([xeus-cling tags](https://github.com/jupyter-xeus/xeus-cling/tags), [xeus-cpp](https://github.com/compiler-research/xeus-cpp), [Cling transitions to LLVM's Clang-Repl](https://root.cern/blog/cling-in-llvm/))
- **`std::linalg` is C++26 (feature-frozen at Hagenberg 2025) but ships in no major standard library yet** — libc++ "not started"; libstdc++ no entry; MSVC STL changelog through 14.52-Preview silent. The [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS) reference implementation is the only working option.
- **Strategic implication**: vendor (not link) gpu.cpp; depend on Dawn via vcpkg; migrate notebook CI to xeus-cpp; shim `std::linalg` via stdBLAS. These are captured operationally in [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md).

---

## Item 1 — gpu.cpp project activity status

### Findings (2026-05-11)

- **Last commit** ([AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp)): 2026-02-10 — `Merge pull request #74 from AnswerDotAI/dev`, authored by `austinvhuang` (project originator). The two prior commits (2026-02-07 and 2026-02-08) were binding fixes from `junjihashimoto`. Activity cadence is roughly one merge-cluster every several months.
- **Last release**: **v0.1.0** dated **2024-08-13**. No v0.1.1, no v0.2.0 — i.e. no release in 18+ months despite commits.
- **Stars / forks**: ~4,000 stars, 193 forks, 8 open issues, 1 open PR. Star count grew rapidly after the [July 2024 launch blog post](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html) but plateaued.
- **Contributors**: the visible commit log is dominated by two names: `austinvhuang` and `junjihashimoto`. This corroborates the Phase-0 bus-factor-1 classification.
- **Answer.AI engagement**: still *using* gpu.cpp — the [GPU Programming from Scratch](https://www.answer.ai/posts/2025-03-17-gpu-programming-scratch.html) educational post (March 2025) and [gpupuzzles.answer.ai](https://gpupuzzles.answer.ai/intro) both build on it. Answer.AI is not staffing it for production hardening, but it has a captive educational use-case keeping it alive.
- **Production deployments**: no evidence of named third-party production deployments. The README and blog post pitch portable low-level GPU compute with ~5s rebuild cycles, not a battle-tested ML kernel runtime.

### What this means for the tensor project

gpu.cpp is what an *educational* library wants — a thin, readable, ~1000-LoC header that wraps Dawn — and almost exactly what a *governance-grade* library does **not** want as a hard dependency. The mitigation hierarchy:

1. **Vendor a pinned copy** of `gpu.hpp` under `third_party/gpu_cpp/` rather than taking a submodule or vcpkg port (none exists — Item 2). Treat it as in-tree code we own.
2. **Keep the abstraction seam** so the WebGPU backend can swap to direct Dawn or `wgpu-native` if Answer.AI stops merging PRs entirely.
3. **Submit upstream patches** for any bugs found rather than forking silently — the cheapest way to extend the bus factor.

These decisions are captured in [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md).

---

## Item 2 — vcpkg port availability

### Findings (2026-05-11)

- **Dawn**: **available and current**. The [vcpkg dawn package](https://vcpkg.io/en/package/dawn.html) lists version `20260410.140140` (date-coded snapshot of `dawn.googlesource.com/dawn`), last refreshed 2026-04-20, BSD-3-Clause. Port-request issue [#41847](https://github.com/microsoft/vcpkg/issues/41847) (Oct 2024) was resolved by addition; the older [#22948](https://github.com/microsoft/vcpkg/issues/22948) (2022) had been closed for inactivity. The vcpkg curated registry reached 2,807 ports at [Release 2026.04.27](https://github.com/microsoft/vcpkg/releases/tag/2026.04.27), +35 net additions. Earlier momentum visible in [What's New in vcpkg (Nov 2025 – Jan 2026)](https://devblogs.microsoft.com/cppblog/whats-new-in-vcpkg-nov-2025-jan-2026/) and [Apr 2026](https://devblogs.microsoft.com/cppblog/whats-new-in-vcpkg-apr-2026/). Stability is not formally flagged; the date-snapshot scheme means consumers should pin a baseline rather than track head.
- **wgpu-native** (gfx-rs/wgpu-native, Rust + C ABI): **no official port**. Search of vcpkg release notes through 2026.04.27 finds no mention; no open `[New Port Request]` either. vcpkg's CMake-centric model fights with `cargo`-built artefacts — a port would need to bundle prebuilts per triplet or invoke `cargo` from CMake, neither of which has clean precedent. The community starter [Twinklebear/wgpu-cpp-starter](https://github.com/Twinklebear/wgpu-cpp-starter) sidesteps vcpkg entirely with FetchContent.
- **gpu.cpp**: **no port, official or community**. The vcpkg ports tree and 2026 release notes return nothing. Given gpu.cpp is header-only over Dawn, a port would be a thin `dawn` re-export plus a `gpu.hpp` header install — trivial (~30 lines of `portfile.cmake`) but nobody has filed one.

### What this means for the tensor project

- **Default WebGPU build path becomes Dawn-via-vcpkg.** This is the largest unblock the research surfaced: the Phase 3 P3.M3 (WGSL element-wise kernels) milestone was previously waiting on a vcpkg-port story for the GPU runtime. As of April 2026 it has one. Pin to a specific `20260410.x` baseline in `vcpkg.json`.
- **Treat wgpu-native as a manual-build flag,** not a vcpkg dependency. Useful for OS-coverage parity (Linux mesa stacks where Dawn is fussy) but requires opting into `cargo`.
- **Contribute a `gpu.cpp` vcpkg port upstream** as a community contribution. High-leverage, low-cost: simultaneously advertises this project and extends gpu.cpp's bus factor by giving Answer.AI a downstream integrator.

---

## Item 3 — xeus-cling C++20 coverage in 2026

### Findings (2026-05-11)

- **Latest stable tag**: **0.15.3, released 2023-07-24** — nearly three years stale ([xeus-cling tags](https://github.com/jupyter-xeus/xeus-cling/tags)). The [Releases page](https://github.com/jupyter-xeus/xeus-cling/releases) is empty (tags only). The project is *not dead* — 3.3k stars, 173 open issues — but in maintenance mode awaiting Cling/Clang-Repl migration.
- **C++ standard ceiling**: the README advertises the **C++17 kernel**. The long-standing [issue #326 "Can I specify C++20?"](https://github.com/jupyter-xeus/xeus-cling/issues/326) (open since April 2020) remains the canonical wishlist. Cling is pinned to an LLVM 13 vintage of Clang, so the C++20 feature mix is what Clang-13 had: concepts and ranges (header `<ranges>`) compile, but the REPL has well-known issues with constraint diagnostics, and full ranges adaptor pipelines often hit Cling's incremental-AST limitations.
- **Specific feature support** (inferred from Clang-13 / Cling 1.0):
  - **Concepts**: partial — definable, but template constraint errors are noisy under the REPL.
  - **NTTP class types**: Clang-13 partial; expect failures on class-type NTTPs (i.e. our `FixedString` and `LabelTag` pattern).
  - **`consteval`**: supported by Clang-13, generally works.
  - **`<ranges>`**: header parseable; complex pipelines unreliable.
  - **Modules**: **not viable** under Cling — incremental compilation and BMI consumption unresolved in any released xeus-cling.
  - **Deducing-`this` (C++23)**: **not supported** — Clang-13 predates the feature.
- **`<mdspan>`**: not in libstdc++/libc++ at the LLVM-13 vintage, so users must `#include` a polyfill such as `kokkos/mdspan`. Loads fine in xeus-cling because it's header-only and C++17-compatible — this matches what PR #30 already restored.
- **Blockers to C++23**: all upstream — Cling's frozen LLVM base, the unfinished modules story, and `import std` requiring Clang-19+. The [Cling Transitions to LLVM's Clang-Repl](https://root.cern/blog/cling-in-llvm/) announcement explicitly redirects future work to **xeus-cpp** ([compiler-research/xeus-cpp](https://github.com/compiler-research/xeus-cpp)), which released **v0.10.0 on 2026-04-02** and is built on Clang-Repl + CppInterOp. xeus-cpp inherits whatever C++23/26 surface the underlying Clang shipping supports (Clang 18–20 in late 2025/early 2026), so concepts, ranges, `<mdspan>`, and deducing-`this` are far more tractable there.

### What this means for the tensor project

The Jupyter Book scaffold should target **xeus-cpp**, not xeus-cling, for any notebook that demonstrates anything beyond C++17:

- The named-axis algebra uses concepts heavily; that mostly works in xeus-cling but produces ugly REPL errors. xeus-cpp produces modern Clang diagnostics.
- The `TypedTensor<T, "i", "j", ...>` path uses class-type NTTPs (`FixedString`) — Clang-13 support is iffy. xeus-cpp on Clang-18+ is fine.
- The `_tex` parser is `consteval`-heavy; works in both, but C++23 `if consteval` only in xeus-cpp.
- Document a `mdspan` polyfill path. Even in xeus-cpp on Clang-20, `<mdspan>` is feature-test-gated; vendor the Kokkos reference header so notebooks compile uniformly. (PR #30 already did this for the build side; the notebook story needs the same.)
- Keep an xeus-cling fallback chapter for users on conda-forge stable channels — but mark it as "frozen, C++17 only."

These actions are captured in [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md).

---

## Item 4 — `std::linalg` (P1673) shipping status

### Findings (2026-05-11)

- **Standardisation**: P1673R13 was voted into the C++ working draft at **Kona 2023** ([P1673R13](https://isocpp.org/files/papers/P1673R13.html), [cplusplus/papers tracking issue #557](https://github.com/cplusplus/papers/issues/557)). Hagenberg (Feb 2025) finalised the **C++26 feature freeze** ([ISO C++ Committee Meeting Update: Hagenberg](https://community.latenode.com/t/iso-c-committee-meeting-update-c-26-feature-freeze-finalized-in-hagenberg/11926)) and accepted the corrective papers [P3050 (conjugated optimisation)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3050r2.html) and [P3222 (transposed special cases)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3222r1.html). So `<linalg>` is in C++26 and the spec is stable.
- **libc++ status**: the [libc++ C++2c status page](https://libcxx.llvm.org/Status/Cxx2c.html) lists P1673R13 with **no completion status** — work has not started as of the May 2026 snapshot. Consistent with libc++'s general policy of prioritising language-feature library hooks over numerical headers.
- **libstdc++ status**: the [GCC C++ Standards Support page](https://gcc.gnu.org/projects/cxx-status.html) directs to libstdc++'s implementation status manual, which does not list `<linalg>` as in-progress. No GCC release notes through GCC 15 mention shipping `<linalg>`.
- **MSVC STL status**: the [microsoft/STL Changelog](https://github.com/microsoft/STL/wiki/Changelog) up through MSVC Build Tools **14.52-Preview** (Q2 2026) lists many C++23 and C++26 additions — `<flat_map>`, `<flat_set>`, explicit lifetime — but **no `<linalg>`**. Microsoft has historically led on numerical headers (`<execution>`, `<mdspan>`), so its silence is meaningful.
- **Reference implementation**: [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS) remains the only working implementation. ~705 commits on `main`, 9 open PRs, 38 open issues — clearly maintained, authored by paper author Mark Hoemmen and Sandia colleagues. Requires C++17, builds with GCC/Clang/MSVC-2019+, and provides optional BLAS-backed kernels.

### What this means for the tensor project

- **Do not depend on `std::linalg` in any user-facing API surface before 2028 at the earliest.** All three vendor libraries are 24+ months behind the standard text. Practical availability for typical educational users (distro GCC / Apple Clang / MSVC stable) is realistically 2029.
- **Use kokkos/stdBLAS as a shim today.** Wrap behind a `tensor::linalg::*` namespace so when libstdc++/libc++/MSVC STL ship `<linalg>`, the standard names can be re-exported without a public API break. Same pattern as `std::experimental::mdspan` → `std::mdspan`.
- **Make the BLAS optionality explicit.** stdBLAS has both a portable C++ implementation and BLAS-backed paths; a canonical-reference library should expose both via a CMake option, defaulting to portable for reproducibility and offering BLAS for performance.
- **Track the C++26 corrective papers.** P3050 and P3222 will be reflected in stdBLAS — pin to a stdBLAS commit *after* those merges, not an older tag.

---

## Implications for the "world-governing framework" pivot

Across the four items, three structural themes emerge.

**1. Every external substrate the project depends on is in a transitional state.** WebGPU's C++ surface is in flux (Dawn stable, gpu.cpp thin, wgpu-native unpackaged); the C++ Jupyter kernel is mid-migration (xeus-cling → xeus-cpp); the standard linear-algebra story is specified-but-not-shipped. A library that aims to *govern* a domain cannot afford to inherit any one of these transitions as a hard requirement. The architecture must therefore:

- **Vendor, not link**, anything bus-factor 1 (gpu.cpp).
- **Adapt, not consume**, anything mid-migration (xeus-cling/xeus-cpp).
- **Shim, not assume**, anything specified-but-unshipped (`std::linalg`).

**2. The Hexagonal architecture is exactly the right insurance.** The reference / Eigen / WebGPU-stub split already in place is well-matched to the reality discovered above. Concretely: the Eigen backend gives a today-deployable BLAS path; the reference backend gives a no-deps fallback for environments where Dawn cannot build; the WebGPU backend (once P3.M3+ lands kernels) targets Dawn-via-vcpkg with gpu.cpp vendored on top. A *fourth* port — `std::linalg` — should be sketched but stubbed against kokkos/stdBLAS until `<linalg>` actually ships in two of the three vendor libraries. The hexagonal seams let the project make all four bets simultaneously without committing any user to any of them.

**3. The educational-artefact identity is a strategic asset, not a liability.** Looking at the four items together: gpu.cpp's role at Answer.AI is *educational* ([GPU Puzzles](https://gpupuzzles.answer.ai/intro)); xeus-cpp's funding ([OAC-1931408](https://compiler-research.org/projects/)) is *educational*; `std::linalg`'s reference implementation lives at *Sandia/Kokkos*, used heavily in *educational* HPC contexts. The "world-governing" framework that will actually win this niche is the one that *teaches* the named-axis + tape-based-autograd + LaTeX-DSL story end-to-end, with deployable backends as a bonus — not one that competes with PyTorch / JAX on raw kernel throughput. The 38-PR-in-2-days velocity is consistent with that thesis.

The North Star therefore reframes operationally from "world-governing framework" to **"canonical reference for differentiable named-tensor computation in modern C++"** — the framing captured in [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md). The four findings stop being obstacles and become the exact dependency surface a canonical reference would care about.

---

## Concrete action items derived from this brief

The Layer A consequences are captured in two ADRs:

- [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) — frames the project as the canonical reference for differentiable named-tensor computation. Refines (does not supersede) ADR-0010.
- [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md) — bundles four operational decisions: Dawn-via-vcpkg as the default WebGPU build path; gpu.cpp vendored under `third_party/`; notebook execution migrated to xeus-cpp; `std::linalg` shimmed through kokkos/stdBLAS.

The Layer B operational follow-ups (these will become PRs in subsequent slices):

- Pin `vcpkg.json` to a `dawn@20260410.x` baseline; document the build matrix for Linux/macOS/Windows.
- Vendor `gpu.hpp` under `third_party/gpu_cpp/` with a `VENDORED_FROM` file recording the upstream commit.
- File a vcpkg port for `gpu.cpp` upstream as a community contribution.
- Migrate Jupyter Book CI from `xeus-cling` to `xeus-cpp@0.10.0`; keep a `legacy_cpp17/` chapter referencing `xeus-cling@0.15.3`.
- Add a `tensor::linalg` namespace backed by `kokkos/stdBLAS` with a `feature_test` switch primed for `__cpp_lib_linalg`.
- Track three external signals quarterly: (i) gpu.cpp commit cadence and any v0.2.0 release, (ii) libc++/MSVC STL changelogs for `<linalg>` first-mention, (iii) xeus-cpp releases for Clang-23/Clang-24 alignment.

---

## Sources

External:

- [AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp)
- [gpu.cpp: portable GPU compute for C++ with WebGPU – Answer.AI](https://www.answer.ai/posts/2024-07-11--gpu-cpp.html)
- [GPU Programming from Scratch – Answer.AI](https://www.answer.ai/posts/2025-03-17-gpu-programming-scratch.html)
- [GPU Puzzles](https://gpupuzzles.answer.ai/intro)
- [vcpkg dawn package](https://vcpkg.io/en/package/dawn.html)
- [vcpkg Release 2026.04.27](https://github.com/microsoft/vcpkg/releases/tag/2026.04.27)
- [What's New in vcpkg (Nov 2025 – Jan 2026)](https://devblogs.microsoft.com/cppblog/whats-new-in-vcpkg-nov-2025-jan-2026/)
- [What's New in vcpkg (Apr 2026)](https://devblogs.microsoft.com/cppblog/whats-new-in-vcpkg-apr-2026/)
- [vcpkg dawn port request #41847](https://github.com/microsoft/vcpkg/issues/41847)
- [vcpkg dawn port request #22948 (original)](https://github.com/microsoft/vcpkg/issues/22948)
- [gfx-rs/wgpu-native](https://github.com/gfx-rs/wgpu-native)
- [Twinklebear/wgpu-cpp-starter](https://github.com/Twinklebear/wgpu-cpp-starter)
- [jupyter-xeus/xeus-cling](https://github.com/jupyter-xeus/xeus-cling)
- [xeus-cling tags](https://github.com/jupyter-xeus/xeus-cling/tags)
- [xeus-cling Issue #326: Can I specify C++20?](https://github.com/jupyter-xeus/xeus-cling/issues/326)
- [compiler-research/xeus-cpp](https://github.com/compiler-research/xeus-cpp)
- [xeus-cpp releases](https://github.com/compiler-research/xeus-cpp/releases)
- [Cling Transitions to LLVM's Clang-Repl](https://root.cern/blog/cling-in-llvm/)
- [Compiler Research Projects](https://compiler-research.org/projects/)
- [P1673R13: A free function linear algebra interface based on the BLAS](https://isocpp.org/files/papers/P1673R13.html)
- [cplusplus/papers issue #557 — P1673R13](https://github.com/cplusplus/papers/issues/557)
- [P3050R2: Fix C++26 by optimizing linalg::conjugated](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3050r2.html)
- [P3222R1: Fix C++26 by adding transposed special cases](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3222r1.html)
- [ISO C++ Committee Meeting Update: Hagenberg](https://community.latenode.com/t/iso-c-committee-meeting-update-c-26-feature-freeze-finalized-in-hagenberg/11926)
- [Compiler support for C++26 — cppreference](https://en.cppreference.com/cpp/compiler_support/26)
- [Standard library header `<linalg>` (C++26) — cppreference](https://en.cppreference.com/cpp/header/linalg)
- [libc++ C++2c status](https://libcxx.llvm.org/Status/Cxx2c.html)
- [GCC C++ Standards Support](https://gcc.gnu.org/projects/cxx-status.html)
- [microsoft/STL Changelog](https://github.com/microsoft/STL/wiki/Changelog)
- [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS)

Internal:

- [ADR-0001](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md), [ADR-0006](../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md), [ADR-0008](../arc42/09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md), [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md), [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md)
- [Phase 3 impl-plan](../impl-plans/2026-05-11_phase-3-webgpu.md)
- [Open discussion-points report (Axis E)](./2026-05-11_open-discussion-points.md)
- [Phase 1 retrospective](./2026-05-11_phase-1-retrospective.md)
- [Phase 2 + 2.5 retrospective](./2026-05-11_phase-2-and-2-5-retrospective.md)
