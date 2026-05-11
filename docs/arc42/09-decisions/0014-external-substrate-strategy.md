---
status: Accepted
owner: tensor
last-reviewed: 2026-05-11
---

# ADR-0014: Adopt external-substrate strategy — Dawn via vcpkg, gpu.cpp vendored, xeus-cpp for notebooks, kokkos/stdBLAS as the `std::linalg` shim

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-11                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-11)                                         |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Triggered by | [external-substrate research report (2026-05-11)](../../reports/2026-05-11_external-substrate-research.md) and the canonical-reference framing in [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md). |
| Refines   | [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md), [ADR-0012](0012-webgpu-adapter-implementation-design.md) (operational details only; the strategic decisions in those ADRs remain in force). |

---

## Context and Problem Statement

The project depends on four external substrates whose stability and availability shape every upcoming Phase:

1. **WebGPU runtime + C++ wrapper** ([ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0012](0012-webgpu-adapter-implementation-design.md)) — needed by Phase 3 (P3.M3+ WGSL kernels).
2. **Jupyter C++ kernel** ([ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)) — needed by every executable notebook and by the Jupyter Book scaffold.
3. **Standard linear algebra** ([ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md) deferred this; conceptually it is the "would-be `KernelBackend`" path that arrives with C++26).
4. **vcpkg port availability** for any of (1)–(3) — determines whether CI is reproducible across Linux / macOS / Windows.

The [external-substrate research report (2026-05-11)](../../reports/2026-05-11_external-substrate-research.md) audited each of these as of May 2026 and surfaced four discrete, independently-decidable findings:

- **Dawn** is now in vcpkg (`20260410.140140`, refreshed 2026-04-20) — a substantial P3.M3 unblock.
- **gpu.cpp** is in soft-maintenance with bus-factor 1; no vcpkg port exists.
- **xeus-cling is effectively frozen** at v0.15.3 / C++17 / Clang-13; its successor **xeus-cpp 0.10.0** (2026-04-02) supports modern Clang and is the path forward.
- **`std::linalg` (P1673)** is C++26-final but no vendor STL has shipped it; **kokkos/stdBLAS** is the only working implementation.

