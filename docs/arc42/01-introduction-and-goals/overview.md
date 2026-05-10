---
status: Draft
owner: tensor
last-reviewed: 2026-05-10
---

# `tensor` — Introduction and Goals

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §1 (Introduction and Goals)                              |
| Owner         | uyuutosa                                                       |
| Related       | ADR-0001 — pivot decision; `docs/reports/2026-05-10_tensor-revival-landscape.md` |
| Last Updated  | 2026-05-10                                                     |

## Revision history

| Version | Date       | Summary                                                        |
| ------- | ---------- | -------------------------------------------------------------- |
| 0.1.0   | 2026-05-10 | Initial draft after revival pivot (ADR-0001)                   |

---

## TL;DR

`tensor` is a **header-only C++20/23 library and bundled Jupyter book that teaches named-axis tensor algebra** — Einstein-notation operations, function tensors, reference (recursive) tensors, and convolutions reformulated as tensor inner products — with a path through automatic differentiation and WebGPU acceleration. Authoring is bidirectional with TeX / LyX so that, in the project's own slogan, *the formula is the program*. It is explicitly an educational artifact, not a production library.

---

## 1. System identity

The `tensor` repository began in ~2016 as a personal C++ template library built around an unusual idea: tensors that carry **axis labels** (`a_i`, `b_j`) and operate on each other through Einstein-style broadcasting (`a_i + b_j → c_{ij}`). The library shipped function-tensors (tensor elements that are functions), reference-tensors (tensor elements that recursively reference the previous element along an axis), and a [proof-of-concept reformulation of N-D convolution as a tensor inner product](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74).

It went dormant after a single commit run. In May 2026 the project is being revived under explicit new positioning (see [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md)): a **teaching library and book** for modern C++ tensor algebra, not a production peer to Eigen, xtensor, libtorch, Kokkos, or the upcoming `std::mdspan` + `std::linalg`.

The 2016 codebase will be retired. The rewrite targets C++20 + `mdspan` interop ([ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)), CMake + vcpkg ([ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)), and a hybrid runtime / NTTP named-axis API ([ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md)).

## 2. Stakeholders

| Stakeholder                          | Interest                                                                                  |
| ------------------------------------ | ----------------------------------------------------------------------------------------- |
| Self-taught C++ learners exploring tensor algebra | Want a small, hackable, idiomatic library + readable notebooks. Primary audience.          |
| University / bootcamp instructors    | Want assignable teaching material for "build your own ML framework" or "modern C++" courses. |
| Researchers in tensor-DSL design     | Want a real working artifact to reference for named-axis API ergonomics in C++.            |
| Practitioners considering production tensor libs | Should be redirected to Eigen / xtensor / libtorch / Kokkos / `std::linalg`. Not the audience. |
| The maintainer (uyuutosa)            | Wants to revive a pet project, develop ideas, and produce a public artifact.               |

## 3. Goals (G-1 … G-7)

Each goal is phrased so success is observable.

- **G-1 — Named-axis algebra is the headline feature.** Every public operation respects axis labels. Mismatched axes are caught at compile time when the user opts in via `_ax` literals, at runtime otherwise. (See [ADR-0004](../09-decisions/0004-adopt-hybrid-named-axis-api.md).)
- **G-2 — Modern C++ idioms throughout.** No raw `new` / owning references. Concepts, NTTPs, `consteval`, ranges, `mdspan` interop. (See [ADR-0002](../09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md).)
- **G-3 — "The formula is the program".** A LaTeX subset (Einstein notation, `\sum`, indexed arithmetic) parses at compile time via `_tex` UDL into the same expression graph as direct C++ tensor expressions. LyX export is a follow-on deliverable. (See [ADR-0005](../09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md).)
- **G-4 — End-to-end teaching arc.** The bundled tutorials walk a learner from `a_i + b_j` through autograd to a small MLP on MNIST, with optional WebGPU acceleration. (See [ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md), [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md).)
- **G-5 — Zero-friction installation for learners.** `git clone && cmake --preset=default && jupyter lab tutorials/` works on Linux, macOS, Windows. No proprietary GPU toolchain required. (See [ADR-0003](../09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md), [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md).)
- **G-6 — Living book, not abandoned tutorial.** A Jupyter Book site is published from the `tutorials/` directory at every release; CI executes every notebook end-to-end. (See [ADR-0008](../09-decisions/0008-distribute-as-header-only-with-jupyter-tutorials.md).)
- **G-7 — Honest scope disclaimers.** README, repo description, and Jupyter Book front-page state plainly that `tensor` is educational; production users are pointed at Eigen / xtensor / libtorch / Kokkos / `std::linalg`. No benchmark-shopping.

