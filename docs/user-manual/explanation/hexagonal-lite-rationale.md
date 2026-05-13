---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Why "Hexagonal lite"?

> **Diátaxis quadrant**: Explanation. Why this architecture, with whose-idea-borrowed-where, and what `"lite"` means in practice. The hard rule is in [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md); the formal decision is in [ADR-0009](../../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).

## What "Hexagonal" means

Hexagonal Architecture is Alistair Cockburn's 2005 framing of a long-standing pattern: separate the **Domain** (the business / scientific logic) from **adapters** (the things that move data in and out). Adapters come in two flavours:

- **Driving adapters** — what *invokes* the Domain. UI, CLI, test harness, REPL, REST endpoint, your `main()`.
- **Driven adapters** — what the Domain *uses* to reach the outside world. Databases, file systems, GPU drivers, network calls.

Both adapter kinds plug into the Domain through **ports** — typed interfaces the Domain declares but does not implement. The Domain depends on ports; adapters depend on ports + Domain types; nothing else.

Two consequences fall out:

1. **Swap adapters without touching the Domain.** Replace the SQL adapter with an in-memory one for tests; replace the WebGPU adapter with a reference CPU one for portability.
2. **The Domain stays small and readable.** It's defined by what it *does*, not by which infrastructure it sits on.

## What "lite" means here

Production Hexagonal implementations often add:

- Command buses + event buses for decoupled adapter ↔ Domain message flow.
- Dependency injection containers wiring everything at startup.
- A pub-sub event log so adapters can observe Domain state changes without coupling.

For a teaching-first library, those layers add ceremony without payoff. `tensor` keeps the *classification discipline* (everything in §5 is Domain / DrivingAdapter / DrivenAdapter) without the message-bus / DI-container plumbing. That's "lite".

## Why this fits `tensor`

Three reasons made the choice obvious in PR #18 / ADR-0009:

1. **Three reasonable kernel backends** (`reference`, `eigen`, `webgpu`) want to coexist in one library. Hexagonal naturally puts them on the same port and reads them as siblings; the alternative — `#ifdef`-spaghetti around CUDA / vendor SDKs — is what production C++ tensor libraries do, and they read like configure-script archeology.
2. **The Domain *is* the teaching artifact.** Phase 6+ also adds a Python SDK as a *DrivingAdapter*, not a re-implementation in Python. Same Domain, second entry surface. Without Hexagonal-lite, the Python SDK would be a separate algebra implementation — twice the surface, twice the test cost, half the citability.
3. **It scales backwards.** A learner reading `tutorials/08_swappable-backends.ipynb` can see *exactly* which container does what, because §5 building-blocks ratifies the classification.

## What you actually do in practice

- Every container in [`../../arc42/05-building-blocks/overview.md`](../../arc42/05-building-blocks/overview.md) is labelled `Domain` / `DrivingAdapter` / `DrivenAdapter`. A reviewer who can't see the classification on a new container raises a flag in PR review.
- Ports are C++20 concepts under `tensor::core::concepts::*`. The one that earns the most traffic is `KernelBackend` (15 methods, [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md)).
- The dependency rule is enforced by `grep` (and a planned `tools/check-hexagon.sh` script) — see [`../../arc42/05-building-blocks/overview.md` §"Enforcement examples"](../../arc42/05-building-blocks/overview.md) for the exact commands.

## What "lite" does NOT exempt you from

- **Dependency rule is hard.** Domain depends on no adapter; adapters depend on Domain + the relevant port; no adapter depends on another adapter directly. CI enforces this with the `grep`-style commands above; PR review enforces it at the design level.
- **The classification is part of the PR.** A new container's row in §5 must include the hexagonal label. Reviewers who see "TBD" reject the PR until the contributor commits to a classification.

## How this differs from Onion / Clean

Onion Architecture (Jeffrey Palermo, 2008) and Clean Architecture (Robert Martin, 2012) are close cousins. They differ from Hexagonal in detail (Onion uses concentric circles; Clean adds use-case classes; Hexagonal stays at two adapter kinds). For an educational library this small, the differences are mostly notational. `tensor` chose Hexagonal because "ports and adapters" is the simplest set of nouns that maps onto C++20 concepts.

## Where this came from

- Alistair Cockburn, "Hexagonal architecture", 2005 — <https://alistair.cockburn.us/hexagonal-architecture/>.
- Eric Evans, *Domain-Driven Design* (2003) — the "ubiquitous language" half of [ADR-0009](../../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md) comes from here.
- Vaughn Vernon, *Implementing Domain-Driven Design* (2013) — for the implementation conventions.
- For the specific lite-without-bus interpretation: [ADR-0009 §Decision Outcome](../../arc42/09-decisions/0009-adopt-ddd-ubiquitous-language-and-hexagonal-lite.md).

## Where to go next

- The hard rule in operation: [`../../design-guide/architectural-discipline.md`](../../design-guide/architectural-discipline.md).
- The Domain centerpiece: [`../../detailed-design/tensor-core.md`](../../detailed-design/tensor-core.md).
- The port surface: [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md).
- Phase 6 (Python SDK as DrivingAdapter): [`../../detailed-design/python-sdk-binding-surface.md`](../../detailed-design/python-sdk-binding-surface.md).