Each substrate has a "right answer" given the research. The question is whether to bundle them into one operational ADR or proliferate four. Bundling wins because the four decisions are tightly coupled (they all derive from the same audit; they all support the same canonical-reference framing in [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) and because they ship together as one PR slice.

---

## Decision Drivers

- **DD-1**: **Substrate sturdiness** — anything bus-factor 1 must be vendored, not linked.
- **DD-2**: **CI reproducibility** — every dependency should have a vcpkg path or an in-tree path. No "bring your own."
- **DD-3**: **Canonical-reference posture** ([ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) — substrate choices should advertise the project's depth and durability, not its fragility.
- **DD-4**: **API stability across substrate migration** — when `<linalg>` ships in libstdc++ / libc++ / MSVC STL, the project's public surface must absorb the change without a breaking release.
- **DD-5**: **Maintainer bandwidth** — each substrate decision must be cheap to execute (≤ 1 PR slice).

---

## Considered Options

For each of the four substrates, the considered options are:

### Substrate 1 — WebGPU build path

1. **Build Dawn from depot_tools per machine** (the original ADR-0012 assumption).
2. **Use Dawn via vcpkg** as a manifest dependency (chosen — newly available).
3. **Use wgpu-native via FetchContent** (rejected — no vcpkg port; Rust toolchain in CI).
4. **Defer GPU build entirely until P3.M3 lands** (rejected — leaves the third backend slot un-buildable).

### Substrate 2 — gpu.cpp integration

1. **Add as git submodule** (rejected — submodules are operationally awkward in a vcpkg-manifest project).
2. **File and consume an upstream vcpkg port** (rejected — none exists yet; filing takes 30 lines but acceptance latency is multi-week).
3. **Vendor under `third_party/gpu_cpp/`** with a `VENDORED_FROM` file recording the upstream commit (chosen).
4. **Skip gpu.cpp; talk to Dawn directly** (rejected — the readability win that ADR-0012 cited as the reason to pick gpu.cpp still holds for the canonical-reference framing).

### Substrate 3 — Jupyter C++ kernel

1. **Stay on xeus-cling** (rejected — frozen at C++17 / Clang-13; class-type NTTPs like `FixedString` are iffy).
2. **Migrate primary notebook execution to xeus-cpp 0.10.0+** (chosen).
3. **Provide both kernels with separate notebook copies** (rejected — doubles the maintenance surface).
4. **Drop executable notebooks entirely; emit pre-rendered output** (rejected — loses the *the formula is the program* demonstrability that the project's identity depends on).

### Substrate 4 — `std::linalg` shim

1. **Wait for `<linalg>` to ship in libstdc++ / libc++ / MSVC STL** (rejected — realistic timeline 2028-2029).
2. **Implement a custom BLAS-shim namespace inside `tensor::core`** (rejected — duplicates kokkos/stdBLAS's work).
3. **Wrap kokkos/stdBLAS behind a `tensor::linalg` namespace** with a `__cpp_lib_linalg` feature-test switch (chosen).
4. **Skip `<linalg>` entirely — Eigen backend covers the equivalent surface** (rejected — Eigen is one backend; canonical-reference posture wants a portable, no-deps `<linalg>`-style path available too).

---

## Decision Outcome

**Chosen options: (1) Dawn-via-vcpkg, (2) Vendor gpu.cpp, (3) Migrate to xeus-cpp, (4) `tensor::linalg` shim over kokkos/stdBLAS.**

### Detail

1. **Dawn via vcpkg.**
   - Add `dawn` to `vcpkg.json` and pin the manifest baseline to a commit that includes `dawn@20260410.140140` or later.
   - The WebGPU backend's CMake plumbing (`-DTENSOR_KERNEL_BACKEND=webgpu`) gains a `find_package(dawn CONFIG REQUIRED)` call gated on the active backend (mirroring how Eigen is wired today).
   - The Phase 3 P3.M3 milestone (WGSL element-wise kernels) is materially unblocked by this — the [Phase 3 impl-plan](../../impl-plans/2026-05-11_phase-3-webgpu.md) is updated accordingly.

2. **Vendor gpu.cpp under `third_party/gpu_cpp/`.**
   - Copy `gpu.hpp` from `AnswerDotAI/gpu.cpp` at a pinned commit; record the commit SHA and date in `third_party/gpu_cpp/VENDORED_FROM`.
   - The header is included from the WebGPU backend; the rest of the upstream repo (CMake, examples) is **not** vendored.
   - When the upstream produces a v0.2.0 release or a notable fix, the maintainer can re-vendor by replacing the header and updating `VENDORED_FROM`.
   - A standing follow-up: contribute a `gpu.cpp` vcpkg port upstream as a community PR; if that lands, this ADR can be revisited to switch from vendored to vcpkg.

3. **Migrate notebook execution to xeus-cpp 0.10.0+.**
   - The notebook CI workflow (`.github/workflows/notebook-ci.yml`) switches its execute job to install `xeus-cpp` instead of `xeus-cling`.
   - The JSON-validation job stays kernel-agnostic.
   - One notebook (the oldest, `00_intro.ipynb`) keeps a parallel `xeus-cling` smoke target under `legacy_cpp17/` to verify the C++17 subset still works for users on conda-forge stable channels. All other notebooks target xeus-cpp.
   - The `book/intro.md` "Reading this book" section updates to name xeus-cpp as the primary kernel.

4. **`tensor::linalg` shim over kokkos/stdBLAS.**
   - Add `kokkos-kernels` (or vendor `stdBLAS` headers directly — TBD by the implementation PR) as an optional dependency.
   - Expose a `tensor::linalg` namespace whose names mirror P1673R13 (`gemv`, `gemm`, `symm`, `trsm`, …) and forward to stdBLAS.
   - Gate by `#if __cpp_lib_linalg && __has_include(<linalg>)` so that when libstdc++ / libc++ / MSVC STL ship `<linalg>`, the namespace transparently re-exports the standard names without a public API break.
   - This is **a fourth backend**, conceptually — the `KernelBackend` port from [ADR-0011](0011-kernel-backend-port-api.md) can be backed by `tensor::linalg`. The implementation PR for this lands no earlier than Phase 5; this ADR fixes the design so the namespace shape is preempted now.

### Y-statement summary

> In the context of **the May-2026 external-substrate audit and the canonical-reference framing in [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)**, facing **four discrete substrate decisions whose answers are coupled by the canonical-reference posture**, we decided for **Dawn via vcpkg + gpu.cpp vendored + xeus-cpp for notebooks + kokkos/stdBLAS as the `<linalg>` shim**, to achieve **substrate sturdiness without inheriting any single bus-factor-1 dependency and without locking the project to a frozen C++17 / Clang-13 toolchain**, accepting **the operational cost of vendoring gpu.cpp (re-vendor on upstream change) and migrating notebook CI to a younger kernel (xeus-cpp 0.10.0 is recent; its own track record is short)**.

---

## Pros and Cons of the Options

(Already discussed per substrate in §Considered Options. The four-option chosen bundle is internally consistent: it minimises external bus-factor risk; it makes every substrate either vcpkg-resolvable or in-tree; it future-proofs the API surface for `<linalg>`.)

---

## Consequences

### Positive

- **Phase 3 P3.M3 unblocked.** The vcpkg Dawn port turns "build Dawn yourself" into "add one line to `vcpkg.json`". The Phase 3 plan can commit to a date.
- **gpu.cpp's bus-factor 1 stops being a project risk** — by vendoring, the project becomes the upstream-of-itself for that header.
- **Notebooks gain class-type NTTP support** (critical for the `TypedTensor<T, "i", "j", ...>` story), modern Clang diagnostics, and C++23 features when wanted.
- **The `<linalg>` ship date stops mattering to the public API.** Whenever it ships, the shim flips and downstream sees no break.
- **All four substrate choices ladder up to [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)**: each is the choice a canonical-reference would make.

### Negative

- **Vendoring increases the repo size.** `gpu.hpp` is ~30 KB; tolerable but the project crosses the "the repo contains code it didn't write" line.
- **xeus-cpp is younger than xeus-cling.** v0.10.0 (April 2026) is recent. If a critical bug surfaces in xeus-cpp during a Jupyter Book deploy, the workflow has to fall back to pre-rendered output until upstream fixes land.
- **The shim namespace `tensor::linalg`** is one more public surface to maintain. Mitigation: keep the shim minimal — only the operators actually used by the autograd / Eigen backends.
- **Re-vendoring discipline required.** A `VENDORED_FROM` file is easy to forget; mitigate with a `tools/check-vendored.sh` script and a PR-time reminder in `CONTRIBUTING.md`.

### Neutral

- [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md), [ADR-0012](0012-webgpu-adapter-implementation-design.md) all remain Accepted; this ADR refines their operational details (which build path; which kernel) without superseding the strategic decisions in them.

### Follow-ups

- [ ] Update `vcpkg.json` to add `dawn` and (later) `kokkos-kernels`; pin the manifest baseline.
- [ ] Update [Phase 3 impl-plan](../../impl-plans/2026-05-11_phase-3-webgpu.md) to mark P3.M3 substantially unblocked and call out the vendored-gpu.cpp + Dawn-vcpkg path.
- [ ] Vendor `gpu.hpp` under `third_party/gpu_cpp/` in a dedicated PR (no functional change; just the header + `VENDORED_FROM`).
- [ ] Switch `.github/workflows/notebook-ci.yml`'s execute job to xeus-cpp 0.10.0; preserve an xeus-cling legacy smoke job.
- [ ] Sketch a Phase-5 impl-plan for the `tensor::linalg` shim; no code yet.
- [ ] Add `tools/check-vendored.sh` (lints presence of `VENDORED_FROM` for every directory under `third_party/`).
- [ ] Add a contributor-facing paragraph in `CONTRIBUTING.md` on the "re-vendor instead of patch" discipline.

---

## Compliance / Validation

- **Verification (Dawn)**: `find_package(dawn)` resolves under `-DTENSOR_KERNEL_BACKEND=webgpu` with the pinned `vcpkg.json` baseline on Linux + macOS + Windows.
- **Verification (gpu.cpp)**: every directory under `third_party/` contains a `VENDORED_FROM` file; the `tools/check-vendored.sh` CI job fails the build if any is missing or stale.
- **Verification (xeus-cpp)**: the notebook-CI workflow's execute job successfully runs `00_intro.ipynb` and `05_autograd-from-scratch.ipynb` end-to-end on the weekly cron.
- **Verification (`tensor::linalg`)**: when implemented (Phase 5+), unit tests exercise the namespace under both `__cpp_lib_linalg`-defined and -undefined builds; both paths produce numerically identical results.
- **Frequency**: per-PR for `VENDORED_FROM`; weekly for notebook-CI cron; per-release for the full vcpkg manifest baseline check.

---

## More Information

### Related ADRs

- Refines (operational details only): [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md), [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md), [ADR-0012](0012-webgpu-adapter-implementation-design.md)
- Strategic foundation: [ADR-0013](0013-reframe-as-canonical-reference-for-named-tensor-computation.md)
- Port-shape this ADR's `tensor::linalg` shim respects: [ADR-0011](0011-kernel-backend-port-api.md)

### References

- [External-substrate research report (2026-05-11)](../../reports/2026-05-11_external-substrate-research.md) — the audit underlying every decision in this ADR.
- [AnswerDotAI/gpu.cpp](https://github.com/AnswerDotAI/gpu.cpp) — substrate vendored under `third_party/gpu_cpp/`.
- [vcpkg dawn package](https://vcpkg.io/en/package/dawn.html) — the now-current vcpkg port.
- [compiler-research/xeus-cpp](https://github.com/compiler-research/xeus-cpp) — primary notebook kernel.
- [kokkos/stdBLAS](https://github.com/kokkos/stdBLAS) — `std::linalg` reference implementation.
- [P1673R13](https://isocpp.org/files/papers/P1673R13.html) — the standard the shim respects.
