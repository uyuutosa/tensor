---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Named-tensor library comparison study (2026-05-14)

| Metadata        | Value                                                                                  |
| --------------- | -------------------------------------------------------------------------------------- |
| Status          | Stable                                                                                 |
| Type            | Layer B — landscape comparison study (Phase 7a M1 deliverable)                         |
| Owner           | uyuutosa                                                                               |
| Trigger         | Phase 7a milestone M1 per [`../impl-plans/2026-05-14_phase-7a-docs-as-product.md`](../impl-plans/2026-05-14_phase-7a-docs-as-product.md) |
| Anchor ADR      | [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md) — strategic narrowing |
| Successor       | Six-monthly recheck at 2026-11-11 per [ADR-0017](../arc42/09-decisions/0017-clarify-reproducibility-envelope.md) |
| Snapshot date   | 2026-05-14                                                                             |

## TL;DR

Nine libraries surveyed across eleven evaluation axes. **`tensor` occupies an unoccupied position** in the matrix: C++20-native + first-class named axes + compile-time-checked axes (NTTP) + LaTeX-subset UDL + canonical-reference-quality docs. Every competitor matches at most three of those five attributes. Haliax (now in Marin) is the closest neighbour but is JAX-bound and lacks the compile-time-checked + UDL surfaces. This study evidences the strategic-narrowing claim of ADR-0021: invest in the moats, decline the production-scale chase.

---

## 1. Methodology

### 1.1 Libraries surveyed

Nine libraries, picked for landscape coverage rather than direct competition:

| Library                  | Why included                                                                                  |
| ------------------------ | --------------------------------------------------------------------------------------------- |
| **`tensor`** (this project) | The subject of the comparison.                                                              |
| **xarray**               | Oldest named-axis library (2014). Scientific-computing reference point.                       |
| **PyTorch named tensors** | Highest-visibility named-axis attempt in a major framework. Reference for "what didn't work". |
| **einops**               | The Einstein-notation-on-host-frameworks reference (ICLR 2022).                              |
| **einx**                 | The next-generation einops (ICLR 2026 oral). Notation layer state of the art.                |
| **Haliax**               | JAX named-axes library at production scale (powers Marin/Levanter 70B+ LLM training).        |
| **Penzai**               | Google DeepMind's JAX named-axes library — notebook-ergonomic.                               |
| **Nx**                   | Elixir's named-tensor library. Off-mainstream reference for "different language ecosystem".  |
| **tinygrad**             | Educational-first minimal-ML library. Positioning reference (not named-axis but adjacent).   |

### 1.2 Axes of comparison

Eleven evaluation axes, chosen to surface the architectural + positioning + docs dimensions that matter for the strategic-narrowing question:

1. **Host language** — what the user code is written in.
2. **Named-axis semantics** — first-class (always-named) / optional (positional with named overlay) / notation-only (translates to positional under the hood) / none.
3. **Compile-time axis checking** — does the type system catch axis-name mismatch at compile / type-check time, or only at runtime?
4. **Differentiability** — does the library provide autograd? Through what mechanism?
5. **GPU / accelerator backend** — what hardware path is available, and through what compiler?
6. **DSL / notation layer** — what non-API surface does the library offer for expressing tensor computations?
7. **Documentation discipline** — what depth of documentation system is offered?
8. **Educational target** — explicit teaching-oriented framing yes/no?
9. **Distribution** — header-only / wheel / package-manager / source-only?
10. **Production scale** — proven at what scale? (token count or parameter count for ML; dataset size for data libraries).
11. **License + governance** — license + project size (solo / org / foundation).

### 1.3 What this study is NOT

- **Not a performance benchmark**. Performance comparisons depend on hardware + workload; out of scope for a landscape study.
- **Not a feature-completeness audit** of any individual library. Each library has dozens of operators / utilities; the matrix records *architectural posture*, not API coverage.
- **Not a recommendation engine**. The study describes the landscape; the strategic response is in [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md).

---

## 2. Comparison matrix

The matrix is split into three tables for readability: language + semantics + safety; differentiability + GPU + DSL; docs + target + distribution + scale + license.

### 2.1 Language, named-axis semantics, compile-time safety

