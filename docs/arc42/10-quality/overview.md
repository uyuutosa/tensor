---
status: Draft
owner: tensor
last-reviewed: 2026-05-11
---

# `tensor` — Quality Requirements (arc42 §10)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §10 (Quality Requirements)                               |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §10: this file makes the project's quality goals **measurable**. The top-of-tree quality priorities ([§1 §4](../01-introduction-and-goals/overview.md) — clarity > correctness > portability > performance) are restated here as a tree of testable scenarios.

## 1. Quality tree (ISO 25010-aligned, project-prioritised)

The top three priorities are ordered ([§1 §4](../01-introduction-and-goals/overview.md)); the rest are not less important, but are *enabled* by the top three.

```
tensor quality goals
├── Clarity (priority 1)
│   ├── QC-1 — Source legibility
│   ├── QC-2 — Diagnostic legibility
│   └── QC-3 — Documentation citability (ADR-0013)
│
├── Correctness (priority 2)
│   ├── QO-1 — Numerical agreement across backends
│   ├── QO-2 — Autograd correctness via finite-difference check
│   └── QO-3 — Reproducible build → byte-for-byte 2016 README replication
│
├── Portability (priority 3)
│   ├── QP-1 — Compiler matrix
│   ├── QP-2 — OS matrix
│   └── QP-3 — Substrate adaptability (KernelBackend port)
│
├── Performance (priority 4 — must not block the above)
│   ├── QF-1 — Reference baseline reproducibility
│   └── QF-2 — Eigen / WebGPU speedup envelope
│
└── Sustainability (cross-cutting)
    ├── QS-1 — Documentation freshness
    └── QS-2 — Substrate sturdiness (ADR-0014)
```

## 2. Quality scenarios

Each scenario is in the Source / Stimulus / Environment / Response / Response Measure form. They are intentionally concrete so a contributor knows when an attribute is upheld.

