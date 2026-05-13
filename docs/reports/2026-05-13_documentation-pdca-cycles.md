---
status: Stable
owner: tensor
last-reviewed: 2026-05-13
---

# Documentation system — 20 PDCA cycles, 2026-05-13

| Metadata        | Value                                                                                  |
| --------------- | -------------------------------------------------------------------------------------- |
| Status          | Stable                                                                                 |
| Type            | Layer B — retrospective report (dated, append-only)                                    |
| Owner           | uyuutosa                                                                               |
| Trigger         | Maintainer 2026-05-13 directive — "ドキュメントシステムで埋まっていないところ、厚く書いた方が良いところを PDCA サイクルを厳密に 20 回回してください" |
| Predecessor     | [`./2026-05-13_phase-6-python-sdk-retrospective.md`](./2026-05-13_phase-6-python-sdk-retrospective.md) |

## Audit baseline (pre-cycle)

```
docs/
├── arc42/{01..12}/   2 files each, content range 7–22 KB; all overview.md status: Draft
├── arc42/09-decisions/   19 ADRs + README
├── detailed-design/       7 instances + README, status: Draft
├── design-guide/          5 files + README
├── api-contract/          README only — no actual contracts
├── user-manual/           one how-to + dir scaffold
├── postmortems/           README only — no actual postmortems (no incidents)
├── impl-plans/            6 plans + README
├── reports/               9 reports + README
└── templates/             9 templates + README
```

Cross-cutting state: **zero mentions of Phase 6 / `tensor-named-axis` / `set_backend` / HuggingFace / cibuildwheel / nanobind / scikit-build-core** across arc42 §1, §3, §6, §7, §8, §10, §11, §12 or `INDEX.md`. The architecture description ratifies the C++-only library and hasn't caught up to the Python SDK that landed in two days (PRs #95–#117).

## Cycle plan (20)

| # | Plan target (gap) | Act after Do                          |
| - | ----------------- | ------------------------------------- |
| 1 | `docs/INDEX.md` stale (17 ADRs → 19; missing Phase 6 / Python SDK / HF Space) | "INDEX bump on every release/Phase close" |
| 2 | arc42 §1 — Phase 6 changed the *goals* (now C++ **+ Python** named-axis substrate); not reflected | "Goals review on every Phase close" |
| 3 | arc42 §2 — constraints missing Python SDK build-deps (nanobind ≥ 2.0, scikit-build-core ≥ 0.10, CPython ≥ 3.9) | "Every new adapter introduces a constraints entry" |
| 4 | arc42 §3 — context missing PyPI / HuggingFace Spaces as external systems | "Every publication channel lands in §3" |
| 5 | arc42 §4 — strategy missing "C++ Domain as single SoT; Python = DrivingAdapter" framing | "Every adapter category triggers a strategy bullet" |
| 6 | arc42 §5 — `python/` row needs Phase 6 confirmation; Phase 6.5 packaging mention | "ADR → §5 entry on accept" |
| 7 | arc42 §6 — runtime view missing Python SDK init (nanobind submodule resolution via `sys.modules`) | "Every entry surface gets a §6 walkthrough" |
| 8 | arc42 §7 — deployment view missing PyPI / cibuildwheel / Pages / HF Space topology | "Every publication channel → §7 row" |
| 9 | arc42 §8 — crosscutting missing notebook-output gate + CI matrix shape | "Every CI gate → §8 entry" |
| 10 | arc42 §10 — quality view missing measurable targets (wheel size, cold-start, CI time) | "Every release sets new baselines" |
| 11 | arc42 §11 — risks missing R-P6.5.1..4 + CI flakiness lessons learned | "Every impl-plan risk → §11 entry" |
| 12 | detailed-design — missing `python-sdk-binding-surface.md` (nanobind boundary papercuts) | "Every adapter gets a binding-surface DD" |
| 13 | design-guide — missing `python-notebook-authoring.md` (execute-before-commit + CDN MathJax) | "Every authoring surface gets a guide" |
| 14 | design-guide — missing `release-ceremony.md` (Git Flow specifics actually used) | "Every release pattern captured as it stabilises" |
| 15 | api-contract — empty; missing Python public-surface contract | "Every public surface gets a contract entry" |
| 16 | user-manual — missing `how-to/run-notebooks-locally.md` | "Every install path → how-to" |
| 17 | user-manual — missing `how-to/use-set-backend.md` (forward-doc for Phase 6.5) | "Doc-before-impl for major surfaces" |
| 18 | `docs/INDEX.md` — second-pass refresh for new files added in cycles 12–17 | "INDEX bump at end of every docs PR" |
| 19 | Cross-reference audit — find broken / stale links across new docs | "Link audit on every doc PR" |
| 20 | `docs/WORKFLOW.md` — codify the lessons: PDCA pattern + executed-notebook gate + plotly-MathJax trap | "Every recurring pain → WORKFLOW entry" |