| Library         | Host language     | Named-axis semantics              | Compile-time axis checking |
| --------------- | ----------------- | --------------------------------- | -------------------------- |
| **`tensor`**    | C++20             | First-class (always-named) + DynamicShape | **Yes** — NTTP `LabelTag<"i">` |
| xarray          | Python (NumPy)    | First-class (`DataArray`)         | No (Python runtime)        |
| PyTorch named tensors | Python (C++ backend) | Optional (positional with named overlay) | No (prototype) |
| einops          | Python (NumPy/PyTorch/JAX/TF/MLX) | Notation-only (dim strings) | No (runtime regex)         |
| einx            | Python (NumPy/PyTorch/JAX/TF/MLX) | Notation-only (`[]` brackets + ellipsis) | No (runtime parser)        |
| Haliax          | JAX (Python)      | First-class (`NamedArray`)        | Partial (JAX trace-time)   |
| Penzai          | JAX (Python)      | First-class (`NamedArray` PyTree) | Partial (JAX trace-time)   |
| Nx              | Elixir            | First-class                       | Partial (dialyzer)         |
| tinygrad        | Python            | None (positional)                 | No                         |

**Read**: only `tensor` has both first-class named axes AND compile-time axis-name checking via C++20 NTTP. Haliax/Penzai's "partial" compile-time checking is the JAX trace-shape check that happens at first `jit` call, which is meaningfully later in the dev loop than C++ compile.

### 2.2 Differentiability, GPU backend, DSL / notation layer

| Library         | Autograd        | GPU / accelerator path                | DSL / notation layer                |
| --------------- | --------------- | ------------------------------------- | ----------------------------------- |
| **`tensor`**    | Tape-based reverse-mode (own) | Reference / Eigen / **WebGPU** (Dawn) | **`_tex` UDL** — LaTeX subset parsed at compile time |
| xarray          | No              | Dask (CPU/GPU shards)                 | Coordinate-based indexing           |
| PyTorch named tensors | PyTorch autograd | CUDA / MPS / ROCm / WebGPU (experimental) | Positional + name kwargs            |
| einops          | Inherits host   | Inherits host                         | Dim-string mini-DSL (`"b h w c -> b c h w"`) |
| einx            | Inherits host   | Inherits host                         | Bracket-DSL (`"b [h w] c"`) + ellipsis |
| Haliax          | JAX autograd    | XLA (CPU/GPU/TPU)                     | Axis-name kwargs                    |
| Penzai          | JAX autograd    | XLA (CPU/GPU/TPU)                     | Axis-name kwargs + PyTree           |
| Nx              | Own autograd    | EXLA (XLA via Erlang)                 | Elixir macros for tensor ops        |
| tinygrad        | Own autograd    | Many (CUDA/Metal/AMD/WebGPU/Clang)    | None (12 RISC-like primitive ops)   |

**Read**: only `tensor` and tinygrad have **own autograd implementations** (everyone else inherits from the host framework). Only `tensor` has a **LaTeX-subset UDL** that parses tensor formulas at compile time. tinygrad's "DSL" is its 12-primitive-op vocabulary, which is a different framing — it has no notation layer in the formal-language sense.

### 2.3 Documentation, target audience, distribution, scale, license

| Library         | Documentation discipline                 | Educational target | Distribution                | Production scale            | License / governance        |
| --------------- | ---------------------------------------- | ------------------ | --------------------------- | --------------------------- | --------------------------- |
| **`tensor`**    | arc42 + Diátaxis + 21 ADRs + 8 detailed-design + Diátaxis user-manual | **Explicit yes** (educational-first per ADR-0010) | Header-only C++ + Python wheel (extras) | N/A (educational + research focus) | MIT (solo: uyuutosa) |
| xarray          | Sphinx + extensive narrative tutorials   | No (production for sci-comp) | pip / conda                 | TB-scale climate datasets   | Apache 2.0 (NumFOCUS)       |
| PyTorch named tensors | PyTorch docs section                | No (production)    | pip (part of PyTorch)       | Experimental — production-scale not advertised | BSD (Meta / PyTorch Foundation) |
| einops          | README + ICLR paper                      | Implicit (notation paper) | pip                         | Used in many published papers | MIT (individual: arogozhnikov) |
| einx            | README + ICLR 2026 oral paper            | Implicit           | pip                         | Recent (ICLR 2026)          | MIT (individual: fferflo)   |
| Haliax          | README + Marin docs                      | No (production)    | pip                         | **70B+ params on TPU v4-2048** | Apache 2.0 (Stanford CRFM → Marin community) |
| Penzai          | ReadTheDocs                              | Implicit (notebook ergonomics) | pip                         | LLM-scale demos             | Apache 2.0 (Google DeepMind) |
| Nx              | hexdocs                                  | No (production for Elixir ML) | hex (Elixir package manager) | Niche                       | Apache 2.0 (Erlef community) |
| tinygrad        | README + own docs site                   | **Explicit yes**   | pip / from source           | Production demos (LLama 3 inference, etc.) | MIT (community: tinycorp)   |

