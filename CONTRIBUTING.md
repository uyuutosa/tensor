# Contributing to `tensor`

Thanks for your interest. `tensor` aspires to **canonical-reference-quality** documentation and design for differentiable named-axis tensor algebra in modern C++ ([ADR-0015](./docs/arc42/09-decisions/0015-aspire-to-canonical-reference-quality-not-self-anoint.md), superseding [ADR-0013](./docs/arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md); refining [ADR-0001](./docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) + [ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)). Educational-first, production-capable via backend adapters. Contributions are welcome on the same terms: clarity > correctness > portability > performance, and the educational + citable artifact is the primary deliverable.

A contribution that honors the canonical-reference disciplines is one that adds **depth** in the existing surface — a clearer derivation, a tighter test, a more readable ADR, a glossary entry naming a previously-unsourced symbol — rather than **breadth** for breadth's sake (e.g. a 17th activation function added "for completeness" is unlikely to land; a redesign that makes the existing six activations more legible likely will). When in doubt, open an issue first and link the source (paper / textbook / ADR) the change descends from.

## Quickstart for contributors

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor

# Build + test (reference backend)
cmake --preset=default
cmake --build --preset=default
ctest --preset=default

# Optional: build with Eigen backend
cmake --preset=default -DTENSOR_KERNEL_BACKEND=eigen
cmake --build --preset=default
ctest --preset=default

# Optional: build + run the perf bench
cmake --preset=default -DTENSOR_BUILD_BENCH=ON -DCMAKE_BUILD_TYPE=Release
cmake --build --preset=default --target tensor_bench
./build/default/bench/tensor_bench
```

## Where work happens

| Layer                              | Location                                                                          |
| ---------------------------------- | --------------------------------------------------------------------------------- |
| Domain types and ops               | [`include/tensor/core/`](./include/tensor/core/)                                  |
| Autograd subsystem                 | [`include/tensor/autograd/`](./include/tensor/autograd/)                          |
| TeX / LyX authoring surface        | [`include/tensor/tex/`](./include/tensor/tex/) + [`lyx-export/`](./lyx-export/)   |
| Kernel backends                    | [`include/tensor/core/backend/`](./include/tensor/core/backend/)                  |
| Tests                              | [`tests/`](./tests/)                                                              |
| Bench                              | [`bench/`](./bench/)                                                              |
| Tutorials                          | [`tutorials/`](./tutorials/)                                                      |
| Jupyter Book site                  | [`book/`](./book/)                                                                |
| Architecture documentation         | [`docs/arc42/`](./docs/arc42/), [`docs/impl-plans/`](./docs/impl-plans/), [`docs/reports/`](./docs/reports/) |

## Branching and PRs

We follow **Git Flow** with the modifications spelled out in [`.claude/rules/version-control.md`](./.claude/rules/version-control.md). The short form:

- `main` is production-ready; only release / hotfix merges land here.
- `develop` is the integration line; feature branches merge here.
- Feature branch naming: `feature/<kebab-description>`; bugfix: `bugfix/<kebab>`.
- Open the PR against `develop`. Use merge commits (no squash) so the integration boundary is visible in `git log --first-parent`. **Squash-merge is forbidden** because it erases the `git-subtree-dir:` metadata required by `libs/pentaglyph-docs/` (see [`.claude/rules/version-control.md` §Hard rules #8](./.claude/rules/version-control.md)).
- Conventional Commits subject lines: `feat:` / `fix:` / `refactor:` / `docs:` / `test:` / `chore:` / `ci:` / `perf:` / `style:` / `build:` / `revert:`.

## Architectural discipline (ADR-0009 / ADR-0011)

This project enforces a **Hexagonal "lite"** layering. The hard rule from [`docs/design-guide/architectural-discipline.md`](./docs/design-guide/architectural-discipline.md):

> The Domain depends on no Adapter; Adapters depend on the Domain via concepts; Adapters do not depend on each other.

When you open a PR that touches `include/`, paste this checklist into the description:

```markdown
### Hexagonal discipline (ADR-0009 / ADR-0011)
- [ ] No header in `include/tensor/core/` (excluding `concepts.hpp` and
      `include/tensor/core/backend/<adapter>/`) includes from
      `include/tensor/{tex,autograd}/` or from another adapter.