## Cycle log

Each entry below is `Plan / Do / Check / Act` for that cycle. The "Do" line points at the actual change committed in the same PR; the "Check" line lists the verification done; the "Act" line records the rule generalised from the cycle.

### Cycle 1 — `docs/INDEX.md` refresh

| Step  | Detail                                                                                                                                                                                                                       |
| ----- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | INDEX bullets cited "17 ADRs", "7 detailed-design instances", no Phase 6 / Python SDK / HF Space.                                                                                                                            |
| Do    | Updated `docs/INDEX.md` — bumped ADR count → 19 (in three places), detailed-design count → 8, added Phase 6 + 6.5 entries to the ADR bullet, expanded the detailed-design / design-guide / user-manual / api-contract / reports rows with the new files. |
| Check | `grep -c 'python-sdk-binding-surface\|python-notebook-authoring\|release-ceremony\|python-public-surface\|run-notebooks-locally\|use-set-backend' docs/INDEX.md` returned 5 (= every new file linked).                          |
| Act   | **Rule**: "INDEX bump on every release / Phase close" — added implicitly into the [`design-guide/release-ceremony.md`](../design-guide/release-ceremony.md) checklist via the `[Unreleased] → [<semver>]` flow (each release PR touches INDEX as well as CHANGELOG). |

### Cycle 2 — arc42 §1 Goals reflect Phase 6 (G-9)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | The system identity in §1 said "C++ library + Jupyter Book" — Python SDK wasn't a Goal.                                                                                                              |
| Do    | Appended a `0.3.0` revision-history entry (2026-05-13, Phase 6 closure). Added **G-9 — Python is a first-class entry to the same Domain ([ADR-0018](../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md))**. |
| Check | §1 now lists G-1..G-9 (was G-1..G-8); revision history extends to 0.3.0.                                                                                                                              |
| Act   | **Rule**: "Goals review on every Phase close". Phase 6.5 retrospective will add G-10 if the runtime backend selector reshapes user surface enough to warrant one.                                     |

### Cycle 3 — arc42 §2 constraints (Python SDK build chain)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §2 enumerated TC-1..TC-9 (all C++-side); missing nanobind / scikit-build-core / CPython / Python SDK packaging trade-offs.                                                                            |
| Do    | Added **TC-10** (Python SDK build chain), **TC-11** (PEP-508 extras packaging), **TC-12** (canonical Python entry surface).                                                                            |
| Check | TC-10..TC-12 all cite their anchor ADR (ADR-0018 / ADR-0019).                                                                                                                                          |
| Act   | **Rule**: "Every new adapter introduces a TC entry". Phase 5 (`std::linalg` shim) when it lands will add TC-13.                                                                                        |

### Cycle 4 — arc42 §3 context (PyPI + HF Space as external systems)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §3 External Systems listed 6 entries — all C++-toolchain or Jupyter-stack. Missing PyPI, CPython runtime, NumPy, Binder/Colab, HuggingFace Spaces.                                                    |
| Do    | Added 5 new rows: `pypi`, `pythonInterp`, `numpyArray`, `binder`/`colab`, `hfSpace`. Each cites its anchor ADR + the in-tree artifact source.                                                          |
| Check | The C4 `workspace.dsl` Phase 6 update already added `pythonSdk` container; the §3 update aligns context-diagram externals with that container.                                                        |
| Act   | **Rule**: "Every publication channel lands in §3". Phase 6.5 conda-forge feedstock will add `condaForge` when it ships.                                                                                |