**Read**: only `tensor` and tinygrad have **explicit educational-target framing**. Only `tensor` ships **arc42 + Diátaxis + ADR-grade documentation system** — all other libraries have READMEs + API docs but no architectural-decision archive. `tensor`'s production-scale story is N/A (and intentionally so per ADR-0021); Haliax dominates the production-scale named-axis story.

---

## 3. Library-by-library notes

### 3.1 `tensor` (this project)

C++20 header-only with optional Python wheel via nanobind. Hybrid named-axis API: compile-time `LabelTag<"i">` via NTTPs ([ADR-0004](../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md)) plus runtime `DynamicShape`. `_tex` UDL parses a LaTeX subset at compile time ([ADR-0005](../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md)) so `R"(c_{ij} = a_{ik} b_{kj})"_tex` becomes a parsed AST without runtime overhead. Hexagonal-lite + `KernelBackend` port ([ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md)) gives three swappable adapters (reference, Eigen, WebGPU/Dawn). Tape-based reverse-mode autograd ([ADR-0007](../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md)). Documentation: 21 ADRs as of 2026-05-14, 8 detailed-design instances, ~125 markdown files. **Solo-maintainer; no production-scale claim**.

### 3.2 xarray

The grandfather of named-axis tensors in Python. Pure-Python on NumPy. No autograd, no native GPU path (Dask provides distributed but not differentiable). Strong scientific-computing adoption (climate, oceanography, geosciences). The `DataArray` and `Dataset` containers carry coordinate metadata + units. Documentation is Sphinx-based with extensive narrative tutorials. **NumFOCUS-supported, mature**.

### 3.3 PyTorch named tensors

Introduced 2019 (PyTorch 1.3); **still "experimental and prototype" as of PyTorch 2.12 (2026-05)**. The framework provides `tensor.refine_names("N", "C", "H", "W")` and named-aware operations, but many PyTorch operators do not propagate names correctly. The named-tensor track in PyTorch has effectively stalled — there are no recent design RFCs and no production-scale users advertise their use. Reference for "what happens when named axes are bolted onto a positional framework rather than designed in".

### 3.4 einops

The Einstein-notation reference. einops translates string DSL like `"b h w c -> b c h w"` to host-framework calls on NumPy/PyTorch/JAX/TF/MLX. Pure notation layer — no own tensor type, no own autograd. **ICLR 2022 paper** legitimised the notation across the ML community. Roughly the lingua franca for axis reorderings in ML papers since 2022.

### 3.5 einx

Successor / generalisation of einops. Adds `[]` brackets for sub-axis grouping and `...` ellipsis. **ICLR 2026 oral paper** as of the snapshot date. Same architectural posture as einops (notation layer over host frameworks); broader expressiveness. Increasingly cited in 2026 ML papers.

### 3.6 Haliax

