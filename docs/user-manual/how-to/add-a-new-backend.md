---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# How to add a new `KernelBackend` adapter

> **Diátaxis quadrant**: How-to. Task-oriented; assumes the reader knows what a `KernelBackend` is. For the conceptual frame, see [`../explanation/hexagonal-lite-rationale.md`](../explanation/hexagonal-lite-rationale.md). For the port details, see [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md).

This guide walks through adding a fourth `tensor::core::backend::*::Backend` (e.g. a hypothetical `tensor::core::backend::cuda::Backend`, even though CUDA-direct is disqualified per ADR-0006 — substitute your own substrate). The steps are the same regardless of substrate.

## Prerequisites

- Read [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md) §4 — the 15 methods.
- Read at least one existing adapter as a model — `tensor::core::backend::reference::Backend` is the canonical one to copy.
- Have a working C++20 build of the project (`cmake --preset=default && cmake --build --preset=default`).

## Step 1 — choose a substrate, write an ADR

Don't skip the ADR. If your substrate is new (not Eigen / WebGPU / `std::linalg`), file a new ADR `arc42/09-decisions/NNNN-<substrate-name>.md` that:

- Names the substrate (`cuda` / `vulkan` / `cuTENSOR` / `LibTorch` / whatever).
- Lists the strong alternatives considered + why rejected.
- Includes the Y-statement.
- Cross-refs [ADR-0006](../../arc42/09-decisions/0006-adopt-webgpu-as-gpu-backend.md) (which currently rejects CUDA-direct) — note explicitly whether your new ADR refines or contradicts ADR-0006.

The ADR is required, not optional — every backend has one.

## Step 2 — implement the adapter

Create the new directory:

```
include/tensor/core/backend/cuda/      # mirror the directory shape of reference/, eigen/, webgpu/
├── backend.hpp                         # the Backend struct
├── ops/                                # element-wise + contract + broadcast + reduce implementations
└── (CMake hookup in the parent CMakeLists.txt)
```

Implement each of the 15 methods in `tensor::core::concepts::KernelBackend`. The signatures are fixed (per ADR-0011); the implementations are yours.

**Convention**: delegate out-of-scope methods to a `reference::Backend` member, as `eigen::Backend` and `webgpu::Backend` already do:

```cpp
namespace tensor::core::backend::cuda {

struct Backend {
    reference::Backend ref_;   // for delegating methods you don't implement yourself

    void add(/* ... */) {
        // your CUDA-specific implementation
    }

    void contract(/* ... */) {
        // your code OR ref_.contract(...) if not yet implemented
    }

    // ... 13 more methods
};

static_assert(tensor::core::concepts::KernelBackend<Backend>);

} // namespace
```

The trailing `static_assert(KernelBackend<Backend>)` is your compile-time conformance check. If it fails, the compiler emits a diagnostic naming the missing method.

## Step 3 — wire the CMake option

In `CMakeLists.txt`, add the new value to the cache variable and the `find_package` logic:

```cmake
set(TENSOR_KERNEL_BACKEND "reference" CACHE STRING
    "Active KernelBackend adapter (reference | eigen | webgpu | cuda)")
set_property(CACHE TENSOR_KERNEL_BACKEND PROPERTY STRINGS reference eigen webgpu cuda)

if(TENSOR_KERNEL_BACKEND STREQUAL "cuda")
    find_package(CUDAToolkit REQUIRED)   # or whatever your substrate needs
    target_link_libraries(tensor INTERFACE CUDA::cublas)
    target_compile_definitions(tensor INTERFACE TENSOR_HAS_CUDA=1)
endif()
```

## Step 4 — register in vcpkg manifest (if relevant)

If your substrate is a vcpkg package, add a feature to `vcpkg.json`:

```json
{
  "features": {
    "cuda": {
      "description": "Enables the CUDA KernelBackend adapter (ADR-NNNN)…",
      "dependencies": ["cuda-toolkit"]
    }
  }
}
```

## Step 5 — write the cross-validation test

Create `tests/test_<name>_backend.cpp`:

```cpp
#include <doctest/doctest.h>
#include <tensor/core/backend/cuda/backend.hpp>
#include <tensor/core/backend/reference/backend.hpp>

TEST_CASE("cuda backend agrees with reference on add") {
    using namespace tensor::core;
    backend::cuda::Backend cuda{};
    backend::reference::Backend ref{};

    auto a = /* construct a tensor */;
    auto b = /* construct a tensor */;
    auto cuda_out = cuda.add(a, b);
    auto ref_out  = ref.add(a, b);

    // Element-wise agreement within tolerance:
    CHECK(numerically_close(cuda_out, ref_out, 1e-9));  // for double
    CHECK(numerically_close(cuda_out, ref_out, 1e-5));  // for float
}
```

Mirror the pattern in `tests/test_eigen_backend.cpp` / `tests/test_webgpu_backend.cpp` — there's an existing helper for "numerically_close" that respects QO-1's two-tier tolerances.

## Step 6 — wire into the §5 building-block table

Edit [`../../arc42/05-building-blocks/overview.md`](../../arc42/05-building-blocks/overview.md) and add a new row:

```markdown
| `tensor::core::backend::cuda` | **DrivenAdapter** | <phase> ✅ | Description… | [NNNN](...) |
```

Also update the Container Ownership Table later in the same file, and the dependency-rule enforcement examples (the `for adapter in reference eigen webgpu` loop now needs `cuda` added).

## Step 7 — write the detailed-design HOW

Create `docs/detailed-design/cuda-element-wise-kernels.md` (or similar) using Template-3 (`docs/templates/3_module-detailed-design.md`). Include the §1 context, §4 design, §6 testing, §7 cross-references, §8 future work.

Add the new DD to the [detailed-design README index](../../detailed-design/README.md).

## Step 8 — update the glossary

Add a §12 entry for any new public name your adapter introduces. Per ADR-0015 §Compliance, every public name traces to a source.

## Step 9 — update the workspace.dsl

Edit [`../../diagrams/c4/workspace.dsl`](../../diagrams/c4/workspace.dsl) to add the new container under `tensor::core::backend`. Regenerate the SVG export (cycle-5 above flags this as a manual step in the current pipeline).

## Step 10 — update INDEX.md

Bump the "X Template-3 instances" count in [`../../INDEX.md`](../../INDEX.md), and add a pointer to the new DD in the `detailed-design/` bullet.

## Step 11 — Phase 6.5 follow-up: companion PyPI distribution

When the Phase 6.5 `set_backend()` packaging lands ([ADR-0019](../../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md)), a new backend also needs:

- A companion distribution `tensor-named-axis-<backend>`.
- A new `[<backend>]` extra in the base `pyproject.toml`'s `[project.optional-dependencies]`.
- A new row in the cibuildwheel matrix.

This is the M3 work in the Phase 6.5 impl-plan. Reference: [`../../impl-plans/2026-05-13_phase-6-5-set-backend.md` §P6.5.M3](../../impl-plans/2026-05-13_phase-6-5-set-backend.md).

## Common pitfalls

- **Forgetting `static_assert(KernelBackend<Backend>)`**. The compile-time check is what catches missing methods. Add it at the bottom of `backend.hpp`.
- **Including a sibling adapter's header.** Violates the dependency rule. Run the `grep` enforcement command from arc42 §5 before opening the PR.
- **Skipping the ADR.** A new backend is an architectural decision; reviewers will block the PR until the ADR lands.
- **Missing the cross-validation test.** The 15-method conformance is enforced by `KernelBackend<Backend>`; *numerical* agreement is enforced by `tests/test_<name>_backend.cpp`. Both are required.

## Cross-references

- [`../explanation/hexagonal-lite-rationale.md`](../explanation/hexagonal-lite-rationale.md) — why the architecture is structured this way.
- [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md) — the port surface.
- [`../how-to/use-set-backend.md`](./use-set-backend.md) — the Python-side runtime selector (Phase 6.5).
- [ADR-0011](../../arc42/09-decisions/0011-kernel-backend-port-api.md) — the port API anchor.
- [ADR-0019](../../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) — the Phase 6.5 packaging strategy.