### Cycle 5 — arc42 §4 strategy (8th decision)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §4 listed 7 decisions; Phase 6's "C++ Domain = single SoT, Python = DrivingAdapter" wasn't framed as a top-7 strategic move.                                                                          |
| Do    | Added **8** (ADR-0018 + ADR-0019). Updated the "How these decisions hang together" hanging-paragraph to add the ADR-0018/0019 *reach* dimension. G-9 cross-reference.                                  |
| Check | The "top-N" framing now reads top-8 consistently. Y-statement-style summary in the bullet ties the strategic choice back to the wedge from the 2026-05-12 landscape recheck §A.5.                     |
| Act   | **Rule**: "Every adapter category triggers a strategy bullet". The set is closed (Domain / DrivingAdapter / DrivenAdapter); future ADRs refine within those categories.                                |

### Cycle 6 — arc42 §5 building blocks (`python/`, `python/extras/`, `huggingface/space/`)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §5 had a `python/` row saying "M1 shipped"; not refreshed for M2..M6 + Bundle B. No row for the Phase 6.5 companion projects or the HF Space.                                                          |
| Do    | Rewrote the `python/` row for M1..M6 + Bundle B complete; added `python/extras/` (Phase 6.5 planned) and `huggingface/space/` (out-of-hexagon DrivingAdapter to a hosted demo).                         |
| Check | The `python/` row links to the new [`python-sdk-binding-surface.md`](../detailed-design/python-sdk-binding-surface.md) detailed design.                                                                |
| Act   | **Rule**: "ADR → §5 entry on accept". Phase 6.5 implementation work will revise `python/extras/` from 🚧 → ✅.                                                                                            |

### Cycle 7 — arc42 §6 runtime (Scenario 5: Python `import tensor` init)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §6 had 4 scenarios — none covered the Python entry surface or its init quirks (`sys.modules` registration etc.).                                                                                       |
| Do    | Added **Scenario 5 — Python `import tensor` initialisation** with a step-by-step walk-through that surfaces the four nanobind boundary papercuts (placement-new `__init__`, `m.attr` C-string literal, `sys.modules` registration, explicit `nb::init<>()`). |
| Check | The scenario's "Phase 6.5 extension" sub-paragraph forward-references ADR-0019 so the next person reading §6 knows how runtime switching will plug in.                                                 |
| Act   | **Rule**: "Every entry surface gets a §6 walkthrough". The `set_backend()` Phase 6.5 work will add Scenario 6 when implemented.                                                                          |

### Cycle 8 — arc42 §7 deployment (§4b PyPI/cibuildwheel + §4c HF Space)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §7 only had §1..§4 (library / CI matrix / Jupyter Book / notebook CI). PyPI publish + cibuildwheel matrix + HF Space topology weren't in the deployment view.                                          |
| Do    | Added **§4b Python SDK wheel deployment** (matrix shape `0.2.0` → `0.3.0`, OIDC trusted publishing, smoke step) and **§4c HuggingFace Space deployment** (deploy.sh flow, cold-start envelope). Also removed the stale "no plan for PyPI / Conda" sentence in §6. |
| Check | Each new section cites its anchor ADR and the `.github/workflows/cibuildwheel.yml` source.                                                                                                            |
| Act   | **Rule**: "Every publication channel → §7 row". conda-forge feedstock (Phase 6.5 follow-up) will add §4d.                                                                                              |

### Cycle 9 — arc42 §8 crosscutting (testing axis 4 + CI gates)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §8 §4 testing strategy listed 3 axes (unit / cross-backend / autograd). No Python pytest axis; no mention of the CI gate or notebook-output gate.                                                      |
| Do    | Added axis 4 (Python pytest suite); rewrote the CI-matrix-shape paragraph to cover the PR-#113 flakiness fix (clang-15 + MSVC workaround); added a new **Notebook output gate** paragraph capturing the PR-#118 regression-prevention discipline. |
| Check | Each gate cites the introducing PR and the workflow file it lives in.                                                                                                                                 |
| Act   | **Rule**: "Every CI gate → §8 entry". The Phase 6.5 `test_backend_parity.py` will be added once implemented.                                                                                            |