- [ ] No header in `include/tensor/core/backend/<adapter>/` includes from
      another `backend/<adapter>/`.
- [ ] Any new container is classified `Domain` / `DrivingAdapter` /
      `DrivenAdapter` in `docs/arc42/05-building-blocks/overview.md` and
      mirrored in `docs/diagrams/c4/workspace.dsl`.
- [ ] Any new port is declared as a C++20 concept in the owning container's `concepts.hpp`.
- [ ] Any new adapter has `static_assert(KernelBackend<Backend>)` next to
      its definition (the conformance pattern documented in
      `docs/detailed-design/kernel-backend-port.md`).
```

## Vendored third-party code

Anything bus-factor 1 in our substrate stack is vendored, not linked ([ADR-0014](./docs/arc42/09-decisions/0014-external-substrate-strategy.md)). As of 2026-05-12, `third_party/` is empty — the only vendored substrate (`gpu_cpp/` at tag 0.2.0) was removed once [ADR-0016](./docs/arc42/09-decisions/0016-substrate-refinement-drop-gpu-cpp-talk-to-dawn-directly.md) superseded ADR-0014 §Decision Outcome point 2 (the project talks to Dawn directly via `webgpu_cpp.h` now). The discipline below still applies for any future vendoring (e.g. if we vendor `kokkos/stdBLAS` to ship the `tensor::linalg` shim per ADR-0014 §Decision Outcome point 4).

Rules for vendored directories:

1. Every subdirectory of `third_party/` must contain a `VENDORED_FROM` file naming upstream repo, commit / tag, license, and the re-vendor procedure. The CI script [`tools/check-vendored.sh`](./tools/check-vendored.sh) enforces this.
2. **Do not patch vendored code.** If upstream has a bug, fix it upstream (or document the workaround in our consuming code, not in the vendored file). When upstream releases a fix, re-vendor.
3. To re-vendor: pick a newer upstream commit (preferably a tagged release), fetch the file(s), update the `VENDORED_FROM` pin, and open a PR titled `third_party: re-vendor <project> at <tag>`.
4. Preserve the upstream `LICENSE`. Vendored code keeps its own license; the repository's MIT license applies only to first-party code under `include/`, `tests/`, `bench/`, `tutorials/`, and `docs/`.

## Documentation alongside code

Every PR that lands code under `include/` updates either [`docs/arc42/05-building-blocks/`](./docs/arc42/05-building-blocks/) or [`docs/detailed-design/`](./docs/detailed-design/). This is enforced by convention, not (yet) by CI. New phases of work get a new dated impl-plan under [`docs/impl-plans/`](./docs/impl-plans/); closed phases get a retrospective under [`docs/reports/`](./docs/reports/).

Architecturally significant decisions are recorded as **MADR v3.0 ADRs** under [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/). ADRs in `Accepted` status are immutable; supersede with a new ADR rather than editing.

## Tutorials

Tutorial notebooks live under [`tutorials/`](./tutorials/). When you ship a new feature that learners would expect to see explained, consider adding a section to an existing notebook rather than writing a new one. Notebook authoring guidelines:

- One concept per section; 1–3 cells per concept.
- Output cells are committed (the Jupyter Book site renders them). [`notebook-ci.yml`](./.github/workflows/notebook-ci.yml) re-executes every notebook weekly against xeus-cpp 0.10+ per [ADR-0014 §3](./docs/arc42/09-decisions/0014-external-substrate-strategy.md), with a parallel `legacy-xeus-cling` smoke job that runs only `00_intro.ipynb` to keep older conda-forge channels green.
- Cross-link to the relevant ADR or `docs/` section so learners can fall through to the architecture surface.

## Reporting issues

Currently there's no issue template. When opening an issue, please include:

- What you tried (command line and / or minimal C++ snippet).
- What you expected.
- What you got (output / error message / build log excerpt).
- Compiler + OS + CMake version (`cmake --version`, `c++ --version`).

## Code of conduct

This project adopts the [Contributor Covenant](./CODE_OF_CONDUCT.md). Be kind, assume good faith, and remember that the project is run on solo bandwidth — fast responses are not guaranteed.

## License

MIT. By contributing you agree your contributions are licensed under the same terms.
