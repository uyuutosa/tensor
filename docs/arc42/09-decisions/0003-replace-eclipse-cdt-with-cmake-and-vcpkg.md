---
status: Accepted
owner: tensor
last-reviewed: 2026-05-10
---

# ADR-0003: Replace Eclipse CDT with CMake and vcpkg

| Metadata  | Value                                                          |
| --------- | -------------------------------------------------------------- |
| Status    | **Accepted**                                                   |
| Type      | ADR (Type 5 / MADR v3.0 / arc42 §9)                            |
| Date      | 2026-05-10                                                     |
| Deciders  | uyuutosa                                                       |
| Consulted | —                                                              |
| Informed  | future contributors                                            |
| Ticket    | —                                                              |

---

## Context and Problem Statement

The repository currently builds via Eclipse CDT (`.cproject`, `.project`, `.settings/`). Eclipse CDT is [still maintained](https://projects.eclipse.org/projects/tools.cdt) but is widely treated as legacy in 2025–2026 ([slant alternatives 2025](https://www.slant.co/options/2472/alternatives/~eclipse-cdt-alternatives)). Every modern C++ tensor / numerical library — Eigen, xtensor, Blaze, Armadillo, Kokkos, libtorch — uses CMake, with [vcpkg](https://vcpkg.io/) or [Conan](https://conan.io/) for dependency management.

A revival positioned as educational (ADR-0001) and rewritten on C++20 + `mdspan` (ADR-0002) cannot retain `.cproject` files: prospective contributors and learners would have to install Eclipse to build a hello-world. CI on GitHub Actions also assumes a CLI build path.

---

## Decision Drivers

- **DD-1**: Contributor / learner cost-of-entry. Two `git clone && cmake -S . -B build && cmake --build build` commands or no contributors.
- **DD-2**: CI portability. GitHub Actions matrices are CMake-shaped.
- **DD-3**: Dependency management for the Kokkos `mdspan` polyfill, [doctest](https://github.com/doctest/doctest) / [Catch2](https://github.com/catchorg/Catch2), [xeus-cling](https://github.com/jupyter-xeus/xeus-cling), and the future WebGPU / Dawn build (ADR-0006).
- **DD-4**: IDE-agnostic. CMake is consumed by VSCode, CLion, Qt Creator, Visual Studio, and `clangd` editors out of the box.

---

## Considered Options

1. **Keep Eclipse CDT.**
2. **CMake + vcpkg.**
3. **CMake + Conan.**
4. **Bazel.**
5. **Meson.**

---

## Decision Outcome

**Chosen option: 2 — CMake (≥ 3.25) + vcpkg as the primary dependency manager, with Conan support left for the community to add if anyone needs it.**

CMake is the genre default. vcpkg is chosen over Conan because (a) it ships pre-built ports of the dependencies this project will need (mdspan polyfill, doctest, Dawn/wgpu-native, xeus-cling adjacents) and (b) the [vcpkg manifest mode](https://learn.microsoft.com/en-us/vcpkg/users/manifests) integrates with CMake `find_package` cleanly. Bazel and Meson are both technically defensible but neither matches CMake's ecosystem reach for educational consumers.

### Y-statement summary

> In the context of **rebuilding the `tensor` library's build system from Eclipse CDT**, facing **the need for low-friction contributor onboarding and CI portability**, we decided for **CMake (≥ 3.25) with vcpkg manifest mode**, to achieve **the genre-default toolchain that learners already know**, accepting **the loss of Eclipse's GUI project model**.

---

## Pros and Cons of the Options

### Option 1: Keep Eclipse CDT

- Pros: zero migration cost.
- Cons: blocks every contributor without Eclipse; GitHub Actions integration is awkward; no dependency resolution; not idiomatic for any modern C++ project.

### Option 2: CMake + vcpkg (chosen)

- Pros: genre default; broad IDE support; vcpkg manifest pins dependencies reproducibly; vcpkg ports for Dawn/WebGPU exist.
- Cons: CMake's syntax has well-known sharp edges; vcpkg + Windows MSVC has occasional friction.

### Option 3: CMake + Conan

- Pros: Conan's package model is more flexible; Python-first tooling appeals to data-science adjacent users.
- Cons: smaller intersection with the C++ tensor library ecosystem; vcpkg has wider ports for the targeted deps.

### Option 4: Bazel

- Pros: hermetic, fast, used by big C++ projects.
- Cons: very steep learning curve for educational consumers; small overlap with our deps' published Bazel rules.

### Option 5: Meson

- Pros: cleaner syntax than CMake; popular in GNOME/Linux desktop space.
- Cons: smaller ecosystem in numerical C++; learners are statistically less likely to know it.

---

## Consequences

### Positive

- `git clone && cmake --preset=default && cmake --build --preset=default` builds the library and the test suite.
- `vcpkg.json` pins exact versions of the `mdspan` polyfill, `doctest`, and (later) Dawn/wgpu.
- CI on GitHub Actions becomes a 30-line matrix.
- VSCode / CLion / `clangd` work without configuration.

### Negative

- Eclipse CDT files (`.cproject`, `.project`, `.settings/`) must be removed or moved to `archive/`.
- Existing build instructions (none documented anyway) become void.
- vcpkg adds a one-time bootstrap step for new contributors.

### Neutral

- Bazel and Meson remain reachable as future additions if a contributor cares enough to write the rules.

### Follow-ups

- [ ] Add `CMakeLists.txt`, `CMakePresets.json`, `vcpkg.json` at repo root.
- [ ] Move `.cproject`, `.project`, `.settings/` to `archive/eclipse-cdt/` or delete in the rewrite branch.
- [ ] Add GitHub Actions workflow `ci.yml` with matrix {GCC 11, Clang 13, MSVC 19.30} × {Debug, Release}.
- [ ] Add `cmake-format` / `clang-format` / `clang-tidy` configurations.
- [ ] Add reproducibility hash via `cmake --preset=default -DTENSOR_LOCK_DEPS=ON`.

---

## Compliance / Validation

- Verification: CI passes only if `cmake --preset=default && ctest --preset=default` succeeds.
- Frequency: every PR.

---

## More Information

### Related ADRs

- Predecessors: [ADR-0001](0001-pivot-to-educational-named-axis-dsl.md), [ADR-0002](0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md)
- Distribution medium: [ADR-0008](0008-distribute-as-header-only-with-jupyter-tutorials.md)

### References

- [vcpkg manifest mode docs](https://learn.microsoft.com/en-us/vcpkg/users/manifests)
- [CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
- [Eclipse CDT alternatives 2025 (Slant)](https://www.slant.co/options/2472/alternatives/~eclipse-cdt-alternatives)