Stanford CRFM's JAX-based named-tensor library. Powers Levanter (CRFM's LLM training framework) which trained 70B-param models on TPU v4-2048. **In November 2025, Levanter merged into the Marin monorepo** (Stanford CRFM's broader foundation-model effort); Haliax is actively developed under that umbrella. First-class `NamedArray` integrates with JAX `jit` / `pjit` / `vmap`. FSDP + tensor parallelism via JAX's mesh / sharding primitives. **The closest neighbour to `tensor` in posture, separated by host language + production-scale story**.

### 3.7 Penzai

Google DeepMind's JAX-based named-tensor library. `NamedArray` PyTree integration; explicit "notebook ergonomics first" framing. Active development; smaller than Haliax in production-scale adoption but with stronger notebook-friendly visualisation utilities. Educational-implicit: documentation features many "look inside the array" tutorials.

### 3.8 Nx

Elixir's tensor library. First-class named axes via Elixir's pattern-matching idioms. EXLA backend bridges to XLA via Erlang ports. **Niche audience** — Elixir ML is small but active (Bumblebee, Axon, Explorer). Reference for "what named-axis tensors look like in a non-Python, non-C++ ecosystem".

### 3.9 tinygrad

Minimal Python ML framework (~1000s of LOC). 12 primitive ops; deeper operations built from those primitives. **Explicitly educational** ("learn ML by reading the source"). No named axes — fully positional. Many GPU backends (CUDA, Metal, AMD, WebGPU, Clang). LLama 3 inference demos run on tinygrad. **Positioning reference**: educational-first works at scale (tinygrad has a real community), even without named axes.

---

## 4. What `tensor` does that nobody else does

Three claims, each mapped to a specific ADR / detailed-design:

### 4.1 Hybrid compile-time + runtime named axes ([ADR-0004](../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md))

**Claim**: `tensor` is the only library where the same code can ask the C++ type system to enforce `i ≠ j` at compile time (via NTTP `LabelTag<"i">`) and use the runtime `DynamicShape` when axis names come from external input. Haliax / Penzai have JAX trace-shape checking which happens at first `jit` call — *meaningfully later* than C++ compile. xarray / PyTorch / einops / einx have no static check at all.

**Why it matters**: compile-time axis-name checking catches "you transposed `i` and `j`" before the program runs, without runtime overhead. The hybrid story means users can use the compile-time path for known-axis kernels (matmul, contract) and the runtime path for axis-name-from-config-file code.

### 4.2 `_tex` UDL — the formula is the program ([ADR-0005](../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md))

**Claim**: `tensor` is the only library where a LaTeX subset compiles to a tensor expression AST at C++ compile time:

```cpp
auto expr = R"(c_{ij} = a_{ik} b_{kj})"_tex;
// `expr` is a parsed AST; no runtime parsing cost.
```

einops + einx have *string DSLs* but they're parsed at runtime via Python regex / parser combinators — not at C++ compile time. xarray + Haliax + Penzai + Nx use Python kwargs (`x.sum("i")`) or method chains — no notation parser at all.

**Why it matters**: the LaTeX subset matches the notation papers actually use (Named Tensor Notation, Chiang & Rush 2021); the C++ compile-time parse means the notation is part of the program, not a string handed to a runtime interpreter. The UDL surface is uniquely available in C++ template-meta-programming.

### 4.3 Canonical-reference-quality documentation as a deliverable ([ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) / [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md))

**Claim**: `tensor` is the only library treating its documentation system as a citable artifact in its own right. Twenty-one ADRs + eight detailed-design instances + full Diátaxis user manual + ~125 markdown files is qualitatively a different document corpus from "README + API docs", which is what every other library has. Phase 7a (this plan) productises the corpus into a citable PDF + workshop paper.

**Why it matters**: future implementers porting the named-axis substrate to another language (Rust, Zig, Mojo) need the decision rationale, not just the API. Textbook authors need the bibliography discipline. The documentation system is the strategic moat that compounds across Phase 7+.

---

## 5. What others do that `tensor` doesn't (and why we decline)

### 5.1 Production-scale ML training (Haliax)

Haliax demonstrates 70B-parameter LLM training on TPU v4-2048 with FSDP + tensor parallelism + activation checkpointing. **`tensor` declines this entire surface** per ADR-0021. The solo-maintainer model cannot sustain distributed-training engineering; users with production-scale needs are correctly redirected to Haliax / Penzai / Marin.

### 5.2 PyPI-ecosystem network effects (xarray, Penzai, einops)

xarray is `pip install xarray` and immediately interoperates with NumPy, Dask, matplotlib, scikit-learn. einops works with any of NumPy / PyTorch / JAX / TF / MLX. **`tensor` is one ecosystem entry behind** by virtue of being C++-first with a Python wrapper. Phase 6 / 6.5 / 6.6 close this gap incrementally; Phase 7a productises the docs around the C++ moats rather than chasing further Python-ecosystem integration.

### 5.3 JAX `vmap` / `pmap` / `jit` / `grad` transformations (Haliax, Penzai)

JAX's function transformations are an architectural feature of JAX, not of named-tensor libraries built on it. `tensor`'s tape-based reverse-mode autograd is a *different* design point — simpler, slower, more obvious. **Declined for the educational frame**: tape-based autograd is easier to teach.

### 5.4 String-DSL adoption (einops, einx)

einops's `"b h w c -> b c h w"` notation is the lingua franca for axis manipulation in 2026 ML papers. **`tensor` declines this surface** in favour of `_tex` UDL — LaTeX-subset matches the math-paper notation, not the einops community notation. The trade-off is intentional: `tensor` optimises for textbook-author / math-literate audience, not ML-paper-author audience.

---

## 6. Where the moats interact

Two interactions worth calling out:

### 6.1 `_tex` UDL × NTTP = compile-time-checked differentiable LaTeX

`R"(\nabla_x c_{ij} = ...)"_tex` (when Phase 7c lands the `\nabla` operator) will produce a compile-time-checked tensor expression where (a) axis names are validated by NTTP, (b) the gradient direction (`\nabla_x`) routes through the autograd tape, (c) the differentiation target's existence is verified at compile time. **No competitor offers this** — Haliax checks shapes at trace time but the differentiation target is a Python variable; einops/einx are notation-only with no autograd; xarray has no autograd.

### 6.2 KernelBackend port × Hexagonal-lite × bundled tutorials = teaching surface

Three swappable adapters (reference / Eigen / WebGPU) sharing one `KernelBackend` interface, with the `tutorials/08_swappable-backends.ipynb` walking through the architectural choice — this is a worked Hexagonal-lite example. **No competitor exposes the architecture as the teaching artifact**; Haliax/Penzai's XLA compiler path is opaque to the user; xarray's Dask integration is hidden behind `chunk()`. `tensor`'s framing turns the architecture itself into a tutorial subject.

---

## 7. Snapshot validity

This study reflects the state of the named-tensor landscape **as of 2026-05-14**. Per [ADR-0017](../arc42/09-decisions/0017-clarify-reproducibility-envelope.md), the next half-yearly landscape recheck is due **2026-11-11**. The strategic-narrowing claim of ADR-0021 rests on two ongoing conditions:

1. Haliax / Penzai continue serving the production-scale Python-ML named-axes wedge.
2. The C++-native + `_tex` UDL + citation-grade docs moats remain uncontested.

If either condition fails at the 2026-11-11 recheck (Haliax/Penzai unmaintained → wedge reopens; a competitor ships C++-native named axes → moat closes), this study and ADR-0021 need supersession review.

---

## 8. References

### Project documents

- [ADR-0001](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) — original educational-first pivot.
- [ADR-0004](../arc42/09-decisions/0004-adopt-hybrid-named-axis-api.md) — hybrid named-axis API.
- [ADR-0005](../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md) — `_tex` UDL.
- [ADR-0007](../arc42/09-decisions/0007-adopt-autograd-as-first-class-subsystem.md) — autograd.
- [ADR-0011](../arc42/09-decisions/0011-kernel-backend-port-api.md) — KernelBackend port.
- [ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md) — canonical-reference reframe.
- [ADR-0015](../arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md) — quality-aspiration discipline.
- [ADR-0017](../arc42/09-decisions/0017-clarify-reproducibility-envelope.md) — half-yearly recheck cadence.
- [ADR-0021](../arc42/09-decisions/0021-strategic-narrowing-to-specialist-reference-positioning.md) — strategic narrowing (this study is the supporting evidence).
- [`2026-05-14_landscape-recheck-and-phase-7-roadmap.md`](./2026-05-14_landscape-recheck-and-phase-7-roadmap.md) — Phase 7+ roadmap.
- [`2026-05-12_landscape-recheck-and-adversarial-review.md`](./2026-05-12_landscape-recheck-and-adversarial-review.md) — predecessor landscape audit.

### External libraries

- xarray: <https://docs.xarray.dev/> ; <https://github.com/pydata/xarray>.
- PyTorch named tensors: <https://docs.pytorch.org/docs/2.12/named_tensor.html> (still labelled experimental and prototype as of PyTorch 2.12).
- einops: <https://github.com/arogozhnikov/einops> ; Rogozhnikov, ICLR 2022, <https://openreview.net/pdf?id=oapKSVM2bcj>.
- einx: <https://github.com/fferflo/einx> ; ICLR 2026 oral, <https://openreview.net/forum?id=QqvQ3iAdpC>.
- Haliax: <https://github.com/marin-community/haliax>. Levanter (now part of Marin): <https://github.com/stanford-crfm/levanter>.
- Penzai: <https://penzai.readthedocs.io/> ; <https://github.com/google-deepmind/penzai>.
- Nx: <https://hex.pm/packages/nx> ; <https://github.com/elixir-nx/nx>.
- tinygrad: <https://github.com/tinygrad/tinygrad> ; <https://docs.tinygrad.org/>.

### Math + notation references

- Chiang, D. & Rush, A. M. — Named Tensor Notation, 2021, <https://arxiv.org/abs/2102.13196>. The math-literature anchor for named-axis notation.
- Marin community foundation-model effort (Stanford CRFM): <https://github.com/marin-community>.
