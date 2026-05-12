---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0002: Rewrite the `tensor` library on a C++20/23 baseline with `std::mdspan` / `std::linalg` interop

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | Claude (research brief)                                        |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The current codebase predates C++17. It uses `new` for owning allocations, raw references for indirection, no concepts, no `constexpr` shape arithmetic, and Eclipse CDT for builds. Modern tensor libraries ([Eigen 5](https://libeigen.gitlab.io/releases/5.0/), [Kokkos 5](https://hpsf.io/blog/2025/kokkos-5-0-embracing-c20-and-celebrating-a-milestone-era-in-hpc/), [xtensor 0.27](https://github.com/xtensor-stack/xtensor)) are converging on C++20, with `mdspan` interop becoming the *de facto* lingua franca for cross-library data exchange.

C++23 introduced [`std::mdspan`](https://en.cppreference.com/w/cpp/container/mdspan); C++26 will ship [`std::linalg` (P1673R13)](https://isocpp.org/files/papers/P1673R13.html), standardizing BLAS-flavored free functions over multidim views. Per ADR-0001, this project has pivoted to educational positioning — and a teaching library that ignores the modern standard substrate teaches obsolete idioms.

We must decide the C++ baseline and the relationship to `mdspan` / `std::linalg` before the rewrite begins.

---

## Decision Drivers

- **DD-1**: Pedagogical alignment with current C++. A teaching artifact must demonstrate *idiomatic 2026* C++, not 2014 idioms.
- **DD-2**: Interop. `mdspan` is becoming the universal multidim view; refusing it isolates the library.
- **DD-3**: Toolchain availability. C++20 is broadly supported (GCC ≥ 11, Clang ≥ 13, MSVC 19.30+); C++23 `mdspan` is in libstdc++ 13 and libc++ 17, with [a reference impl](https://github.com/kokkos/mdspan) for older toolchains.
- **DD-4**: Maintenance. Modern C++ removes large categories of legacy code (`new`/`delete`, manual SFINAE) that bloat the existing codebase.

---

## Considered Options

1. **C++17 baseline, no `mdspan` interop** — minimal modernization, broadest compiler support.
2. **C++20 baseline, optional `mdspan` interop via [Kokkos reference impl](https://github.com/kokkos/mdspan)** — concepts + NTTPs + ranges, with `mdspan` as a first-class interop type even on toolchains that pre-date C++23.
3. **C++23 baseline, `mdspan` from `std`** — strict, requires very recent toolchains.
4. **C++26 baseline** — premature; standard not yet finalized at decision time.

---

## Decision Outcome

**Chosen option: 2 — C++20 baseline with first-class `mdspan` interop via the Kokkos reference impl on older toolchains and `std::mdspan` on toolchains that ship it.**

C++20 unlocks the modern idioms the rewrite needs (concepts, NTTPs, ranges, `consteval`, requires-clauses, [deducing-this](https://en.cppreference.com/cpp/26) backported via constructors), with sufficiently broad compiler support for an educational artifact. `mdspan` interop is non-negotiable for DD-2 — the [Kokkos reference implementation](https://github.com/kokkos/mdspan) backfills it on C++20 and is shipped as a header-only library, eliminating the trade-off. `std::linalg` will be additively integrated when C++26 toolchains land.

### Y-statement summary

> In the context of **a from-scratch rewrite of the `tensor` C++ library positioned as educational**, facing **the choice of language baseline and the relationship to `std::mdspan` / `std::linalg`**, we decided for **C++20 + Kokkos `mdspan` reference impl, with `std::mdspan` and `std::linalg` consumed additively as toolchains catch up**, to achieve **modern idioms, broad interop, and a teaching surface that demonstrates 2026-era C++**, accepting **that pre-C++20 toolchains are unsupported**.

---

## Pros and Cons of the Options

### Option 1: C++17, no `mdspan`

- Pros:
  - Largest compiler matrix.
- Cons:
  - No concepts → ugly SFINAE.
  - No NTTP string literals → no compile-time named-axis ergonomics.
  - Refusing `mdspan` interop isolates the library from every modern peer.
  - Teaches yesterday's idioms.

### Option 2: C++20 + Kokkos `mdspan` (chosen)

- Pros:
  - Concepts, NTTPs, ranges, `consteval`, modules-eligible.
  - `mdspan` interop available everywhere via reference impl.
  - GCC 11 / Clang 13 / MSVC 19.30 satisfy this baseline; `xeus-cling` (Jupyter) supports C++20.
- Cons:
  - Carries the Kokkos `mdspan` polyfill as a header dependency until `std::mdspan` is universal.

### Option 3: C++23, `std::mdspan` from `std`

- Pros:
  - Strictly modern, no polyfill.
- Cons:
  - Excludes too many real-world toolchains in 2026 (especially distribution-shipped GCC/Clang on stable Linux releases).
  - Educational reach narrowed.

### Option 4: C++26

- Pros:
  - Includes `std::linalg`.
- Cons:
  - Standard not finalized; toolchains essentially absent at decision time.

---

## Consequences

### Positive

- The rewrite uses concepts (cleaner constraints), NTTPs (compile-time named axes — see ADR-0004), `consteval` shape arithmetic, and ranges-style composition.
- `mdspan` interop means data can be shared zero-copy with Kokkos, xtensor, future `std::linalg` consumers, NumPy via Python bindings, and Jupyter notebooks.
- The legacy `new`/raw-ref code is replaced wholesale; the educational story explicitly contrasts the 2016 pre-C++17 implementation with the 2026 rewrite.

### Negative

- Pre-C++20 toolchains (GCC 10, Clang 12, MSVC 19.28) are unsupported. CI must enforce.
- The Kokkos `mdspan` polyfill adds a build-time dependency until `std::mdspan` is universal.

### Neutral

- `std::linalg` integration is deferred to C++26 toolchain availability; the API contract should anticipate it (e.g. expose `std::linalg`-shaped views) without requiring it.

### Follow-ups

- [ ] Build system + dependency management → ADR-0003.
- [ ] CI matrix: GCC 11+, Clang 13+, MSVC 19.30+ (later: drop polyfill when `std::mdspan` is the floor).
- [ ] Document the C++ standard transition in `docs/design-guide/`.

---

## Compliance / Validation

- Verification: CMake `target_compile_features(tensor PUBLIC cxx_std_20)`; CI runs all three compilers at the minimum versions.
- Frequency: every PR.

---

## More Information

### Related ADRs

- Predecessor: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md)
- Build system: [ADR-0003](0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md)
- Named-axis API: [ADR-0004](0004-adopt-hybrid-named-axis-api.md)

### References

- [Kokkos `mdspan` reference implementation](https://github.com/kokkos/mdspan)
- [P1673R13 — std::linalg](https://isocpp.org/files/papers/P1673R13.html)
- [Herb Sutter Hagenberg 2025 trip report (C++26 freeze)](https://herbsutter.com/2025/02/17/trip-report-february-2025-iso-c-standards-meeting-hagenberg-austria/)
- [cppreference C++20](https://en.cppreference.com/w/cpp/20)
