# archive/

Read-only history. Material under this directory is preserved for reference but is **not** part of the build, the test suite, or the public API. None of it should be edited; if a fact in here is wrong, fix it in the live tree, not here.

## Contents

| Subdirectory          | Origin                                                                                  | Why it is preserved                                                                                       |
| --------------------- | --------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| `eclipse-cdt/`        | The Eclipse CDT project files (`.cproject`, `.project`, `.settings/`) used until 2026-05 | Reference for the original IDE setup; superseded by [CMake + vcpkg per ADR-0003](../docs/arc42/09-decisions/0003-replace-eclipse-cdt-with-cmake-and-vcpkg.md). |
| `legacy-2016/Tensor/` | The pre-C++17, hand-written 2016 tensor implementation                                  | Pedagogical reference for the rewrite; the new implementation under `include/tensor/` is *not* a port — see [ADR-0001](../docs/arc42/09-decisions/0001-pivot-to-educational-named-axis-dsl.md) and [ADR-0002](../docs/arc42/09-decisions/0002-rewrite-on-cpp20-baseline-with-mdspan-interop.md). |
| `legacy-2016/samples/` | The 2016 sample programs (Marquardt, ConvolveWithTensor, Optimizer)                    | Demonstrate what the original API was used for. The intro tutorial (`tutorials/00_intro.ipynb`) reproduces the convolution sample on the new API. |

## Why a rewrite, not a refactor

The 2016 codebase predates C++17. It uses raw `new`/`delete` for owning storage, returns owning references (`Tensor<T>&` from heap-allocated objects), has no concepts, no `constexpr` shape arithmetic, and assumes Eclipse CDT for builds. Modern C++ idioms have moved decisively past every one of those choices. Trying to incrementally refactor the legacy code would mean fighting the existing structure on every step; a clean rewrite under `include/tensor/` is faster *and* a better teaching artifact.

The legacy code stays here as:

1. **Historical record.** Some of the design ideas (named axes, function tensors, reference tensors, convolution-as-tensor-inner-product) originated here.
2. **Specification by example.** When in doubt about what the rewrite should *do*, the legacy code is the closest thing to a reference implementation.
3. **Pedagogical contrast.** The Jupyter Book site can show side-by-side excerpts: "this is the 2016 way; this is the 2026 way".

## Editing rules

- **Do not modify** anything under `archive/`. If a piece of code or config is needed in the live tree, copy and modernize it there.
- **Do not extend** subdirectories under `archive/`. Add new archives only when something graduates out of the live tree wholesale (with an ADR explaining why).
- **Do not link** the live `tensor` library or test suite against headers under `archive/`. The build excludes it explicitly.
