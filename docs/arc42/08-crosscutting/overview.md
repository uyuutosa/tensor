---
status: Draft
owner: tensor
last-reviewed: 2026-05-12
---

# `tensor` — Crosscutting Concepts (arc42 §8)

| Metadata      | Value                                                          |
| ------------- | -------------------------------------------------------------- |
| Status        | Draft                                                          |
| Type          | arc42 §8 (Crosscutting Concepts)                               |
| Owner         | uyuutosa                                                       |
| Last Updated  | 2026-05-11                                                     |

> Per arc42 §8: this file collects **principles that cross multiple building blocks** — error handling, memory, concurrency, testing strategy, naming, etc. A topic lives here when it applies to more than one container in [§5](../05-building-blocks/overview.md). Module-local concerns belong in detailed-design docs ([`../../detailed-design/`](../../detailed-design/)).

## 1. Error-handling policy

The library does **not** use exceptions for predictable error conditions. Instead:

- **Compile-time validation** is preferred. The `TypedTensor<T, "i", "j", ...>` path catches axis-label mismatches via `static_assert`; the `KernelBackend` concept catches an under-implemented adapter at the `static_assert(KernelBackend<B>)` line at the bottom of every backend header.
- **`assert(condition && "explanation")`** for invariant violations that can in principle reach a `Variable`'s backward closure or a `Tensor`'s constructor with mismatched shapes. These compile out of release builds, matching the educational-first quality priority (clear failure in Debug, performant in Release).
- **Exceptions** are reserved for **the parser** (`tensor::tex::parse`) — user-supplied input can legitimately be malformed, and the parser throws `tensor::tex::ParseError` with a position-annotated message.
- **No `std::expected`** today; the project's C++20 baseline ([TC-1](../02-architecture-constraints/overview.md)) does not assume it. A C++23 baseline bump would enable a future migration of the parser's `throw` to `std::expected` for non-exception-friendly consumers.

This policy keeps the educational kernel for arithmetic operators a single legible for-loop (no try/catch / no expected unwrapping) while honoring the canonical-reference framing's preference for compile-time correctness ([G-8 ubiquitous-language discipline](../01-introduction-and-goals/overview.md)).

## 2. Memory management

- **No raw `new` / `delete` in the public surface.** `Tensor<T, N>` and `DynamicTensor<T>` own their buffers via `std::vector<T>`; `TypedTensor<T, Labels...>` is a thin wrapper.
- **Each operator returns a new tensor.** The reference adapter allocates per call; the Eigen adapter reuses Eigen's stack-allocated map types where possible but still allocates the result. The WebGPU adapter (shipped per [PRs #60–#62](https://github.com/uyuutosa/tensor/pulls?q=is%3Apr+is%3Amerged)) allocates Dawn `wgpu::Buffer` objects per dispatch; the `WebGPUContext` singleton ([`webgpu_detail/context.hpp`](../../../include/tensor/core/backend/webgpu_detail/context.hpp)) keeps Instance / Adapter / Device / Queue alive across calls, so only buffers (not the Dawn objects) are re-created. Per-call allocation is a quality-4 (performance) tradeoff that priorities 1–3 ([§10 quality](../10-quality/overview.md)) explicitly trump.
- **No expression-template fusion.** See [`detailed-design/tensor-core.md §5.3`](../../detailed-design/tensor-core.md) for the alternative-considered rejection.
- **Tape allocation.** `tensor::autograd::Tape` is a `thread_local std::vector<std::function<void()>>`; each registered backward closure allocates one `std::function`. This is per-call allocation pressure that the [discussion-points report Axis C](../../reports/2026-05-11_open-discussion-points.md) flags as a future profile-driven investigation target.

## 3. Concurrency model