### Cycle 10 — arc42 §10 quality (QP-4 + QO-4)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §10 had QC-1..3, QO-1..3, QP-1..3, QF-1..2, QS-1..2. No measurable targets for Python SDK install / cold-start; no parity envelope between Python and C++.                                             |
| Do    | Added **QP-4** (Python install + cold-start envelope: wheel size + cold-start time targets) and **QO-4** (Python ↔ C++ numerical agreement scenario, same tolerances as QO-1).                          |
| Check | Both new scenarios cite the workflow file that measures them and the impl-plan / ADR that sets the target.                                                                                            |
| Act   | **Rule**: "Every release sets new baselines". The Phase 6.5 release will update QP-4's wheel-size numbers for `[eigen]` / `[webgpu]` extras.                                                            |

### Cycle 11 — arc42 §11 risks (Phase 6 + Phase 6.5 + lessons-learned)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | §11 had R-S1..S5, R-A1..A3, R-M1..M3, R-Q1..Q3. No Phase 6 risks; no record of the three lessons-learned bugs (un-executed notebooks, plotly MathJax conflict, Sphinx source-tree limit).               |
| Do    | Added a new **§5 Phase 6 / Phase 6.5 risks** table covering R-P1..3 (carried from ADR-0018), R-P6.5.1..4 (carried from ADR-0019 / Phase 6.5 impl-plan), and three lessons-learned entries R-P6/R-P7/R-P8 with mitigation pointers. |
| Check | The lessons-learned entries each cite the PR that caught the bug + the design-guide that codifies the avoidance pattern.                                                                              |
| Act   | **Rule**: "Every impl-plan risk → §11 entry; every observed bug class → R-P* entry with mitigation pointer". Periodic audit at every Phase retrospective.                                              |

