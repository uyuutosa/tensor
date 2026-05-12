---
status: Active
owner: tensor
last-reviewed: 2026-05-10
---

# Phase 1 — `tensor` revival: rewrite skeleton up to `0.0.1-alpha`

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Active                                                         |
| Type          | Layer B — implementation plan (dated, append-only)             |
| Owner         | uyuutosa                                                       |
| Related       | ADR-0001..0008; `docs/arc42/01-introduction-and-goals/overview.md` |
| Target window | 2026-05-10 → 2026-06-30 (≈ 7 weeks)                            |
| Exit version  | `0.0.1-alpha` (internal alpha; not published)                  |

## TL;DR

Phase 1 establishes a runnable C++20 skeleton — modern build, core named-axis types, the `_ax` and `_tex` UDLs at MVP scope, and the first tutorial notebook reproducing the 2016 Qiita convolutions-as-inner-products post. **Autograd, WebGPU, and the full tutorial set are deferred to Phase 2 and Phase 3.** The point of Phase 1 is to make `0.0.1-alpha` *real*: a learner can `git clone && cmake --build && jupyter lab tutorials/00_intro.ipynb` and see the project's headline pitch work end-to-end on the CPU path.

## Phasing strategy

| Phase | Window               | Exit version     | Primary deliverable                                             |
| ----- | -------------------- | ---------------- | --------------------------------------------------------------- |
| **1** | 2026-05-10 → 06-30   | `0.0.1-alpha`    | CPU skeleton + `_ax` / `_tex` MVP + intro notebook              |
| 2     | 2026-07-01 → 08-31   | `0.0.2-alpha`    | Tape-based autograd + MLP-on-toy-data tutorial                  |
| 3     | 2026-09-01 → 10-31   | `0.0.3-alpha`    | WebGPU backend + browser/Notebook GPU demo                      |
| 4     | 2026-11 → 12         | `0.1.0`          | Full tutorial corpus, Jupyter Book published, public announce   |

This document covers Phase 1 only. Subsequent phases will be planned in their own dated impl-plan files when Phase 1 closes.

## Milestone breakdown

Each milestone targets ~1 week of evening / weekend work for a solo maintainer; slip is expected.

### M1 — Build system reset (week 1–2)

> **Realises**: ADR-0003 (CMake + vcpkg).