- **Single-threaded user-side execution.** `tensor::autograd::Tape::current()` is `thread_local`, so two threads independently doing autograd will each have their own tape. There is no synchronisation primitive in the public surface.
- **Eigen backend** internally uses SIMD; this is parallelism *within* one core's vector lanes, not across threads.
- **WebGPU backend** is async on the GPU side (dispatch + wait per call per [ADR-0012](../09-decisions/0012-webgpu-adapter-implementation-design.md)), but presents a synchronous interface to the caller. Caller-side multi-threading interacts with one Dawn `Context` per `Backend` instance; the safe pattern is one `Backend` per thread.
- **No parallel runtime libraries** (`std::execution`, OpenMP, TBB) are linked. The educational-first identity prioritises one-thread legibility; parallelism is a quality-4 concern revisited only when profile evidence demands it.

## 4. Testing strategy

The project tests across four orthogonal axes (see [§10 quality scenarios QO-1..QO-3](../10-quality/overview.md) for the response measures):

1. **Per-container unit tests** — one `tests/test_<container>_<feature>.cpp` per concern. 20+ source files exercising tensor::core, tensor::autograd, tensor::tex, and the three KernelBackend adapters. Listed in [`detailed-design/tensor-core.md §6`](../../detailed-design/tensor-core.md).
2. **Cross-backend numerical agreement** — `tests/test_eigen_backend.cpp` and `tests/test_webgpu_backend.cpp` assert that `reference::Backend` and the alternative adapter produce element-wise-agreeing tensors within a documented tolerance (`1e-9` for `double`, `1e-5` for `float`). This is the canonical correctness witness for "any new backend implementation is a working KernelBackend". Phase 6.5 adds `python/tests/test_backend_parity.py` mirroring this discipline from the Python side once `set_backend()` ships.
3. **Autograd gradient_check** — every autograd primitive's analytical gradient is finite-difference-checked at every release. Tests live alongside the primitives in `tests/test_autograd_*.cpp`.
4. **Python pytest suite** — `python/tests/test_smoke.py`, `test_arithmetic.py`, `test_contract_numpy.py`, `test_autograd.py`, `test_autograd_extensions.py`, `test_tex.py`. Run on `ubuntu-latest × CPython 3.11` via [`.github/workflows/python-wheel-smoke.yml`](../../../.github/workflows/python-wheel-smoke.yml) on every PR. Each test cross-validates the Python entry point against the same numerical result the C++ tests assert, within `1e-12` for `double` / `1e-5` for `float`.