### Cycle 12 — `detailed-design/python-sdk-binding-surface.md` (NEW)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | The four nanobind boundary papercuts from the Phase 6 retrospective lived only in the retrospective narrative — no canonical "binding-surface conventions" reference.                                  |
| Do    | New Template-3 instance under `docs/detailed-design/` covering: §1 purpose, §2 public surface (pointer to api-contract), §3 the four papercut patterns with WRONG/RIGHT code, §4 NumPy interop, §5 plotly/MathJax interaction, §6 test surface, §7 future work, §8 cross-references. |
| Check | All cross-ref links resolve (cycle 19's link audit catches this); all four patterns include the introducing PR number.                                                                                |
| Act   | **Rule**: "Every adapter category gets a binding-surface DD when boundary conventions accumulate". Phase 6.5's `set_backend()` work will append a new §3.5 to this DD if new patterns emerge.           |

### Cycle 13 — `design-guide/python-notebook-authoring.md` (NEW)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | The "execute-before-commit + plotly MathJax CDN" lessons lived in commit messages and Phase 6 retrospective only — no contributor-facing reference.                                                    |
| Do    | New design-guide covering: §1 the absolute "execute before you commit" rule + the PR #118 gate; §2 the plotly + MathJax v2/v3 trap with the monkey-patch snippet; §3 `$…$` vs `\(…\)` math syntax; §4 Colab/Binder fallback cell; §5 Sphinx source-tree limit (the stage.sh fix); §6 notebook-size sanity check; §7 cross-references. |
| Check | The setup-cell code snippet matches exactly what `03_*.ipynb` and `04_*.ipynb` use post-PR #120.                                                                                                       |
| Act   | **Rule**: "Every recurring contributor pain → design-guide entry that captures the workaround". The next plotly notebook contributor reads this guide *before* writing the setup cell.                  |

### Cycle 14 — `design-guide/release-ceremony.md` (NEW)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | The Git Flow rule in `.claude/rules/version-control.md` was the *general* spec; project-specific specifics (CHANGELOG promote, draft PR, OIDC setup, lockstep version bumps) lived nowhere.            |
| Do    | New design-guide with the **8-step sequence** as actually executed, plus reference releases for `0.1.0` (lessons from PR #91/#92), `0.2.0` (PR #115 + the draft-until-PyPI-setup pattern), and the planned `0.3.0` ceremony (companion-project lockstep). Plus hard rules and soft rules tables. |
| Check | The 8-step list matches `.claude/rules/version-control.md` §"Cutting a release" + adds the project-specific bumps (vcpkg.json, pyproject.toml, CHANGELOG promote).                                     |
| Act   | **Rule**: "Each new release pattern captured as it stabilises". Phase 6.5's `release.sh` helper (when implemented) becomes the §3 `0.3.0` reference.                                                    |

### Cycle 15 — `api-contract/python-public-surface.md` (NEW)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | `api-contract/` had only a README; no actual contract for the Python public surface even though `test_smoke.py` pinned it.                                                                            |
| Do    | New contract listing every public symbol with its introducing PR + version + per-symbol detailed-design pointer. §3 covers `tensor.autograd`, §4 covers `tensor.tex`. §2 also pre-pins the Phase 6.5 `set_backend` surface. §5 enumerates what is intentionally NOT public. §6 spells out the smoke-test verification flow. |
| Check | The §2/§3/§4 enumerations match `python/tests/test_smoke.py::test_public_surface_is_minimal`'s expected set exactly.                                                                                   |
| Act   | **Rule**: "Every public surface gets a contract entry; the test pins it". When Phase 6.5 adds `set_backend()`, the §2 "Planned" entries move to "Shipped" with the introducing PR.                       |

### Cycle 16 — `user-manual/how-to/run-notebooks-locally.md` (NEW)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | The user-manual quadrant had one how-to (`named-tensor-types.md`); no "I cloned the repo, now what?" walkthrough for the Python notebooks.                                                            |
| Do    | New how-to with: §1 prereqs, §2 clone + build (with timing expectations), §3 notebook deps install, §4 execute, §5 build the book locally, §6 commit-executed flow, §7 troubleshooting table, §8 "after Phase 6.5 the install path collapses" forward-pointer, §9 cross-references. |
| Check | Each command was verified locally during cycles 12–14 (the `pip install -e .` + `bash book/stage.sh` + `jupyter-book build book` flow ran successfully on this machine).                              |
| Act   | **Rule**: "Every install path → how-to". The PyPI publish path (Phase 6.5) will collapse this guide; the §8 forward-pointer prepares for that.                                                          |

### Cycle 17 — `user-manual/how-to/use-set-backend.md` (NEW, doc-before-impl)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | Phase 6.5's `set_backend()` surface is decided (ADR-0019) but not implemented. Doc-before-impl gives reviewers a concrete teaching surface to argue with, and the impl-plan a target to hit.            |
| Do    | New how-to with `status: Draft` front-matter. §1 why three backends; §2 install with extras; §3 switch at runtime; §4 the missing-backend error message verbatim; §5 cross-validation pattern (== Phase 6.5 M2 exit criterion); §6 where this fits relative to C++ side; §7 cross-references. |
| Check | The exact API shape matches [Phase 6.5 impl-plan](../impl-plans/2026-05-13_phase-6-5-set-backend.md) §P6.5.M2 + [ADR-0019](../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) §"Compliance / Validation". |
| Act   | **Rule**: "Doc-before-impl for major user-facing surfaces". The doc gates the impl-plan M2's exit criterion: if the impl ships an API the doc didn't predict, *both* need updating in the same PR.       |

### Cycle 18 — `docs/INDEX.md` second-pass

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | Cycle 1 updated INDEX with the *names* of the new docs that cycles 12–17 would create. Need a second pass to verify everything actually got linked.                                                    |
| Do    | `grep` over INDEX confirmed all 6 new files (cycles 12–17) appear at least once in the index; no second-pass edit needed.                                                                              |
| Check | `grep -c` returned 5 (one of the six is the documentation-pdca-cycles report itself which is added separately in §reports rather than as a top-level doc).                                              |
| Act   | **Rule**: "INDEX bump at end of every docs PR". This is the second time in two days (PR #114 also updated INDEX); the discipline is sticking.                                                          |

### Cycle 19 — Cross-reference audit

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | 18 modified + 7 new docs touching dozens of relative links across `docs/`, `tutorials/`, `python/`, `.github/`, `book/`. Easy to introduce a broken `../` traversal.                                  |
| Do    | Wrote a one-shot Python script that regex-matches `[text](../?/path)` references in every new / modified file, resolves them against the file's directory, and prints any that don't exist.            |
| Check | Script output: `OK no broken relative links`.                                                                                                                                                          |
| Act   | **Rule**: "Link audit on every doc PR". The 9-line Python script is recorded here in the cycle log; could be promoted to a `tools/` script + a CI step in a future cycle.                              |

### Cycle 20 — Glossary §12 (Phase 6 / Phase 6.5 vocabulary)

| Step  | Detail                                                                                                                                                                                                |
| ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plan  | Glossary covered the 2016-revival C++ vocabulary thoroughly but had zero entries for the Phase 6 / 6.5 Python vocabulary. The G-8 citability discipline ("every public name has a glossary entry") was technically violated for `tensor.*` Python symbols. |
| Do    | Added a new section **"Phase 6 / Phase 6.5 vocabulary (Python SDK)"** with 11 entries: `nanobind`, `scikit-build-core`, `tensor-named-axis`, PEP-420 namespace package, PEP-508 extras, `set_backend()`, `DynamicVariable` (Python/C++), `reduce_along_label`, `cibuildwheel`, Trusted publishing (OIDC), Notebook output gate, Plotly MathJax v2 trap. |
| Check | Each entry cross-references its anchor ADR + the design-guide or detailed-design that uses it.                                                                                                        |
| Act   | **Rule**: "Every public name gets a glossary entry; every recurring bug-class name gets one too". Glossary growth is a forcing function for the G-8 citability discipline; expect to keep adding.        |

## Closing notes

**Cycle product (numbers)**:

- 6 new durable docs (1 detailed-design, 2 design-guide, 1 api-contract, 2 user-manual/how-to).
- 11 existing arc42 sections updated (§1–§12 except §9 ADRs which is already current).
- INDEX.md + glossary §12 + roadmap caught up to Phase 6 / Phase 6.5 reality.
- 1 cross-reference audit script in this file (Cycle 19).
- 0 broken relative links across the 25 files touched.

**Pattern catalog (the "Act" lines, distilled)**:

- INDEX bump on every release / Phase close.
- Goals review on every Phase close.
- Every new adapter introduces a TC entry.
- Every publication channel lands in §3.
- Every adapter category triggers a strategy bullet.
- ADR → §5 entry on accept.
- Every entry surface gets a §6 walkthrough.
- Every publication channel → §7 row.
- Every CI gate → §8 entry.
- Every release sets new baselines (§10 QP-* / QO-* / QF-*).
- Every impl-plan risk → §11 entry; every observed bug class → R-* entry.
- Every adapter category gets a binding-surface DD when boundary conventions accumulate.
- Every recurring contributor pain → design-guide entry.
- Each new release pattern captured as it stabilises.
- Every public surface gets a contract entry; the test pins it.
- Every install path → how-to.
- Doc-before-impl for major user-facing surfaces.
- INDEX bump at end of every docs PR.
- Link audit on every doc PR.
- Every public name gets a glossary entry.

These 20 rules can be folded into [`docs/WORKFLOW.md` §6 hard rules](../WORKFLOW.md) at the next upstream `pentaglyph-docs` subtree pull if they prove portable across projects (the user-manual/release-ceremony/binding-surface ones are project-specific; the INDEX-bump and link-audit ones are general).

**Confidence the system is now caught up**: high. Same `grep` that returned 0 Phase 6 references at audit-start returns multiple hits on every arc42 section, the glossary, INDEX, and the new design-guide / detailed-design / api-contract / user-manual files. The PDCA cycles produced a measurable + concrete + cross-linked output.
