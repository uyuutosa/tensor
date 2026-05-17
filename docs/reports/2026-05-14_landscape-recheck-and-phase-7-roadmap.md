---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Landscape recheck (2026-05-14) + Phase 7+ strategic roadmap

| Metadata        | Value                                                                                  |
| --------------- | -------------------------------------------------------------------------------------- |
| Status          | Stable                                                                                 |
| Type            | Layer B — landscape report + multi-phase strategic plan                                |
| Owner           | uyuutosa                                                                               |
| Trigger         | Maintainer 2026-05-14 directive — "業界動向詳しく調べて勝ち筋のあるimpl planロードマップを作りたい" |
| Predecessor     | [`./2026-05-12_landscape-recheck-and-adversarial-review.md`](./2026-05-12_landscape-recheck-and-adversarial-review.md), [`./2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md) |
| Successor (planned) | Phase 7 retrospective + 6-month landscape audit at 2026-11-11 (per ADR-0017)        |

## TL;DR

The May-2012 landscape recheck reported §A.5: *"no first-class named-axis tensor exists in production Python ML"*. **Two days later (2026-05-14), this needs refinement**: the JAX side (Haliax + Penzai, both 2024-2025 vintage) has substantially filled the named-axes-in-production-Python-ML wedge. **The C++ angle, the `_tex` UDL, and the canonical-reference-quality docs discipline remain distinctive** — but the strategic position has shifted from "first mover" to "specialist". This report:

1. Re-surveys the named-tensor / DSL / educational-ML landscape as of 2026-05.
2. Identifies the project's remaining moats and the gaps that closed.
3. Proposes a **Phase 7+ roadmap** of 6 candidates ranked by "win-likelihood × strategic fit", and **recommends a 3-phase sequence**: Phase 7a (Documentation-as-Product) → Phase 7b (R-P6.5.5 lift) → Phase 7c (`_tex` parser expansion) leading to `0.4.0`.

## 1. Landscape recheck

### 1.1 Named-tensor libraries — current state (2026-05)

| Library | Host language | Differentiable? | Named-axis treatment | Status / Activity (2026-05) | Distinctive vs `tensor` |
| ------- | ------------- | --------------- | -------------------- | ----------------------------- | ------------------------ |
| **PyTorch named tensors** | Python (C++ backend) | yes (via PyTorch autograd) | optional, post-hoc | **Still "experimental and prototype"** — same status as 2021 ([PyTorch docs](https://docs.pytorch.org/docs/2.12/named_tensor.html)). The wedge stays open here. | Not by-default named; not citable as a worked example. |
| **xarray** | Python (NumPy backend) | no (DataArray is purely data-structured) | first-class | Stable since 2014. Scientific-computing focus (climate / geo). | No autograd, no GPU acceleration beyond Dask, no compile-time-checked axes. |
| **einops** | Python (NumPy/PyTorch/JAX/TF/MLX wrapper) | inherits from host | notation only | Mature; ICLR 2022 paper. | A *notation layer*, not a tensor library — operates on host-framework tensors. |
| **einx** | Python (NumPy/PyTorch/JAX/TF/MLX wrapper) | inherits from host | notation only | **ICLR 2026 oral** — academic traction. Compatible with einops + adds `[]`-brackets and ellipsis. | Same — a notation layer over host frameworks. |
| **Haliax** | JAX | yes (via JAX) | first-class `NamedArray` | **Production-scale** — powers Levanter / Marin, trained 70B-param LLMs on TPU v4-2048. **As of Nov 2025, Levanter merged into Marin** (Stanford CRFM monorepo) — active development. | Production-shape; FSDP + tensor parallelism. The "first-class named tensor in production Python ML" gap has been filled here. |
| **Penzai** | JAX | yes (via JAX) | first-class `NamedArray` PyTree | Active. Google DeepMind. Notebook-ergonomics-first; partially inspired by Haliax. | Notebook ergonomics; LLM-focused. |
| **Nx** | Elixir | yes (custom autograd) | first-class | Active. Erlang VM ecosystem. | Wrong language for the C++/Python audience. |
| **tinygrad** | Python (C/shader components) | yes (own autograd) | not named-axis | Active. Minimal (~1000s of lines). 12 primitive ops. RISC-like philosophy. | Minimal ML framework, not named-axis; competes with `tensor`'s educational positioning differently. |

### 1.2 What changed between the 2026-05-12 landscape recheck and today

The May-12 report's §A.5 finding ("no first-class named-axis tensor exists in production Python ML") is **now partially false**. Two days of intermediate landscape activity (which the May-12 scan missed) plus a more targeted recheck reveal:

- **Haliax (Stanford CRFM) is production-grade**. It powers Levanter, which trained LLMs at 70B params. The Marin merger (Nov 2025) means active development continues.
- **Penzai (Google DeepMind)** brings major-vendor backing to the named-axes pattern in JAX.
- **einx (ICLR 2026 oral)** legitimises the notation academically.

Effect on `tensor`'s competitive position: the "first-class named-axis Python ML library" wedge that ADR-0018 / Phase 6 was aiming at is **substantially served by Haliax + Penzai in the JAX ecosystem**. The Python SDK we shipped is not a unique offering; it's one named-axis option among several.

### 1.3 What did NOT change — the moats that hold

The project's distinctive position in 2026-05-14:

1. **C++20 hybrid named-axis API.** Compile-time NTTP `LabelTag<"i">` + runtime `DynamicShape` ([ADR-0004](../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md)). **No competitor has both compile-time-checked and runtime-rank named axes in the same library.** Haliax / Penzai are runtime-only (JAX traces don't constraint axes at C++ compile time). xarray is runtime-only. einx / einops are positional with named overlays.
2. **`_tex` UDL — "the formula is the program".** A LaTeX subset that parses at compile time via `R"(c_{ij} = a_i b_j)"_tex` into the same AST the runtime `tex::parse(s)` produces ([ADR-0005](../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md)). **No competitor offers this.** einx has a syntax that is *inspired by* Einstein notation but is not actual LaTeX-subset parsing; einops is positional letters.
3. **Hexagonal-lite + ports + adapters.** Three swappable `KernelBackend` adapters (reference / Eigen / WebGPU); the Python SDK is a DrivingAdapter consuming the same C++ Domain. **No competitor exposes this architecture as a teaching surface.** Haliax / Penzai are JAX-trace-based; the underlying XLA compiler is opaque to the user.
4. **Canonical-reference-quality docs discipline** ([ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md)) — arc42 §1–12, 19 ADRs, 8 detailed-design instances, full Diátaxis user-manual, ~125 markdown files. **No competitor's documentation system carries this much process artefact**. Haliax + Penzai have READMEs + API docs; not bibliography-discipline-grade.
5. **Header-only C++20 distribution** — the educational-first install path is 5-min from `git clone` to `cmake --preset=default && ctest`. No JAX install, no CUDA install, no proprietary GPU SDK.

### 1.4 Honest weaknesses (vs the named-axes JAX side)

- **No production scale story**. Haliax did 70B params on TPU v4-2048. `tensor`'s `bench/` measures small inputs on RTX 3090.
- **Python SDK is shallow**. Phase 6's Python surface is `DynamicTensor` + arithmetic + `contract` + autograd + `tex`. Haliax has FSDP / tensor parallelism / pjit integration. Penzai has tree-of-arrays + nnx-style integration.
- **No GPU compiler / autotuner**. Other libraries route through XLA / Triton / TVM and get autotuning for free. `tensor::core::backend::webgpu` is hand-written WGSL kernels.
- **No JAX-style transformations** (`vmap`, `pmap`, `jit`, `grad`). The autograd is tape-based reverse-mode only.

These aren't bugs — they're the explicit scope choices of [ADR-0001](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) / [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md): educational-first, production-capable via backend adapters, no operator-coverage commitment. The strategic question is whether to *narrow* into these strengths or *widen* into the gaps.

## 2. Strategic reframe — narrow into the moats

Given the landscape recheck, the project's winning play is **NOT** "compete with Haliax on production-scale named-tensor ML". That race is already substantially run.

The project's winning play **IS**: be the **canonical reference** for *the algebra* of named-axis differentiable tensors, in *modern C++*, with *citation-grade documentation* and *educational hexagonal-lite architecture*. The audience is:

- **Future implementers** porting the substrate to a new language (Rust / Zig / Mojo / WGSL-on-browser) — they cite tensor's ADRs as the design bibliography.
- **Textbook authors** writing about named-axis tensor algebra — they cite the `_tex` UDL as the worked example of "the formula is the program".
- **Computer-vision learners** working through MVG via autograd — they use the `python/notebooks/03_multifocal-tensors.ipynb` + `04_python-bundle-adjustment-perspective.ipynb` as paper-style replications.
- **Modern C++ teachers** demonstrating NTTP / `constexpr` / concepts / `mdspan` — they use `tensor::core` as a non-trivial worked example.

The project is **not** for production-scale ML practitioners. That audience is correctly redirected to Haliax / Penzai / Marin.

This reframe doesn't change ADR-0001 / ADR-0010 / ADR-0015 — they already said "educational-first, production-capable via backend adapters, citability discipline". What changes is the **competitive positioning**: from "first-mover in named-axis Python ML" (an open landscape in May 2026) to "specialist reference in named-axis tensor algebra" (the residual position after Haliax / Penzai filled the Python ML gap).

## 3. Phase 7+ candidates — ranked

Six candidates considered. Ranking on three axes: **strategic fit** (with the narrowed positioning), **win likelihood** (probability of producing a citable artifact), **effort budget** (maintainer's solo bandwidth).

### 3a. Documentation-as-Product — package the canonical reference

**What.** Take the existing arc42 + detailed-design + Diátaxis + tutorials artefact, package it as a downloadable PDF + a citable website artefact. Target ICLR / NeurIPS workshop submission (named tensor notation paper companion) or a textbook draft. Use the `_tex` UDL as the headline differentiator.

**Strategic fit** 🟢 highest. Directly cashes in on ADR-0015's citability discipline + the docs-system PDCA work (PRs #121, #122) that already brought 100% green coverage of Phase 6 + 6.5 across all 5 documentation layers. The asset exists; the productisation is the missing step.

**Win likelihood** 🟢 high. The artefact already exists at >95% completeness; the work is packaging + publishing.

**Effort budget** 🟡 medium (1-2 weeks). Mostly editorial work + a Jupyter-Book-to-PDF pipeline + a citation flow.

**Output** — a `0.4.0`-tagged release of the docs site as a Reference Artifact + a NeurIPS workshop paper draft under `docs/reports/`.

### 3b. R-P6.5.5 lift — type-owner-separation refactor

**What.** Per [`./2026-05-14_r-p6-5-5-lift-exploration.md`](./2026-05-14_r-p6-5-5-lift-exploration.md), split `python/src/_tensor_native.cpp` into a types-owner module + 3 backend-specific kernel modules. Closes the "single-backend-per-process" constraint that surfaced in Phase 6.5 M3.

**Strategic fit** 🟡 medium. Closes a known tech-debt item but doesn't expand the moats. Mostly a UX improvement for the Python SDK adopters who *do* exist.

**Win likelihood** 🟢 high. The path is known (nanobind upstream discussion #1067 confirms the pattern); the impl-plan + ADR + retrospective shape is the existing Phase 6.5 template.

**Effort budget** 🟡 medium (3-5 days). C++ refactor + perf measurement + new ADR.

**Output** — Phase 6.6 milestone shipping with `0.3.0` or `0.4.0`. ADR-0020 superseding ADR-0019 §"set_backend rebind" point.

### 3c. `_tex` parser expansion — `\prod` / `\delta` / `\nabla`

**What.** Extend the LaTeX-subset parser to cover product-reduction (`\prod_i`), Kronecker delta (`\delta_{ij}`), and gradient operator (`\nabla`). `\nabla` is the breakout feature: an autograd-aware formula `\nabla_x f(x)` becomes a `backward()`-prepared expression.

**Strategic fit** 🟢 high. Extends the project's MOST distinctive feature (the `_tex` UDL). The `\nabla` operator would put `tensor` ahead of every competitor on "the formula is the program" semantics — none of them parse the `∇` symbol into a differentiation-aware AST.

**Win likelihood** 🟡 medium. The parser is straightforward; the `\nabla` semantics need design work (when does `\nabla` bind? Single-variable? Multi-axis? Does it produce a Jacobian?).

**Effort budget** 🟡 medium (1-2 weeks). Parser additions + Evaluator visitor entries + tests + a notebook demonstrating each new operator.

**Output** — Phase 7c milestone; one new tutorial notebook (`tutorials/09_tex-parser-expansion.ipynb`) + `python/notebooks/06_python-tex-nabla.ipynb`.

### 3d. WebGPU coverage closeout — 12/15 → 15/15

**What.** Implement `reduce_sum` / `unbroadcast` / non-simple `contract` on WGSL ([`docs/detailed-design/kernel-backend-port.md` §8 future work](../detailed-design/kernel-backend-port.md)). Brings the WebGPU adapter to full coverage on `float`.

**Strategic fit** 🟡 medium. Improves the WebGPU adapter; doesn't introduce a new strategic differentiator.

**Win likelihood** 🟢 high. The pattern is established (the existing 12-of-15 methods are templates).

**Effort budget** 🟡 medium (1-2 weeks). WGSL writing + dispatch wiring + cross-validation tests.

**Output** — Phase 7d milestone; updates `webgpu-*.md` detailed-designs.

### 3e. Comparison study — named-tensor library landscape paper

**What.** A `docs/reports/2026-MM-DD_named-tensor-library-comparison.md` putting `tensor` in matrix against xarray / Haliax / Penzai / einx / PyTorch named tensors. Citable for the textbook-author audience.

**Strategic fit** 🟢 high. Bibliography-discipline output (ADR-0015 G-8). Useful for future implementers citing tensor.

**Win likelihood** 🟢 high. Research + writing only; no implementation risk.

**Effort budget** 🟢 small (3-5 days). Reading + benchmarking + writing.

**Output** — single Layer-B report; can be folded into Phase 7a (Documentation-as-Product) instead of a standalone phase.

### 3f. WASM / browser build — Emscripten + WebGPU

**What.** Ship `tensor` as a browser-side library via Emscripten + the existing WebGPU adapter. Users `import * as tensor from 'tensor-named-axis.wasm.js'` in a notebook or web demo.

**Strategic fit** 🟢 high IF the educational positioning extends to browser-based demos.

**Win likelihood** 🔴 low (substantial engineering: Emscripten + nanobind doesn't exist; would need a different binding layer; WebGPU-in-browser via Emscripten has its own pitfalls).

**Effort budget** 🔴 large (4-6 weeks at least). New binding layer + new build pipeline + new test surface.

**Output** — would be Phase 8+ — too speculative for the immediate next slice.

## 4. Recommended sequence — Phase 7 → `0.4.0`

A **3-phase sequence** that maximises strategic fit × win likelihood × bounded effort:

| Phase | Candidate | Effort | Output |
| ----- | --------- | ------ | ------ |
| **Phase 7a** | **Documentation-as-Product** (3a) + folded comparison study (3e) | 1-2 weeks | Citable PDF + NeurIPS workshop draft + landscape comparison report. Cashes ADR-0015 citability. |
| **Phase 7b** | **R-P6.5.5 lift** (3b) | 3-5 days | Phase 6.6 milestone; ADR-0020; type-owner module refactor; single-process multi-backend works. |
| **Phase 7c** | **`_tex` parser expansion** (3c) | 1-2 weeks | `\prod` / `\delta` / `\nabla` operators; the `\nabla`-as-autograd-trigger is the unique-in-the-landscape feature. |

**Exit version: `0.4.0`**. Two new ADRs (ADR-0020 multi-backend dispatch; ADR-0021 `_tex` parser expansion). One new impl-plan (`2026-MM-DD_phase-7-canonical-reference.md`). One new retrospective at Phase 7 close.

**Why this sequence**: Phase 7a packages the existing asset (docs are 95% there; productisation is the missing 5%). Phase 7b closes the only known real tech debt and is well-scoped. Phase 7c extends the project's most distinctive feature (the `_tex` UDL) into territory no competitor occupies. Sequence is roughly 4-6 weeks of dedicated effort, splittable across phases.

## 5. What this report DOES NOT recommend

- **Direct competition with Haliax / Penzai on production-scale named-tensor ML.** Already substantially served. Loss-position.
- **Adding JAX-style `vmap` / `pmap` / `jit` / `grad` transformations.** Substantial work, no distinctive payoff against the existing JAX ecosystem.
- **Switching binding tool from nanobind to pybind11.** `module_local` would solve R-P6.5.5 but contradicts ADR-0018; the type-owner-separation refactor in Phase 7b is a smaller move that doesn't fight the architecture.
- **Browser WASM build** in Phase 7. Substantial engineering with low win likelihood; reconsider as Phase 8 if the documentation push generates demand.
- **conda-forge feedstock now.** Already planned in [arc42 §7 §4d](../arc42/07-deployment/overview.md); ship after `0.3.0` PyPI publish.

## 6. Half-yearly recheck cadence

This report's strategic positioning re-validates against the next 6-monthly landscape audit due **2026-11-11** per [ADR-0017](../arc42/09-decisions/0017-clarify-reproducibility-envelope.md). Specifically re-check:

- Haliax / Penzai / Marin's named-axes API surface — does it converge towards or diverge from `tensor`'s vocabulary?
- nanobind's `module_local` (or analogous) feature status — would it lift Phase 6.6 / R-P6.5.5 work?
- `std::linalg` (P1673) — has it shipped in any vendor's STL yet? Triggers Phase 5 (`tensor::linalg` shim).
- WebGPU + Dawn stability + browser WebGPU adoption rate.
- New named-tensor papers / libraries at ICLR / NeurIPS / ICML 2026.

If the named-tensor JAX ecosystem retracts (Haliax/Penzai go unmaintained), the May-12 wedge re-opens and the strategic reframe in §2 above reverses. Track the indicators.

## 7. References

- [`./2026-05-12_landscape-recheck-and-adversarial-review.md`](./2026-05-12_landscape-recheck-and-adversarial-review.md) — the predecessor that this report refines on §A.5.
- [`./2026-05-10_tensor-revival-landscape.md`](./2026-05-10_tensor-revival-landscape.md) — the original Phase 0 landscape that motivated the revival.
- [`./2026-05-14_r-p6-5-5-lift-exploration.md`](./2026-05-14_r-p6-5-5-lift-exploration.md) — the companion exploration report Phase 7b feeds from.
- [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — the canonical-reference-quality positioning that Phase 7a operationalises.
- [Haliax repository](https://github.com/marin-community/haliax).
- [Levanter (now Marin) repository](https://github.com/stanford-crfm/levanter).
- [Penzai documentation](https://penzai.readthedocs.io/).
- [einx repository](https://github.com/fferflo/einx) + [ICLR 2026 paper](https://openreview.net/forum?id=QqvQ3iAdpC).
- [einops repository](https://github.com/arogozhnikov/einops) + [ICLR 2022 paper](https://openreview.net/pdf?id=oapKSVM2bcj).
- [PyTorch named tensors documentation](https://docs.pytorch.org/docs/2.12/named_tensor.html) — still experimental.
- [Named Tensor Notation (Chiang & Rush)](https://arxiv.org/abs/2102.13196) — the math-literature anchor.
- [tinygrad documentation](https://docs.tinygrad.org/) — adjacent educational-first minimal-ML library, not named-axis but a positioning reference.
