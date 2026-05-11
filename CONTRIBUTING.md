# Contributing to `tensor`

Thanks for your interest. `tensor` is an **educational-first** library ([ADR-0001](./docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) refined by [ADR-0010](./docs/arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)); contributions are welcome on the same terms — clarity > correctness > portability > performance, and the educational artifact is the primary deliverable.

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
### Hexagonal discipline (ADR-0009)
- [ ] No header in `include/tensor/core/` (excluding `concepts.hpp`) includes
      from `include/tensor/{tex,gpu,autograd}/`.
- [ ] No header in `include/tensor/<adapter>/` includes from any other adapter.
- [ ] Any new container is classified `Domain` / `DrivingAdapter` /
      `DrivenAdapter` in `docs/arc42/05-building-blocks/overview.md`.
- [ ] Any new port is declared as a C++20 concept in the owning container's `concepts.hpp`.
```

## Documentation alongside code

Every PR that lands code under `include/` updates either [`docs/arc42/05-building-blocks/`](./docs/arc42/05-building-blocks/) or [`docs/detailed-design/`](./docs/detailed-design/). This is enforced by convention, not (yet) by CI. New phases of work get a new dated impl-plan under [`docs/impl-plans/`](./docs/impl-plans/); closed phases get a retrospective under [`docs/reports/`](./docs/reports/).

Architecturally significant decisions are recorded as **MADR v3.0 ADRs** under [`docs/arc42/09-decisions/`](./docs/arc42/09-decisions/). ADRs in `Accepted` status are immutable; supersede with a new ADR rather than editing.

## Tutorials

Tutorial notebooks live under [`tutorials/`](./tutorials/). When you ship a new feature that learners would expect to see explained, consider adding a section to an existing notebook rather than writing a new one. Notebook authoring guidelines:

- One concept per section; 1–3 cells per concept.
- Output cells are committed (the Jupyter Book site renders them; xeus-cling re-execution is part of the future CI plan).
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
