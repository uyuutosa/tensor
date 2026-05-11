---
status: Stable (open for direction)
owner: tensor
last-reviewed: 2026-05-11
---

# Open discussion points after 33 merged PRs

| Metadata     | Value                                                          |
| ------------ | -------------------------------------------------------------- |
| Status       | Stable; each section is **open for the maintainer's direction**. |
| Type         | Layer B — discussion / planning report (dated, append-only)     |
| Owner        | uyuutosa                                                       |
| Triggered by | maintainer asked for 論点整理 + 調査議論 (2026-05-11)               |
| Followed by  | individual ADRs and impl-plans triggered by chosen directions   |

## Purpose

After PRs #1–#33 the project has shipped Phase 1, Phase 2, Phase 2.5, most of Phase 1.5 mop-up, Phase 3 design (ADR-0012), and a Phase 4 prep scaffold. With this density of work landed, several **next-decision points** are visible that no single ADR or impl-plan captures cleanly. This report surveys those points across seven axes, synthesises the project's current state per axis, lays out 2–4 options, and recommends one — with the explicit caveat that **each is the maintainer's call**.

The report is dated and append-only per the Layer B convention; supersede with a new dated report when the open points are resolved.

---

## Axis A — Educational reach and visibility

**Synthesis.** The project ships strong educational artifacts (four notebooks, a Jupyter Book scaffold, twelve ADRs, two retrospectives, the LyX export plugin). Discoverability is currently zero outside the repo URL. Per [ADR-0010](../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md) production adoption is permitted but not invited, so the educational audience is the primary target.

**Options.**

1. **Quiet release** — push `0.1.0`, enable GitHub Pages, do nothing else; let the project be found organically by anyone searching for named-axis C++ libraries.
2. **Single high-quality launch post** — write one blog post / Show HN / Qiita follow-up to the 2016 article tying together the rewrite, the named-axis API, and the `_tex` bridge.
3. **Multi-channel launch** — blog + HN + a Reddit post in r/cpp, r/programminglanguages; reach LyX community via lyx.org forums.
4. **Talk submission** — submit a 30-minute talk to CppCon / Meeting C++ / CppNow 2027.

**Trade-offs.** (1) honours the educational-first stance but trades discoverability. (3) risks attracting production users who will be disappointed by the ADR-0010 "as-is" disclaimer; the disclaimer language needs to be front-loaded. (4) is the highest-leverage but the longest lead time.

**Recommendation.** **Option 2** — one well-crafted post built around the *"the formula is the program"* slogan and the `_tex` Evaluator demo (which is genuinely novel in C++). Promote on the author's existing Qiita / X audience. Avoids the production-user wave that (3) would create. (1) is fine but leaves the work invisible.

---

## Axis B — API surface tension: `Variable` vs `DynamicVariable` vs `TypedTensor`

**Synthesis.** The library currently ships three named-axis tensor types:

- `Tensor<T, N>` / `Variable<T, N>` — static rank, **runtime** axis labels.
- `DynamicTensor<T>` / `DynamicVariable<T>` — runtime rank.
- `TypedTensor<T, Labels...>` — static rank, **compile-time** axis labels (ADR-0004's NTTP path).

These are three points on a 2-dimensional axis (rank × label-time). Functionality overlap is real: `Variable<T, N>` exists primarily because rank-N is known at compile time for the autograd kernels; `TypedTensor` exists because labels can also be lifted to compile time; `DynamicVariable` exists because broadcast operators may collapse or expand rank at runtime.

**Options.**

1. **Status quo** — three types, document the use cases of each.
2. **Two-type collapse** — drop one of the three and let users go through conversions. Most natural drop: `Variable<T, N>` becomes a thin alias / deprecated; `DynamicVariable<T>` becomes the default.
3. **One-type collapse** — `TypedTensor` becomes the only public type; runtime-rank disappears, and broadcast operators promote to higher rank via metafunctions on label packs.
4. **Concept-based unification** — define `NamedTensor` / `NamedVariable` concepts; ship multiple implementations satisfying them; let user code be backend-agnostic on the type axis.

**Trade-offs.** (3) is most elegant but doesn't survive the *runtime-shape* reality of training loops with dynamic batch sizes. (2) is pragmatic but throws away the static-rank kernel optimisations Phase 2 took advantage of. (4) is academically appealing but doubles the documentation surface and probably exceeds Phase 1.5 mop-up time-budget.

**Recommendation.** **Option 1 (status quo) + better docs**: write a "which type when?" decision guide as a Diátaxis *how-to* under `tutorials/` (planned slot — `tutorials/02_function-and-reference-tensors.ipynb` could absorb this content, or a new short reference document under `docs/user-manual/how-to/`). The three types exist for legitimate reasons; the cost is documentation, not code.

---

## Axis C — Performance roadmap beyond P2.5.M4

**Synthesis.** [P2.5.M4 perf report](./2026-05-11_backend-performance-comparison.md) committed a reference baseline: matmul 512³ takes ~570ms on a quiet laptop. Eigen GEMM should crush this; WebGPU is the next leap. But several non-kernel performance dimensions are unaddressed:

- **Autograd tape allocation per call.** `Tape::current()` lives thread-local; each operator allocates a `std::function<void()>` plus shared_ptrs. For a 50-layer NN training loop this is real allocation pressure.
- **`DynamicTensor` round-trip cost in autograd.** Every broadcast op creates a fresh tensor; no in-place support.
- **Cold-start compile time.** The header-only library + template-heavy concepts make consumer build times non-trivial.

**Options.**

1. **Defer to user feedback** — wait until someone files a perf bug.
2. **Profile-driven improvements** — add a `bench/profile.sh` driver that runs the autograd MLP training loop under `perf record` (Linux); identify the top hot spots; address top 3.
3. **Aggressive refactor** — adopt a `TensorView` non-owning wrapper, arena allocator for tape entries, expression-template fusion for elementwise chains.
4. **Compile-time gains via modules** — switch from headers to C++20 modules.

**Trade-offs.** (3) starts to drift from the educational-first stance; expression templates make the source harder to read. (4) is bleeding-edge — toolchain support for modules in 2026 is uneven and adds CI complexity. (1) risks unaddressed pain when someone actually uses the library for training.

**Recommendation.** **Option 2 — profile-driven**. Allocate a 1-week investigation slice as Phase 1.5+ ("P1.5.M_perf"). Outcomes: a Layer B report on what's actually slow + at most three targeted fixes (e.g. a small-object optimisation for tape entries; a `Variable::detach` ergonomic for the rebuild pattern). Defer (3) and (4) until profile evidence demands them.

---

## Axis D — C++23 baseline upgrade timing

**Synthesis.** [ADR-0002](../arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md) pinned the project to C++20. By 2026-05, C++23 is broadly available: GCC 13+, Clang 16+, MSVC 19.34+, all of which ship `std::mdspan` natively (which lets us drop the polyfill alias work from PR #30). C++23 also brings `std::expected`, `std::print`, `std::ranges` extensions, deducing-this, and `if consteval`.

**Options.**

1. **Keep C++20** — ADR-0002 is in force; supersede only when a specific user demand emerges.
2. **Bump to C++23 in Phase 4** — coincides with `0.1.0` so the toolchain matrix updates once.
3. **Allow C++23 features behind a feature-test macro** — opt-in `if (__cpp_lib_expected) return std::expected<...>;` paths in the headers.

**Trade-offs.** (2) is the cleanest discontinuity but means dropping GCC 11 / Clang 13 from the support matrix; readers on older distros will need to upgrade. (3) preserves compatibility but doubles the code-path count.

**Recommendation.** **Option 1 (defer)**. C++20 was the right call for 2026; C++23 buys ergonomics but no headline features the project actually depends on yet. Revisit at Phase 5+ when `std::linalg` (P1673) is broadly available — that's the feature whose absence ADR-0002 explicitly accepted, and its arrival is the natural trigger for the bump.

---

## Axis E — WebGPU runtime stability concern

> **Resolution — 2026-05-11**: the original Option 1 (hold ADR-0012 with a 2-week timebox) was followed; PR #38 shipped the P3.M2 stub. The [external-substrate research (2026-05-11)](./2026-05-11_external-substrate-research.md) then audited the runtime question end-to-end. Key findings: Dawn is now in vcpkg as `20260410.140140`; gpu.cpp is bus-factor 1 with no vcpkg port. [ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md) captures the resolved strategy: **Dawn via vcpkg, gpu.cpp vendored under `third_party/gpu_cpp/`**, with `wgpu-native` left as an opt-in flag. The 2-week timebox is effectively retired by ADR-0014.

**Synthesis.** [ADR-0012](../arc42/09-decisions/0012-webgpu-adapter-implementation-design.md) picked **gpu.cpp on Dawn** with wgpu-native as fallback. As of May 2026:

- **gpu.cpp** — Answer.AI's project, single maintainer; active through 2025 but no major releases since. Risk: bus factor 1.
- **Dawn** — Google's WebGPU implementation; the engine behind Chrome's WebGPU; well-maintained but heavy to embed (build pulls in Tint shader compiler, ~1GB).
- **wgpu-native** — Mozilla / Linux Foundation; very active; C ABI integration is one more layer.

**Options.**

1. **Hold the ADR-0012 choice** — proceed to P3.M2 with gpu.cpp + Dawn.
2. **Switch primary to wgpu-native** — its activity profile is stronger; Rust-engine concerns minimal since we go through the C ABI.
3. **Pick later** — write Phase 3 P3.M2 as a "Backend slot with TBD runtime" stub; commit to a runtime at P3.M3.

**Trade-offs.** (1) commits early and lets implementation start; (2) hedges against gpu.cpp's bus factor at the cost of more integration code; (3) is the most expensive engineering choice because every other piece (CMake, vcpkg, CI) wants the runtime to be known.

**Recommendation.** **Option 1 with a 2-week timebox** — proceed with gpu.cpp + Dawn per ADR-0012. If P3.M2 (CMake plumbing) hits a vcpkg-port wall in ≤ 2 weeks, supersede ADR-0012 with ADR-0013 picking wgpu-native. Either path is reversible because the Hexagonal port keeps the runtime swap to one CMake variable. **(See resolution note above; ADR-0014 captures the final operational choice.)**

---

## Axis F — Project sustainability operationally

**Synthesis.** The repo currently has:

- No `CONTRIBUTING.md` (contribution flow undefined).
- No `CODE_OF_CONDUCT.md` (norms undocumented).
- No release cadence (no tags yet; `0.1.0` planned for Phase 4 close).
- No process for keeping the `arc42` sections and ADR text fresh as the codebase evolves (today done manually per PR).
- Doc audits today are reactive (the maintainer asked for one; it caught real drift).

**Options.**

1. **Defer everything to post-`0.1.0`** — ship the educational artifact first; project hygiene next.
2. **Light touch now** — write `CONTRIBUTING.md` (paste from the version-control rule + design-guide) and `CODE_OF_CONDUCT.md` (use the Contributor Covenant template). 1-PR change.
3. **Full setup now** — also add a "doc-freshness" CI lint that fails the build if `README.md`'s ADR count diverges from `ls docs/arc42/09-decisions/00*.md`, etc.

**Trade-offs.** (2) is cheap. (3) catches future doc-drift automatically but adds CI complexity for a solo-maintainer project. (1) is fine but the next contributor (likely a notebook reader) has no obvious onboarding ramp.

**Recommendation.** **Option 2** in the next PR; revisit (3) post-`0.1.0` if the maintainer sees contributor interest from the launch post (Axis A).

---

## Axis H — Python SDK (deferred to "after the C++ side is in order")

> **Captured 2026-05-11** at the maintainer's direction: *"裏側が整ったらPython SDKも考えたい、最後で良い"* — once the C++ side is in order, want to think about a Python SDK; it can be last.

**Synthesis.** Once Phase 3 dispatch wiring lands (P3.M3.2 / P3.M4.2 with a real GPU runner), Phase 4 `0.1.0` cuts, and Phase 5+ `tensor::linalg` shim lands ([ADR-0014](../arc42/09-decisions/0014-external-substrate-strategy.md)), the C++ surface is feature-complete enough that a Python wrapper would have a stable target to bind. The canonical-reference framing ([ADR-0013](../arc42/09-decisions/0013-reframe-as-canonical-reference-for-named-tensor-computation.md)) would extend naturally: the Python surface mirrors the C++ names, the ADR sequence remains the bibliography, citation tooling (`CITATION.cff`) covers both.

**Options.**

1. **pybind11 wrapper** — the standard choice; mature; verbose but well-understood. Same `Tensor`, `Variable`, `Evaluator` types exposed; NumPy interop via buffer protocol; DLPack for zero-copy with PyTorch / JAX.
2. **nanobind wrapper** — pybind11's faster successor from the same author. ~3× faster build, smaller binaries, type stubs out of the box. Newer; smaller community.
3. **Cython wrapper** — would let the SDK match NumPy / pandas ergonomics more closely. Heavier build dependency; loses some compile-time type information from the C++ side.
4. **Python-side reimplementation** — port the named-axis algebra to pure Python over NumPy. Rejected on sight: defeats the canonical-reference purpose; doubles maintenance.

**Trade-offs.** (1) is the safe boring choice. (2) is the high-leverage modern choice but the project would be one of nanobind's earlier large adopters. (3) misaligns with the C++-canonical posture.

**Recommendation (provisional, revisit when Phase 5+ ships).** **Option 2 — nanobind** is the natural fit because: (a) the project's canonical-reference posture is itself a "modern choice over conservative choice" stance; (b) nanobind's better build / binary characteristics matter more for an educational artifact a learner installs from PyPI; (c) the maintainer's solo bandwidth benefits from less boilerplate.

**Sequencing — what must land first.**

1. **Phase 3 P3.M3.2 / P3.M4.2 — dispatch wiring** (GPU runner + Eigen 5 baseline bump). Without this, the GPU half of the SDK has no story.
2. **Phase 4 `0.1.0` release tag**. The Python SDK targets a tagged C++ commit, not `develop` HEAD.
3. **Phase 5+ `tensor::linalg` shim**. Once `std::linalg` (P1673) is reachable through the shim, the Python SDK's `linalg` submodule has the same call sequence under both C++26-native and shimmed builds.

After those three, **Phase 6 — Python SDK** becomes the natural next major phase. Tentative scope: `tensor` PyPI package, `import tensor` brings in `Tensor` / `DynamicTensor` / `TypedTensor` / `Variable` / `tex.parse` / `tex.evaluate`; backend selection is a Python-side configure-time flag mapping to a `-DTENSOR_KERNEL_BACKEND=...` build of the wheel.

**Open questions for the maintainer at Phase 6 entry.** (a) PyPI publishing identity (maintainer's account or an org). (b) Whether to publish wheels via conda-forge as well — historically `xeus-cpp` is conda-forge, so conda-forge wheels would close the loop with the notebook story. (c) Whether the Python SDK gets its own repo or stays a subdirectory `python/` here.

**No action now**; capture is so this direction does not get forgotten.

## Axis G — Sparse tensors, distributed training, autograd higher-order

**Synthesis.** [ADR-0001 §Out of scope](../arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) explicitly rules out:

- Sparse tensors as first-class.
- Distributed (multi-node) training.
- Forward-mode and source-to-source autograd.

The rules were correct for Phase 1's solo-bandwidth constraints. After PR #33 the architecture (Hexagonal lite + `KernelBackend` port + autograd composability) would, *in principle*, support all three as additional adapters / extensions. The question is whether they ever leave the out-of-scope list.

**Options.**

1. **Keep them out of scope** — defer indefinitely; they're not the educational pitch.
2. **Pick one as a "Phase 5+" planned item** — typically sparse, since the Domain types already accept any axis-extent (sparse = special storage; the port already takes `DynamicTensor`).
3. **Open-call for contributors** — if Axis F establishes a CONTRIBUTING.md, signal that any of these would be welcomed as adapter contributions.

**Recommendation.** **Option 1 + revisit at Phase 5+ planning**. None of these are blocking the current educational mission. The architectural payoff (these are *possible* without Domain changes) is itself a teaching moment in tutorial 08, and that's enough for now.

---

## Recommendation summary

| Axis | Recommendation                                                         | Decision-by         | Status (2026-05-11)                              |
| ---- | ---------------------------------------------------------------------- | ------------------- | ----------------------------------------------- |
| A    | One high-quality launch post (option 2)                               | Maintainer; before `0.1.0` tag | reframed by ADR-0013 ("canonical reference"); launch-post scope grows accordingly |
| B    | Status quo + "which type when?" how-to doc                            | Next mop-up PR      | how-to shipped in PR #35                         |
| C    | Profile-driven 1-week perf investigation (option 2)                    | Phase 1.5+ slot     | unchanged; pending                               |
| D    | Defer C++23 to Phase 5+ when `std::linalg` lands broadly               | No action now       | confirmed by external-substrate research (`<linalg>` ships in 2028+) |
| E    | Hold ADR-0012 choice with a 2-week timebox at P3.M2                    | Phase 3 entry       | resolved by ADR-0014 (Dawn via vcpkg + gpu.cpp vendored) |
| F    | Light-touch CONTRIBUTING.md + CODE_OF_CONDUCT.md now (option 2)        | Next PR             | shipped in PR #35                                |
| G    | Keep ADR-0001 out-of-scope as is; revisit at Phase 5+                  | No action now       | reframed by ADR-0013 (canonical-reference posture narrows additions further) |
| H    | Python SDK (nanobind recommended); after Phase 3.M*.2 + Phase 4 + Phase 5 land | Phase 6 entry         | maintainer-flagged 2026-05-11 ("最後で良い"); sequenced behind dispatch wiring + release + std::linalg shim |

## What I'm asking for

For each axis, please pick **agree / disagree / different option**. Anywhere "different option" — let me write up the alternative as a candidate ADR. Anywhere "agree" — I open follow-up PRs.

I am especially interested in **A** (since publicising the project changes the kind of issues that arrive) and **F** (since onboarding even one contributor compounds), and would defer to the maintainer's instinct on **G**.

## References

- [Phase 1 retrospective](./2026-05-11_phase-1-retrospective.md)
- [Phase 2 + 2.5 retrospective](./2026-05-11_phase-2-and-2-5-retrospective.md)
- [Backend perf comparison (P2.5.M4 baseline)](./2026-05-11_backend-performance-comparison.md)
- [Phase 3 plan](../impl-plans/2026-05-11_phase-3-webgpu.md)
- [All ADRs](../arc42/09-decisions/)