**CI matrix shape (post-PR #113).** The 9-job C++ matrix executes on every PR ([QP-1](../10-quality/overview.md)); the Python wheel smoke adds one more job; the deploy-book + notebook-validate jobs cover the docs side. Develop has been 100% green since PR #113 fixed the long-standing MSVC `__msvc_string_view` parse bug and the clang-13 frontend segfault on `test_label_tag.cpp` (`clang-13` → `clang-15` matrix bump). The cibuildwheel matrix (release-only, ~20 wheels per tag for `0.2.0` / ~52 for `0.3.0`) is triggered by tag push only.

**Notebook output gate** ([`.github/workflows/notebook-ci.yml`](../../../.github/workflows/notebook-ci.yml) `validate` job, added PR #118). Every `python/notebooks/*.ipynb` must be committed with non-null `execution_count` AND at least one output cell. Catches the "notebook committed source-only → Jupyter Book renders no figures" bug class (caught the hard way in PR #117 when the MVG 3D plotly scenes were invisible on the published site). C++ tutorials under `tutorials/` are not gated because xeus-cpp execution requires a conda environment; their execution is covered by the weekly cron in the same workflow.

## 5. Naming and vocabulary conventions

Every public name traces to a source — paper, ADR, or textbook — per the G-8 citability discipline ([ADR-0013](../09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)). The [§12 glossary](../12-glossary/overview.md) is the project bibliography for the vocabulary; if a name appears twice with different definitions, file an issue.

Specific conventions:

- **C++ namespace = container name.** `tensor::core::Tensor`, `tensor::autograd::Variable`, `tensor::tex::Evaluator`.
- **No abbreviations in public names.** `gpu`, not `g`; `autograd`, not `ag`; `KernelBackend`, not `KBackend`.
- **Plural file names mirror plural namespace concepts.** `concepts.hpp`, `ops.hpp` — not `concept.hpp`, `op.hpp`.
- **Tests follow the implementation file name** with `test_` prefix: `axis.hpp` → `tests/test_axis_shape.cpp`. Minor renames over time are caught by CI when the prefix mapping is broken.
- **WGSL kernel constants** end in `F32` for the floating-point variant, leaving room for future `F16` / `F64` siblings without renaming.

## 6. Architectural discipline (single hard rule)

From [ADR-0009](../09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md), inherited by every PR:

> A header under `include/tensor/core/` that is not `concepts.hpp` may not `#include` anything from `include/tensor/{tex,autograd}/` or from another adapter under `include/tensor/core/backend/`.

The Domain (`core` + `autograd` extension + ports in `concepts.hpp`) depends on no adapter. Adapters depend on `concepts.hpp`. **No adapter depends on another adapter directly.** This rule keeps the Hexagonal-lite shape legible and would be enforced by CI when the project adds the architectural-discipline grep job; until then it is reviewed at PR time. Full motivation and enforcement plan: [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md).

## 7. Versioning and ABI policy

- The library is alpha until the maintainer cuts `0.1.0`. Even at `0.1.0`, the project does **not** commit to ABI stability — see [TC-8](../02-architecture-constraints/overview.md) and [ADR-0010 §Decision Outcome point 3](../09-decisions/0010-refine-positioning-to-educational-first-production-capable.md).
- Header-only distribution ([TC-2](../02-architecture-constraints/overview.md)) means "ABI" practically reduces to "do user-side templates that include this version of the headers still compile against the next version?". The answer: maybe; review the CHANGELOG `[Unreleased]` block for breaking-shape changes.
- Pinned vcpkg baseline + vendored `third_party/` content means the *external* surface (Dawn, Eigen, fmt) is stable for any given commit of this repo. Re-vendoring discipline ([TC-9](../02-architecture-constraints/overview.md)) governs upgrades.

## 8. Security posture

- **Header-only library**, no networked component, no IPC surface. Attack surface is the typical C++ template-instantiation surface (compiler diagnostics on malformed user code).
- **Vendored content** under `third_party/` carries upstream's license and is reviewed when re-vendored. `tools/check-vendored.sh` enforces presence of `VENDORED_FROM`.
- **No PII / regulated computation.** The library is suitable for educational, research, and non-regulated production workloads. Regulated industries should self-validate against their own requirements; the project offers no certification.

## 9. Internationalisation

The library has no localised strings. ADRs, arc42, detailed-design, CHANGELOG, README, and tutorials are English ([OC-4](../02-architecture-constraints/overview.md)); contributor conversations (issues, PRs) may be in any language. The maintainer of record (uyuutosa) maintains the project in both English (formal) and Japanese (informal).

## 10. Logging policy

No logger. `tensor::core` performs no logging; `tensor::autograd::gradient_check` and the test suite print to `std::cout` / `std::cerr` directly. If a future contributor wants to add diagnostic logging to the WebGPU dispatch wiring ([`webgpu_detail/dispatch.hpp`](../../../include/tensor/core/backend/webgpu_detail/dispatch.hpp)), the canonical choice is `fmt::print` (already a dependency).

## 11. Cross-references

- §1 §G-8 citability discipline: [`../01-introduction-and-goals/overview.md`](../01-introduction-and-goals/overview.md)
- §2 constraints this concept-set respects: [`../02-architecture-constraints/overview.md`](../02-architecture-constraints/overview.md)
- §5 building blocks each concept applies to: [`../05-building-blocks/overview.md`](../05-building-blocks/overview.md)
- §9 ADRs cited above: [`../09-decisions/`](../09-decisions/)
- §10 quality scenarios these concepts support: [`../10-quality/overview.md`](../10-quality/overview.md)
- §12 vocabulary anchored here: [`../12-glossary/overview.md`](../12-glossary/overview.md)
- Architectural-discipline design guide: [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md)
