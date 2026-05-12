---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0001: Pivot `tensor` to an educational / DSL-research project anchored on named-axis tensor algebra

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief: `docs/reports/2026-05-10_tensor-revival-landscape.md`) |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The `tensor` C++ library has been dormant since ~2016. Reviving it as a *production* dense linear algebra / deep learning library competes head-on with [Eigen 5.0 (2025)](https://libeigen.gitlab.io/releases/5.0/), [xtensor](https://github.com/xtensor-stack/xtensor), [libtorch](https://docs.pytorch.org/docs/stable/notes/libtorch_stable_abi.html), [Kokkos 5](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/), and the upcoming standard library [`std::mdspan` + `std::linalg`](https://isocpp.org/files/papers/P1673R13.html). That competition is unwinnable for a one-person revival.

The library does have a genuinely differentiated design surface: **named-axis (Einstein-notation) tensor algebra with function tensors, reference tensors, and convolution-as-tensor-inner-product** — see `docs/reports/2026-05-10_tensor-revival-landscape.md` §2. Named-axis APIs are *underserved* in C++; PyTorch named tensors have stalled at "prototype", JAX `xmap` was deleted, and existing C++ einsum-flavored libs ([Einsums](https://einsums.github.io/Einsums/), [Fastor](https://github.com/romeric/Fastor), [Tenseur](https://istmarc.github.io/post/2024/10/27/on-designing-tenseur-a-c-tensor-library-with-lazy-evaluation/), [TCL](https://github.com/springer13/tcl)) attack the space from non-overlapping angles.

We must decide the project's positioning before any rewrite begins, because the positioning controls every downstream decision (build system, GPU strategy, API shape, ABI commitments).

---

## Decision Drivers

- **DD-1**: Solo maintainer bandwidth. The project must be defensible as a one-person effort.
- **DD-2**: Differentiation from the saturated production niche. The project must exist where existing libraries do not.
- **DD-3**: Pedagogical clarity. Existing artifacts (the [author's blog post on convolutions-as-tensor-inner-products](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74)) are pedagogical in spirit; the rewrite should amplify that.
- **DD-4**: Long-term identity. The narrative ("this is a teaching library for named-axis tensor algebra in modern C++") should fit on one slide.

---

## Considered Options

1. **Production-grade tensor library** — compete with Eigen / xtensor / libtorch on speed/coverage.
2. **Educational / DSL-research library** — explicitly framed as a teaching artifact for named-axis tensor algebra and adjacent ideas (function tensors, reference tensors, convolutions-as-inner-products), with bundled pedagogy (notebooks, blog, book).
3. **Hybrid** — production-grade with an educational outer layer.

---

## Decision Outcome

**Chosen option: 2 — Educational / DSL-research library, anchored on named-axis as the headline feature.**

The named-axis story is the most differentiated and most sellable; function tensors, reference tensors, and the convolution-as-inner-product reformulation become *teaching narratives* that motivate the named-axis substrate rather than competing with it for the spotlight. The pedagogical frame solves DD-1 (no need to chase performance benchmarks) and DD-2 (no overlap with Eigen/libtorch territory). The hybrid option (3) was rejected because attempting to be both production and educational dilutes both audiences and inflates maintenance.

### Y-statement summary

> In the context of **reviving the dormant `tensor` C++ library**, facing **a saturated production niche dominated by Eigen, xtensor, libtorch, Kokkos, and the upcoming `std::mdspan` + `std::linalg`**, we decided for **explicit educational / DSL-research positioning anchored on named-axis tensor algebra** to achieve **a defensible single-maintainer scope and a differentiated narrative**, accepting **that the library will not pursue production-grade performance or coverage**.

---

## Pros and Cons of the Options

### Option 1: Production-grade tensor library

- Pros:
  - Largest theoretical market.
- Cons:
  - Cannot match Eigen 5 / Kokkos / libtorch / `std::linalg` on perf, coverage, or hardware support with one maintainer.
  - Forces commitments (CUDA-direct, ABI stability, sparse, distributed) that solo bandwidth cannot sustain.

### Option 2: Educational / DSL-research library (chosen)

- Pros:
  - Genuine whitespace: no incumbent owns "named-axis tensor algebra in modern C++".
  - Allows aggressive scope discipline — features that don't teach can be cut.
  - Pairs naturally with bundled pedagogy (Jupyter via [xeus-cling](https://github.com/jupyter-xeus/xeus-cling), the existing convolutions blog, future TeX/LyX integration — see ADR-0005).
  - Solo-maintainer-friendly: "one good idea, well taught" is achievable.
- Cons:
  - Smaller audience than production positioning.
  - Educational projects can drift into "abandonware after one tutorial" without ongoing investment.

### Option 3: Hybrid

- Pros:
  - In theory, captures both audiences.
- Cons:
  - In practice, dilutes both. Production users notice perf gaps; learners notice complexity creep.
  - Doubles the API surface and roadmap.

---

## Consequences

### Positive

- All downstream ADRs (build system, GPU backend, API shape, distribution) inherit a clear north star.
- Permits aggressive cuts: anything that does not improve named-axis pedagogy is out of scope.
- Aligns with the project's historical character (the original blog post + Eclipse CDT toy).

### Negative

- README, GitHub repo description, and any future package metadata must explicitly disclaim production use. Users hitting perf walls will be told "use Eigen / xtensor / libtorch" by design.
- Forecloses any future pivot toward production unless this ADR is superseded.

### Neutral

- The "function tensor" and "reference tensor" features survive, but as teaching exhibits rather than headline features.

### Follow-ups

- [ ] Update `README.md` to lead with the educational pitch (after this ADR set lands).
- [ ] Update `docs/arc42/01-introduction-and-goals/` with the new positioning (this PR).
- [ ] Decide build system → ADR-0003.
- [ ] Decide named-axis API shape → ADR-0004.
- [ ] Decide TeX/LyX authoring surface → ADR-0005.

---

## More Information

### Related ADRs

- Implementation foundation: [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)
- Build system: [ADR-0003](0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)
- Named-axis API shape: [ADR-0004](0004-adopt-hybrid-named-axis-api.md)
- TeX/LyX integration: [ADR-0005](0005-adopt-tex-lyx-as-authoring-surface.md)
- GPU backend: [ADR-0006](0006-adopt-webgpu-as-gpu-backend.md)
- Autograd: [ADR-0007](0007-adopt-autograd-as-first-class-subsystem.md)
- Distribution: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)

### References

- Research brief: `docs/reports/2026-05-10_tensor-revival-landscape.md`
- Author's [convolutions-as-tensor-inner-products blog post (Qiita)](http://qiita.com/uyuutosa/items/12e87f4695bd151b1d74)
- [tinygrad](https://github.com/tinygrad/tinygrad) — reference exemplar of the educational-then-production genre
- [micrograd](https://github.com/karpathy/micrograd) — canonical teaching artifact
- [Tensor Considered Harmful (Harvard NLP)](https://nlp.seas.harvard.edu/NamedTensor)
