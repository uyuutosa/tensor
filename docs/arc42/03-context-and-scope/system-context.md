---
status: Draft
owner: tensor
last-reviewed: 2026-05-10
---

# `tensor` — System Context (arc42 §3)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §3 (Context and Scope)                                   |
| Owner         | uyuutosa                                                       |
| Related       | `arc42/01-introduction-and-goals/overview.md`; ADR-0001..0008  |
| Last Updated  | 2026-05-10                                                     |

> The C4 Level 1 system-context diagram lives in [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl). Render via the [`/diagram-render`](../../../.claude/skills/diagram-render/SKILL.md) skill; this file mirrors the diagram in prose and tables.

## Reading order

1. **Actors** — who interacts with `tensor`.
2. **External systems** — what `tensor` depends on at runtime / build time.
3. **Boundaries** — what is explicitly inside vs. outside the system.
4. **Out-of-band channels** — non-runtime relationships (e.g. upstream subtree).

---

## 1. Actors

| Identifier   | Role                                | Goal                                                                                       |
| ------------ | ----------------------------------- | ------------------------------------------------------------------------------------------ |
| `learner`    | Self-taught C++ programmer or researcher | Understand named-axis tensor algebra, autograd, and modern C++ by reading and running the tutorials. Primary audience. |
| `instructor` | University / bootcamp instructor    | Assign tutorials and exercises as coursework. Secondary audience.                          |

The library is **not** intended for production engineers picking a tensor lib for a real workload — that audience is redirected to Eigen / xtensor / libtorch / Kokkos / `std::linalg` (see [ADR-0001](../09-decisions/0001-pivot-to-educational-named-axis-dsl.md) §Stakeholders).

## 2. External systems

| Identifier            | Description                                                                          | Interface                              |
| --------------------- | ------------------------------------------------------------------------------------ | -------------------------------------- |
| `toolchain`           | Host C++ compiler + build tools: GCC 11+ / Clang 13+ / MSVC 19.30+, CMake 3.25+, vcpkg | Compile-time. `tensor` is consumed as a header-only CMake interface library. |
| `jupyterStack`        | Jupyter Lab / Notebook with the [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) C++20 kernel | Runtime. Tutorials execute inside this kernel. |
| `webgpuRuntime`       | [Dawn](https://dawn.googlesource.com/dawn) (desktop) or [wgpu-native](https://github.com/gfx-rs/wgpu-native) — uses the OS GPU driver already on the learner's machine | Runtime. `tensor::gpu` dispatches WGSL kernels through the [WebGPU API](https://www.w3.org/TR/webgpu/). Phase 3+. |
| `browser`             | Chrome / Firefox / Safari with WebGPU support                                        | Reads the Jupyter Book site. (Future) runs browser-side demos. |
| `ghPages`             | GitHub Pages                                                                         | Build-time deploy target for the Jupyter Book site.            |
| `upstreamPentaglyph`  | The [pentaglyph-docs](https://github.com/uyuutosa/pentaglyph-docs) repository — vendored as a git subtree at `libs/pentaglyph-docs/` | Build-time / authoring-time. `git subtree pull` keeps the kit current. |

## 3. Boundaries — what is inside the system

The system under design is **the `tensor` repository as a whole**, including:

- The header-only C++20 library (`tensor/` namespace, distributed via `include/`).
- The `tutorials/` notebook corpus.
- The Jupyter Book site generated from those notebooks.
- The (Phase 3+) LyX export module.

The system does **not** include the C++ toolchain, the Jupyter stack, or the WebGPU runtime — those are external dependencies the learner installs. The rule of thumb: anything the maintainer can fix by editing this repo is inside; anything else is outside.

## 4. Out-of-band channels (non-runtime)

| Channel                                       | Direction          | Purpose                                                                  |
| --------------------------------------------- | ------------------ | ------------------------------------------------------------------------ |
| `git subtree pull --prefix=libs/pentaglyph-docs ... main --squash` | inbound | Pulls upstream documentation-kit changes from `uyuutosa/pentaglyph-docs`. |
| `git subtree push` (rare)                     | outbound           | Pushes documentation-kit improvements made here back upstream.            |
| GitHub Issues / PRs                           | bidirectional      | Community contributions and discussion.                                   |
| Jupyter Book site (`ghPages`)                 | outbound           | Public-facing reading surface; unlike issues this is one-way.             |
| The 2016 Qiita blog post                      | inbound (legacy)   | Source content for `tutorials/00_intro.ipynb`. Treated as external prior art. |

## 5. Relationships (mirrors the DSL)

| From          | To                | Verb                                                       |
| ------------- | ----------------- | ---------------------------------------------------------- |
| `learner`     | `tensor`          | Builds against, reads, and runs tutorials of               |
| `instructor`  | `tensor`          | Assigns tutorials and exercises from                       |
| `tensor`      | `toolchain`       | Is consumed by (CMake + vcpkg)                             |
| `tensor`      | `jupyterStack`    | Tutorials run inside (xeus-cling kernel)                   |
| `tensor`      | `webgpuRuntime`   | Dispatches GPU kernels through (Phase 3+)                  |
| `tensor`      | `ghPages`         | Jupyter Book site is published to                          |
| `tensor`      | `upstreamPentaglyph` | Vendored documentation kit is pulled from               |

L2 (container-level) relationships are documented in [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md) to avoid duplicating the DSL.

## 6. What is *not* in scope here

- Internal decomposition of the library — see [`../05-building-blocks/`](../05-building-blocks/).
- Detailed runtime sequences (e.g. autograd backward pass) — see [`../06-runtime/`](../06-runtime/).
- Module HOW (data layout, kernel implementation) — see [`../../detailed-design/`](../../detailed-design/).

## Cross-references

- §1 Introduction and Goals: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §4 Solution Strategy: [`../04-solution-strategy/strategy.md`](../04-solution-strategy/strategy.md)
- §5 Building Blocks: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §9 Decisions: [`../09-decisions/`](../09-decisions/)
- C4 source: [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl)