### QC-1 — Source legibility

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A reader who has never seen the codebase but is fluent in C++20 and named-axis tensor algebra.     |
| Stimulus          | Reads the umbrella header [`include/tensor/tensor.hpp`](../../../include/tensor/tensor.hpp) and walks down into one container of their choice. |
| Environment       | Repository at a tagged release; reader has the ADR sequence open alongside.                        |
| Response          | The reader can connect every public type and free function in the chosen container to either a math notation (paper / textbook) or an ADR. |
| Response measure  | Per [G-8 citability discipline](../01-introduction-and-goals/overview.md): every public name traces to a source. Half-yearly bibliography audit ([ADR-0015 §Compliance](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding ADR-0013) verifies. |

### QC-2 — Diagnostic legibility

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A reader who mis-uses the API (e.g. axis-label mismatch in `TypedTensor<T, "i">` + `TypedTensor<T, "j">`). |
| Stimulus          | Compiles a snippet that violates a concept constraint or a `static_assert`.                        |
| Environment       | GCC 11+, Clang 13+, MSVC 19.30+ — i.e. the supported compiler matrix.                              |
| Response          | The compiler diagnostic names the constraint that failed and the operand types involved.          |
| Response measure  | `tests/test_typed_tensor.cpp` includes failure-case probes; the README's `which-named-tensor-type` how-to documents expected diagnostics.    |

### QC-3 — Documentation citability ([ADR-0015](../09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding ADR-0013)

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A paper / textbook / downstream implementation author who wants to cite this work.                |
| Stimulus          | Looks for `CITATION.cff` and the bibliographic ADR sequence.                                       |
| Environment       | Repository at a tagged release reachable via the GitHub web UI.                                    |
| Response          | The author cites the work via the GitHub "Cite this repository" UI (CFF v1.2.0) and links specific ADRs as well-defined design choices. |
| Response measure  | `CITATION.cff` exists at repo root; CFF parses; references list at least four bibliographic ADRs.  |

### QO-1 — Numerical agreement across backends

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A learner running tutorial 08 (Hexagonal payoff) on their laptop.                                   |
| Stimulus          | Re-runs the same notebook with `-DTENSOR_KERNEL_BACKEND=reference` then `eigen`.                    |
| Environment       | Default 10-job CI matrix or local equivalent.                                                       |
| Response          | Output tensors agree element-wise to within `1e-9` for `double`, `1e-5` for `float`.               |
| Response measure  | [`tests/test_eigen_backend.cpp`](../../../tests/test_eigen_backend.cpp) and [`tests/test_webgpu_backend.cpp`](../../../tests/test_webgpu_backend.cpp) enforce these tolerances via the cross-backend test suite. CI green on every PR. |

### QO-2 — Autograd correctness via finite-difference check

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | Test runner (CI).                                                                                  |
| Stimulus          | Runs `gradient_check(f, x)` on every autograd primitive (`+`, `-`, `*`, `exp`, `log`, `relu`, `neg`, `dot`). |
| Environment       | Default 10-job CI matrix.                                                                          |
| Response          | Analytical gradient matches central-difference gradient.                                            |
| Response measure  | Per-primitive tolerance documented in [`tests/test_autograd_*.cpp`](../../../tests/); typically `1e-5` for `double`. CI green on every PR. |

### QO-3 — Reproducible byte-for-byte 2016 README replication

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A reader running [`tutorials/00_intro.ipynb`](../../../tutorials/00_intro.ipynb).                  |
| Stimulus          | Executes the named-axis broadcasting examples from the 2016 Qiita post on the new API.            |
| Environment       | xeus-cpp 0.10+ kernel; legacy xeus-cling supported via the smoke path.                             |
| Response          | The five 5×5 outer-sum tables and the `a*f = (1,4,7,10,13)`, `r*3 = (9,27,81,243,729)` outputs match the 2016 article byte-for-byte. |
| Response measure  | Tutorial 00 sections 3-5 carry expected-output cells against which the executed notebook is diffed. |

### QP-1 — Compiler matrix

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | Repository CI on every PR / push to `develop` / `main`.                                            |
| Stimulus          | Builds + tests the full repository.                                                                |
| Environment       | GitHub-hosted runners.                                                                              |
| Response          | All 10 jobs (Ubuntu × {GCC 11, Clang 13} × {Debug, Release} + macOS appleclang × {Debug, Release} + Windows MSVC × {Debug, Release} + Ubuntu Eigen Release + Ubuntu WebGPU-stub Release) pass. |
| Response measure  | CI badge green on `develop`. `vendored-check` lint gate passes.                                    |

### QP-2 — OS matrix

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A reader on Linux / macOS / Windows.                                                                |
| Stimulus          | `git clone && cmake --preset=default && cmake --build && ctest`.                                   |
| Environment       | Stock GCC / Clang / AppleClang / MSVC at the documented minimums.                                  |
| Response          | Build, test, bench succeed without OS-specific patches.                                            |
| Response measure  | QP-1 holds on all three OSes in CI.                                                                |

### QP-3 — Substrate adaptability (`KernelBackend` port)

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A future contributor adding a new backend (Kokkos, `std::linalg`, BLIS, …).                        |
| Stimulus          | Implements the 15-method `KernelBackend` concept ([ADR-0011](../09-decisions/0011-kernel-backend-port-api.md)) in a new header under `include/tensor/core/backend/<name>.hpp`. |
| Environment       | Any of the three OSes in QP-1.                                                                     |
| Response          | The Domain code (under `include/tensor/{core,autograd,tex}/` except `concepts.hpp`) requires zero modification. |
| Response measure  | `static_assert(KernelBackend<NewBackend>)` succeeds; the existing test suite passes when `-DTENSOR_KERNEL_BACKEND=<new>` is set.   |

### QF-1 — Reference baseline reproducibility

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | A reader running `bench/`.                                                                          |
| Stimulus          | `cmake --preset=default -DTENSOR_BUILD_BENCH=ON && ./build/.../tensor_bench`.                       |
| Environment       | Default release build, reference backend, single-core laptop.                                       |
| Response          | The three documented baseline cases — add 1M, matvec 1024², matmul 512³ — produce numbers within 2× of the documented baseline ([`docs/reports/2026-05-11_backend-performance-comparison.md`](../../reports/2026-05-11_backend-performance-comparison.md)). |
| Response measure  | The 2× tolerance is generous because the canonical-reference-quality framing prioritises clarity over absolute speed; the bench is for *signal*, not for benchmark-shopping. |

### QF-2 — Eigen / WebGPU speedup envelope

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | Future P2.5.M4 + P3.M4.2 perf reports.                                                              |
| Stimulus          | Same bench as QF-1, on `eigen` and (once GPU dispatch is wired) `webgpu` backends.                  |
| Environment       | Any modern laptop; for WebGPU, a Dawn-compatible GPU.                                              |
| Response          | Expected envelopes: Eigen 50–100× faster on matmul; WebGPU compute-bound at 512³ matmul, transfer-dominated for small element-wise. |
| Response measure  | Reports get a number per case; outliers > 10× from the envelope are an explicit signal to inspect. |

### QS-1 — Documentation freshness

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | Each PR that touches `include/`.                                                                    |
| Stimulus          | The PR's diff includes a `docs/` change (per `.claude/rules/documentation.md`).                    |
| Environment       | PR review.                                                                                          |
| Response          | The arc42 §5, detailed-design, or ADR file most relevant to the code change is updated.            |
| Response measure  | Manual audit in code review; periodic audits (see PR #11, PR #29, PR #34, PR #45) catch drift.     |

### QS-2 — Substrate sturdiness ([ADR-0014](../09-decisions/0014-external-substrate-strategy.md))

| Field             | Value                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------- |
| Source            | The maintainer's quarterly substrate review.                                                        |
| Stimulus          | Three external signals: (i) gpu.cpp commit cadence / v0.2.x releases, (ii) libc++ / MSVC STL `<linalg>` shipping, (iii) xeus-cpp Clang-23 / Clang-24 alignment. |
| Environment       | Audit notebook / report (Layer B).                                                                  |
| Response          | Each signal gets a one-line status update; substrate decisions in ADR-0014 are re-validated.       |
| Response measure  | Quarterly audit report under `docs/reports/`. First report due ~2026-08-11 (3 months post-ADR-0014). |

## 3. Cross-references

- §1 §4 quality goals (top three): [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §2 constraints bounding these qualities: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md)
- §5 components responsible: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §6 runtime scenarios (TBD): [`../06-runtime/`](../06-runtime/)
- §11 risks tied to each quality scenario (TBD): [`../11-risks/`](../11-risks/)
- ADRs cited above: [`../09-decisions/`](../09-decisions/)