- [ ] Add `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` at repo root.
- [ ] Move `.cproject`, `.project`, `.settings/` into `archive/eclipse-cdt/` (preserve git history; do not delete).
- [ ] vcpkg manifest pins the [Kokkos `mdspan`](https://github.com/kokkos/mdspan) reference impl, [doctest](https://github.com/doctest/doctest), and [fmt](https://github.com/fmtlib/fmt) (for diagnostic formatting until C++20 `std::format` is universally available).
- [ ] GitHub Actions workflow `ci.yml`: matrix {GCC 11, Clang 13, MSVC 19.30} × {Debug, Release}, runs `cmake --preset=default && ctest --preset=default`.
- [ ] `clang-format`, `clang-tidy`, `cmake-format` configurations committed.
- [ ] `README.md` build section rewritten with the two-command quickstart.

**Exit criterion**: green CI on a "hello world" `int main() { return 0; }` translation unit that links the (still empty) `tensor` interface library.

### M2 — Core types (week 2–3)

> **Realises**: ADR-0002 (C++20 + mdspan), partial ADR-0004 (axis identity).

- [ ] `tensor::Axis` — pair of `(label, extent)`, label as small-string-optimized string-view-like.
- [ ] `tensor::Shape<N>` — fixed-rank shape, ordered list of `Axis`. `consteval` shape arithmetic helpers.
- [ ] `tensor::Tensor<T, N>` — header-only, contiguous, owning. Backed by `std::vector<T>` + `Shape<N>`.
- [ ] `tensor::mdview(t)` — returns a `std::mdspan` (or polyfill) over the buffer. Round-trip: `Tensor<T,N>::from_mdspan(...)`.
- [ ] `tensor::print(t)` / `operator<<(ostream&, Tensor const&)` — preserves the visual formatting of the 2016 README's output blocks (axis labels in headers).
- [ ] doctest test suite: shape arithmetic, mdspan round-trip, basic indexing, formatting.

**Exit criterion**: a learner can construct `Tensor<double, 1> a({Axis{"i", 5}}, {1,2,3,4,5})`, print it, and round-trip it through `std::mdspan` losslessly.

### M3 — Hybrid named-axis core ops (week 3–4)

> **Realises**: rest of ADR-0004.

- [ ] `tensor::LabelTag<...>` — NTTP string-literal-encoded compile-time axis identity.
- [ ] `_ax` UDL: `"i"_ax` returns `LabelTag<"i">`.
- [ ] Element-wise binary ops `(+, -, *, /)` with **runtime** broadcasting on `Axis` labels: `a("i") + b("j") → c("i","j")`.
- [ ] Element-wise binary ops with **compile-time** broadcasting on `LabelTag`: `a("i"_ax) + b("j"_ax)` produces a tensor with statically known axis labels and a `static_assert`-quality error on mismatch.
- [ ] Both paths share a single underlying kernel template; no semantic divergence.
- [ ] Property tests: for every (op, shape pair), runtime path and compile-time path produce identical buffers.

**Exit criterion**: the README's four-arithmetic-operations example block runs unmodified against the new API.

### M4 — `_tex` MVP parser (week 4–5)

> **Realises**: partial ADR-0005 (TeX/LyX authoring surface).

- [ ] `consteval` recursive-descent parser for a tightly scoped LaTeX subset:
  - Subscripted variable: `a_i`, `a_{ij}`
  - Sum: `\sum_i`, `\sum_{ij}`
  - Indexed arithmetic: `a_i + b_j`, `a_i b_i` (implicit contraction over repeated index)
  - Parens, `=`, `\cdot`
- [ ] `_tex` UDL: `R"(c_{ij} = a_i b_j)"_tex` → expression graph using `LabelTag<...>`.
- [ ] Runtime variant `tensor::tex(string)` sharing the same grammar.
- [ ] Output rendering: `tensor::to_latex(expr)` — round-trip property test (`parse(render(e)) == e`) on a corpus of 20+ expressions.
- [ ] *Out of scope this milestone*: LyX module, fractions, full LaTeX, error-message polish beyond minimum.

**Exit criterion**: a notebook cell that contains `auto c = R"(c_{ij} = a_i + b_j)"_tex;` produces the same tensor as the equivalent C++ expression and renders back to the original LaTeX in the cell output.

### M5 — Function tensor + reference tensor (week 5–6)

> **Realises**: ADR-0001 (preserves the original library's pedagogical exhibits).

- [ ] `tensor::FunctionTensor<F>` — element type is a callable `F`. Multiplication applies `F` element-wise.
- [ ] `tensor::ReferenceTensor<T>` — element `n+1` references element `n` along a chosen axis; multiplying recomputes the recurrence.
- [ ] Modernized API: no `new`/`delete`; uses `std::function` or templated callables.
- [ ] doctest tests covering the two examples from the 2016 README (function tensor and reference tensor).
- [ ] Both surface in the intro notebook (M6) as teaching exhibits.

**Exit criterion**: the reference-tensor `r * 3 → (9, 27, 81, 243, 729)` example from the original README runs against the new API.

### M6 — Tutorial `00_intro.ipynb` (week 6–7)

> **Realises**: partial ADR-0008 (Jupyter delivery).

- [ ] [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) kernel install instructions documented.
- [ ] `tutorials/00_intro.ipynb` modernizes the [2016 Qiita post](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74) using the new API:
  - Section 1: define `a_i`, `b_j`, four-arithmetic table.
  - Section 2: function tensor `f`, multi-application `a * f * b * f * a * f`.
  - Section 3: reference tensor `r`, recurrence interpretation.
  - Section 4: convolutions reformulated as tensor inner products (the centerpiece).
  - Section 5: TeX bridge — write the same expressions in `_tex` and show LaTeX output cells.
- [ ] `nbconvert` smoke test in CI executes the notebook end-to-end and asserts no errors.
- [ ] `tutorials/README.md` lists the planned notebook progression (forward-looking; only `00_intro` exists).

**Exit criterion**: `git clone && jupyter lab tutorials/00_intro.ipynb` works on a clean Ubuntu 24.04, macOS 14, and Windows 11 with WSL; CI runs the same notebook headlessly per PR.

## Cross-cutting concerns

- **Documentation per code change.** Per `.claude/rules/documentation.md`, every PR that lands code in this phase updates either `arc42/05-building-blocks/` or `detailed-design/`. The `doc-orchestrator` agent enforces this.
- **Branching.** Each milestone is one or more `feature/<kebab>` branches off `develop`, merged via PR with merge commits (Git Flow + the no-squash-on-subtree rule from `.claude/rules/version-control.md`).
- **Versioning.** Phase 1 produces internal alphas only; no GitHub release tags until `0.1.0`. CMake project version is bumped per milestone (`0.0.1-alpha.M1`, `0.0.1-alpha.M2`, …).
- **Out of Phase 1**: autograd (Phase 2), WebGPU (Phase 3), LyX export (Phase 3 / 4), full Jupyter Book deployment (Phase 4), `std::linalg` interop (Phase 4 or as toolchains stabilize).

## Risks and mitigations

| Risk                                                          | Likelihood | Impact | Mitigation                                                                  |
| ------------------------------------------------------------- | ---------- | ------ | --------------------------------------------------------------------------- |
| `consteval` parser error messages are brittle (M4)            | High       | Medium | Allocate explicit time in M4 for `static_assert` message wording; defer cosmetic polish to Phase 4. |
| xeus-cling lags C++20 features (M6)                           | Medium     | Medium | Pin a known-good xeus-cling version in `tutorials/environment.yml`; fall back to clang +`__cpp_consteval` checks. |
| vcpkg port for `mdspan` polyfill diverges from upstream       | Low        | Low    | Vendor a known-good `mdspan` commit via vcpkg overlay if needed.            |
| Solo bandwidth slips (chronic)                                | High       | Low    | Treat per-milestone exit criteria as the contract; let the calendar slip rather than the criteria. |

## Cut lines (what to drop if behind schedule)

In priority order — drop top first:

1. M5 modernization of the function/reference tensors (preserve only one of the two for the intro notebook).
2. M6 sections 4 (convolutions) and 5 (TeX bridge); ship intro at sections 1–3 only.
3. M4 runtime `tensor::tex(...)`; ship `_tex` compile-time only.

Anything else (M1, M2, M3 core, M5 minimum) is non-negotiable for `0.0.1-alpha`.

## Follow-ups beyond Phase 1

- Plan Phase 2 (autograd) in `2026-07-XX_revival-phase-2.md` when this file closes.
- Update `arc42/05-building-blocks/` per milestone.
- Consider whether xeus-cling 's C++20 limitations push us to a [pyodide-based browser kernel](https://pyodide.org/) for browser-runnable demos in Phase 3.

## References

- [ADR-0001..0008](../arc42/09-decisions/)
- `arc42/01-introduction-and-goals/overview.md` (Phase 1 closes G-1, G-2, G-3 partial, G-5)
- `docs/reports/2026-05-10_tensor-revival-landscape.md`
- [xeus-cling](https://github.com/jupyter-xeus/xeus-cling)
- [Kokkos mdspan](https://github.com/kokkos/mdspan)
