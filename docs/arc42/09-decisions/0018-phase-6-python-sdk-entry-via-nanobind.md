---
status: Accepted
owner: tensor
last-reviewed: 2026-05-12
---

# ADR-0018: Phase 6 — Python SDK via nanobind, in-tree under `python/`, prioritised early per maintainer 2026-05-12 directive

| Metadata  | Value                                                                  |
| --------- | ---------------------------------------------------------------------- |
| Status    | **Accepted**                                                           |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                                    |
| Date      | 2026-05-12                                                             |
| Deciders  | uyuutosa                                                               |
| Consulted | Claude (session of 2026-05-12); 2026-05-11 open-discussion-points report Axis H. |
| Informed  | future contributors                                                    |
| Ticket    | —                                                                      |
| Refines   | none (new phase entry).                                                |
| Triggered by | Maintainer 2026-05-12 message: "あと、pythonSDKははやくやりたい" (after the `0.1.0` cut). The 2026-05-11 [open discussion points report](../../reports/2026-05-11_open-discussion-points.md) §H captured the direction but parked the timing; this ADR brings it forward. |

---

## Context and Problem Statement

The `0.1.0` ship (2026-05-12; PRs #91 / #92, tag `0.1.0` on `main`, Pages live) closes Phase 4. The roadmap after that points at:

- **Phase 5** — `tensor::linalg` shim over kokkos/stdBLAS via `__cpp_lib_linalg` feature-test ([ADR-0014 §Decision Outcome point 4](0014-external-substrate-strategy.md)).
- **Phase 6** — Python SDK ([open discussion points report Axis H](../../reports/2026-05-11_open-discussion-points.md)), parked as "最後で良い" by the maintainer 2026-05-11.

On 2026-05-12 (post-cut) the maintainer signalled a priority shift: bring Phase 6 forward. The driver is **reach** — most ML/DL practitioners are in Python; a credible Python surface multiplies the project's audience for the canonical-reference aspiration of [ADR-0015](0015-aspire-to-canonical-reference-quality-not-self-anoint.md). Phase 5 is not a blocker because:

- `std::linalg` (P1673) is not shipped in any vendor STL as of 2026-05 (P1673R13 working draft; libc++ / libstdc++ / MSVC STL all unshipped). Phase 5 is *pre-empted*, not *blocked*.
- The Python SDK does not depend on `std::linalg`; it consumes the `DynamicTensor` + `KernelBackend` adapter surfaces that already ship.

The decisions this ADR captures:

1. **Binding library** — nanobind vs pybind11 vs Cython vs Python-side reimplementation.
2. **Repository layout** — in-tree `python/` subdirectory vs separate sibling repo.
3. **Build system** — scikit-build-core vs raw setuptools + CMake.
4. **PyPI publishing identity** — individual `uyuutosa` vs new organisation.
5. **conda-forge wheels** — yes / no.
6. **Initial public surface** — minimum set of types and operations Python sees first.

---

## Decision Drivers

- **DD-1**: Canonical-reference framing ([ADR-0015](0015-aspire-to-canonical-reference-quality-not-self-anoint.md)) demands clarity over performance. The binding layer should *read like* C++ on one side and *feel idiomatic* on the other. Bindings that obscure either side fail the readability discipline.
- **DD-2**: Solo bandwidth ([§11 R-M1](../11-risks/overview.md)). Whatever stack lands here, the maintainer alone keeps it green. Conservative defaults; minimal moving parts.
- **DD-3**: Educational-first positioning ([ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md)). The Python surface mirrors the C++ Domain, not a reshaped re-creation. Learners can read one and recognise the other.
- **DD-4**: Substrate sturdiness (the ADR-0014 / ADR-0016 lesson). Pick the tool that is **actively maintained** and **synchronised with its dependencies**; avoid the gpu.cpp scenario where a wrapper layer lags its substrate.
- **DD-5**: Reach. Python is where ML/DL practitioners are; a credible Python surface multiplies the canonical-reference aspiration's audience.
- **DD-6**: Reversibility. The binding library + layout decisions should not lock the project out of revisiting them; a clean Hexagonal-lite boundary keeps the Python adapter swappable like any other.

---

## Considered Options

The decision space splits cleanly into six axes (one per question above). For each axis the options are listed; the chosen option is named at the end with reasons.

### Axis A — Binding library

1. **nanobind** (chosen) — Wenzel Jakob (the original pybind11 author)'s "what pybind11 would look like if I designed it today" rewrite. ~4× faster compile, ~5× smaller binaries, ~10× lower runtime overhead; requires C++17 (we have C++20). Mature CMake + scikit-build-core integration; Stable ABI for Python 3.12+. Active maintenance (Wenzel + community).
2. **pybind11** — the genre standard. Wider tutorials / Stack Overflow surface area. Heavier compile / binary footprint. Older API.
3. **Cython** — different paradigm (Python-flavoured C compiled to C). Better for incremental Pythonisation of existing Python; worse fit for *exposing* a header-only C++ library.
4. **Python-side reimplementation** — rewrite the Domain in pure Python (numpy / jax). Loses the canonical-reference C++ artifact entirely; out of scope.

### Axis B — Repository layout

1. **In-tree `python/`** (chosen) — the Python SDK lives under `python/` in the main `tensor` repository. One `git log`, one issue tracker, the C++ and Python surfaces share an `0.x.y` version. Mirrors what the `lyx-export/` and `bench/` subdirectories already do.
2. **Separate sibling repo** — `tensor-python` as its own repo. Decouples versioning; doubles release ceremony and CI matrix.

### Axis C — Build system

1. **scikit-build-core** (chosen) — the modern CMake ↔ Python build glue; nanobind's documented + tested companion. Produces `pyproject.toml`-driven wheels via CMake. Mature in 2026; SciPy Proceedings article + reference impl.
2. **setuptools + CMake invocation** — older path. More glue code to maintain; nanobind's own examples have moved off it.
3. **meson-python** — alternative; less common for nanobind. Skip.

### Axis D — PyPI publishing identity

1. **Individual `uyuutosa`** (chosen) — matches the maintainer identity already on `CITATION.cff` and GitHub. Solo bandwidth (DD-2) makes a new organisation overhead without payoff.
2. **New organisation** (e.g. `tensor-cpp`) — would require setting up org PyPI credentials, maintainership policy. Postpone until external contribution makes it worthwhile.

### Axis E — conda-forge wheels

1. **Yes** (chosen) — pyproject.toml + scikit-build-core produces a wheel; submitting to conda-forge after PyPI publish is a separate small step. Conda-forge is the standard distribution channel for scientific-Python users (a meaningful slice of the project's audience).
2. **No** — PyPI only. Misses conda users; minor save in CI complexity.

### Axis F — Initial public surface (what Python sees first)

1. **Minimum-viable surface** (chosen):
   - `tensor.core.DynamicTensor` — the runtime-rank tensor. Static-rank `Tensor<T, N>` is hard to expose well to Python (Python is rank-erased); start with `DynamicTensor`.
   - `tensor.core.DynamicShape` + `tensor.core.Axis` — supporting types.
   - The four arithmetic operators (`+`, `-`, `*`, `/`) with Einstein-style broadcast over named axes. The headline feature.
   - `tensor.core.contract` — the named-axis contraction primitive.
   - `tensor.autograd.DynamicVariable` + `backward` + `gradient_check` — the autograd surface.
   - `tensor.tex.parse` + `tensor.tex.Evaluator` + the equivalent of the `_tex` UDL (in Python, an `r"..."`-string form).
   - `tensor.core.set_backend("reference" | "eigen" | "webgpu")` — Hexagonal-lite payoff visible to Python users (mirrors the C++ CMake variable).
   - NumPy interop via the buffer protocol so `np.asarray(t)` and `tensor.from_numpy(arr, labels=("i","j"))` work.
2. **Full mirror** — bind every public C++ symbol. Out of scope for M1; many compile-time-NTTP types (`TypedTensor<T, "i", "j">`, `LabelTag<S>`) do not have natural Python forms.
3. **Surface-by-surface staged** — same as option 1 but split across multiple PRs. (The chosen option is staged anyway; this is essentially option 1.)

---

## Decision Outcome

**Chosen: nanobind binding library, in-tree `python/` layout, scikit-build-core build, individual `uyuutosa` PyPI identity, yes conda-forge wheels, minimum-viable surface (Axis F option 1).**

### Y-statement summary

> In the context of **the maintainer's 2026-05-12 directive to bring the Python SDK forward in the queue (originally Phase 6 / Axis H / "最後で良い" per the 2026-05-11 open-discussion-points report)**, facing **the substrate / layout / publishing choices needed to scaffold a credible Python surface**, we decided for **nanobind + in-tree `python/` + scikit-build-core + individual PyPI identity + conda-forge + minimum-viable surface (`DynamicTensor` + autograd `DynamicVariable` + `tex.Evaluator` + backend selection)**, to achieve **a Python SDK that mirrors the C++ Domain readably and ships through 2026-mature tooling without locking the project out of revisiting choices later**, accepting **the cost of one new top-level subdirectory + one new CI matrix dimension + the inability to bind compile-time-NTTP `TypedTensor` / `LabelTag` directly (Python is rank-erased)**.

### Concrete next steps

A separate impl-plan ([`docs/impl-plans/2026-05-12_phase-6-python-sdk.md`](../../impl-plans/2026-05-12_phase-6-python-sdk.md)) lays out the milestones P6.M1 through P6.M6:

- **P6.M1** — scaffold: `python/`, `pyproject.toml`, scikit-build-core wired, nanobind discovered, "hello-tensor" smoke binding.
- **P6.M2** — bind `DynamicShape` + `Axis` + `DynamicTensor` with the four arithmetic operators and named-axis broadcast.
- **P6.M3** — bind `tensor::core::contract` + NumPy interop (buffer protocol + `from_numpy` / `to_numpy`).
- **P6.M4** — bind autograd: `DynamicVariable<T>`, `backward()`, `gradient_check`.
- **P6.M5** — bind `tensor::tex::parse` + `Evaluator<T>`; expose `tex_parse(R"(...)")` helper that mirrors the `_tex` UDL.
- **P6.M6** — `set_backend()` runtime selection; cross-validation test (Python-side calls match C++-side results).

Exit: `0.2.0` tag (Phase 6 close) — first PyPI publish + first conda-forge submission.

---

## Pros and Cons of the Options

(Per MADR §Pros and Cons — abbreviated; the full reasoning is in §Considered Options above.)

### Axis A — nanobind (chosen)

- **Pros**: 4× compile, 5× binary size, 10× runtime over pybind11; same author with 10 years of hindsight; first-class scikit-build-core; Stable ABI on 3.12+.
- **Cons**: smaller community than pybind11 (but the binding API is so similar that pybind11 examples translate directly); requires C++17 (we have C++20).

### Axis B — In-tree `python/` (chosen)

- **Pros**: one repo, one version, one issue tracker; precedent (`lyx-export/`, `bench/`).
- **Cons**: increases the repo's surface area for non-Python contributors who want to grep just C++.

### Axis C — scikit-build-core (chosen)

- **Pros**: modern + tested + nanobind's documented path; pyproject.toml driven.
- **Cons**: newer than legacy setuptools; one more tool in the chain.

### Axis D — Individual `uyuutosa` (chosen)

- **Pros**: zero extra setup; matches `CITATION.cff` + GitHub identity.
- **Cons**: one-person bus factor for the PyPI namespace. Mitigation: the org migration path stays open (PyPI supports transferring projects between users / orgs).

### Axis E — Yes conda-forge (chosen)

- **Pros**: reaches the scientific-Python audience PyPI alone misses.
- **Cons**: one extra release step; conda-forge maintenance is a separate flavour of release ceremony.

### Axis F — Minimum-viable surface (chosen)

- **Pros**: ships the headline functionality (named-axis algebra + autograd + `_tex` surface); avoids the hard problem of compile-time-NTTP types in a rank-erased target language.
- **Cons**: Python users cannot get compile-time mismatched-axis errors. The Python form gets runtime errors (the same as the C++ `DynamicTensor` path).

---

## Consequences

### Positive

- Python users access the headline named-axis algebra + autograd + `_tex` surface — the project's reach multiplies materially.
- The binding layer is small (nanobind keeps the C++ side recognisable; the Python side stays idiomatic).
- Phase 5 is decoupled — it can ship whenever `<linalg>` is mature, without blocking Phase 6.

### Negative

- One more dimension in the CI matrix (Linux / macOS / Windows × Python 3.9 / 3.10 / 3.11 / 3.12 / 3.13 wheels).
- Maintainer bandwidth ([R-M1](../11-risks/overview.md)) is now spread across C++ + Python + book + bibliography audit.
- Compile-time-NTTP types (`TypedTensor<T, "i", "j">` / `LabelTag<S>`) cannot be exposed to Python; that part of the C++ surface is C++-only.

### Neutral

- The C++ Domain is unchanged. The Python SDK is a DrivingAdapter ([ADR-0009](0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md)) consuming the same `KernelBackend` port the C++ tests use.
- ADR-0015's three disciplines (bibliography / ubiquitous-language / reproducibility) extend to the Python surface naturally — `tensor.core.DynamicTensor` is the same vocabulary as the C++ `tensor::core::DynamicTensor`.

### Follow-ups

- [ ] Write impl-plan [`docs/impl-plans/2026-05-12_phase-6-python-sdk.md`](../../impl-plans/2026-05-12_phase-6-python-sdk.md) with the 6 milestones above. (Bundled in the same PR as this ADR per [WORKFLOW.md §2](../../WORKFLOW.md).)
- [ ] P6.M1 scaffolding PR: `python/`, `pyproject.toml`, `python/CMakeLists.txt`, nanobind smoke. Separate PR.
- [ ] arc42 §5 building-blocks: add `python/` row classified as DrivingAdapter.
- [ ] C4 `workspace.dsl`: add a `pythonSdk` container alongside `tensor::tex` and `lyx-export`.
- [ ] CI matrix expansion for wheel builds (Phase 6 close).
- [ ] PyPI / conda-forge publishing setup (Phase 6 close — exit gate for `0.2.0`).

---

## Compliance / Validation

- **Verification**: P6.M6 (the final milestone) is a cross-validation test that Python-side and C++-side calls produce identical results for the four-arithmetic-broadcast / `contract` / autograd `backward` / `tex.Evaluator` paths. This is the discipline that keeps the Python surface from drifting from the C++ Domain.
- **Frequency**: per-PR within Phase 6; per-release after Phase 6 closes.

---

## More Information

- Open discussion points report Axis H (the original directional capture): [`docs/reports/2026-05-11_open-discussion-points.md`](../../reports/2026-05-11_open-discussion-points.md).
- nanobind documentation: <https://nanobind.readthedocs.io/>.
- scikit-build-core: <https://scikit-build-core.readthedocs.io/>.
- 2026 landscape: [`docs/reports/2026-05-12_landscape-recheck-and-adversarial-review.md`](../../reports/2026-05-12_landscape-recheck-and-adversarial-review.md) §A.5 (PyTorch named-tensor still experimental; JAX/jaxtyping standard for axis annotation; **no first-class named-axis tensor exists in Python ML** — this gap is exactly the wedge this ADR opens).
- Related: [ADR-0010](0010-refine-positioning-to-educational-first-production-capable.md) (production-capable via adapters), [ADR-0011](0011-kernel-backend-port-api.md) (`KernelBackend` port), [ADR-0015](0015-aspire-to-canonical-reference-quality-not-self-anoint.md) (canonical-reference framing).