## 4. Quality goals

In priority order (top three):

| Priority | Quality       | Why                                                                                                  |
| -------- | ------------- | ---------------------------------------------------------------------------------------------------- |
| 1        | **Clarity**   | Every public API surface, every notebook, every diagnostic message is optimized for *being read*.    |
| 2        | **Correctness** | Mathematical correctness is non-negotiable for a teaching artifact. All ops have property tests; gradients are gradient-checked against finite differences. |
| 3        | **Portability** | Must build on GCC 11+, Clang 13+, MSVC 19.30+, and run notebook tutorials on Linux/macOS/Windows. |

Performance is explicitly ranked **below** clarity and correctness. The library is allowed to be slower than Eigen / libtorch by an order of magnitude as long as the implementation reads like the math and the diagnostics are clear. (See [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md).)

## 5. Out of scope

Captured here so future reviewers do not litigate them as gaps.

- **Production deployment.** No ABI stability, no semver-after-1.0 commitments, no LTS branches.
- **Coverage parity with Eigen / xtensor / libtorch.** We will skip operations that do not teach.
- **Distributed training.** Single-node, period.
- **Sparse tensors as a first-class citizen.** Possible later; not in the first release.
- **CUDA-direct GPU.** Disqualified in [ADR-0006](../09-decisions/0006-adopt-webgpu-as-gpu-backend.md) on educational-cost-of-entry grounds.
- **Forward-mode and source-to-source autograd.** Reachable as future complementary backends ([ADR-0007](../09-decisions/0007-adopt-autograd-as-first-class-subsystem.md)).

## 6. Success criteria (concrete, measurable)

The first release is `0.1.0`. Success at `0.1.0` means:

- [ ] All ADR-0001 follow-ups are closed.
- [x] CMake build green on the {GCC 11, Clang 13, MSVC 19.30} × {Debug, Release} matrix in CI. *(achieved 2026-05-11 via PR #9.)*
- [x] `tutorials/00_intro.ipynb` reproduces the 2016 blog post's convolutions-as-inner-products in the rewritten library. *(shipped 2026-05-10 via PR #8; xeus-cling end-to-end CI execution is the Phase 1.5 follow-up.)*
- [ ] `tutorials/05_autograd-from-scratch.ipynb` trains a 3-layer MLP on a toy dataset. *(Phase 2 — see [`../../impl-plans/2026-05-11_phase-2-autograd.md`](../../impl-plans/2026-05-11_phase-2-autograd.md), P2.M5 + P2.M6.)*
- [ ] `tutorials/06_webgpu-acceleration.ipynb` runs a named-axis matmul on the WebGPU backend and matches CPU results. *(Phase 3.)*
- [ ] Jupyter Book site is reachable at the repo's GitHub Pages URL. *(Phase 4.)*
- [x] README's first paragraph leads with the educational pitch and disclaims production use. *(shipped 2026-05-10 via PR #2; reinforced in the 2026-05-11 docs sync.)*

For interim phase-by-phase deliverables, see the dated impl-plans under [`../../impl-plans/`](../../impl-plans/).

## 7. Cross-references

- §2 Architecture Constraints (TBD): C++20 baseline, header-only, MIT licence, solo-maintainer bandwidth.
- §3 Context and Scope: [`../03-context-and-scope/system-context.md`](../03-context-and-scope/system-context.md).
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md).
- §5 Building Blocks: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md).
- §9 Decisions: [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) … [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).
- §10 Quality (TBD): clarity / correctness / portability, with measurable definitions.
