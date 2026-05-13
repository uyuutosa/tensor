---
status: Draft
owner: tensor
last-reviewed: 2026-05-13
---

# How to — pick a kernel backend with `tensor.set_backend()` (Phase 6.5)

> **Status note.** Phase 6.5 is in the planning stage as of 2026-05-13 — [ADR-0019](../../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) ratified, [impl-plan](../../impl-plans/2026-05-13_phase-6-5-set-backend.md) drafted, implementation in flight. This how-to is published *ahead* of implementation per the project's "doc-before-impl for major surfaces" pattern. The exact API shape may shift during M2; this page will move from Draft to Stable when `0.3.0` ships.

## 1. Why three backends?

The C++ Domain in `tensor::core` is portable but slow on its own. Three pre-built kernel adapters bring speed without changing the Domain:

| Backend       | What it brings                                                          | When to pick it                                                                  |
| ------------- | ----------------------------------------------------------------------- | -------------------------------------------------------------------------------- |
| `reference`   | Pure-C++ implementation; canonical answer for the test suite.            | The educational on-ramp. Works on every machine. ~5 MB wheel.                    |
| `eigen`       | Eigen 3.4 SIMD element-wise + BLAS-flavoured GEMM.                       | CPU speedup, no GPU needed. ~20 MB wheel.                                        |
| `webgpu`      | Dawn-backed WGSL kernels on the local GPU (Vulkan / Metal / D3D12).     | GPU speedup; great on the maintainer's RTX 3090. ~60 MB wheel (Dawn runtime).    |

## 2. Install only what you need

```bash
# Default — reference only, fastest cold-start, smallest download:
pip install tensor-named-axis

# Add Eigen:
pip install tensor-named-axis[eigen]

# Add WebGPU:
pip install tensor-named-axis[webgpu]

# Add both:
pip install tensor-named-axis[all]
```

Under the hood, the extras pull in companion PyPI distributions — `tensor-named-axis-eigen`, `tensor-named-axis-webgpu` — that share the same `tensor/` namespace package (PEP-420). The matching `_tensor_native_<backend>.so` lives inside each companion wheel.

## 3. Switch at runtime

```python
import tensor

# What's installed?
print(tensor.list_available_backends())
# → ['reference']            (default install)
# → ['reference', 'eigen']   (after pip install ...[eigen])

# What's currently active?
print(tensor.current_backend())
# → 'reference'

# Switch:
tensor.set_backend('eigen')

# Subsequent tensor construction uses Eigen kernels:
a = tensor.DynamicTensor(
    tensor.DynamicShape([tensor.Axis("i", 1000), tensor.Axis("j", 1000)]),
    [0.0] * 1_000_000,
)
b = a + a       # ← element-wise SIMD via Eigen
```

A `set_backend()` call **only affects tensors constructed after it**. Tensors built before the switch continue to use their build-time backend until they go out of scope. Per ADR-0019, mid-computation switching is undefined.

## 4. Asking for a backend you don't have installed

```python
import tensor
tensor.set_backend('webgpu')
# RuntimeError: webgpu backend is not installed.
# Install with:  pip install tensor-named-axis[webgpu]
# Or install all backends:  pip install tensor-named-axis[all]
# Currently available: ['reference', 'eigen']
```

The error message includes the exact `pip install` command for the missing backend. There's no fallback-to-reference: the explicit "request what you don't have" path raises so misconfigured pipelines fail loudly.

## 5. Cross-validation

```python
import numpy as np
import tensor

shape = tensor.DynamicShape([tensor.Axis("i", 64), tensor.Axis("j", 64)])
A_np = np.random.randn(64, 64)
B_np = np.random.randn(64, 64)

A = tensor.from_numpy(A_np, ["i", "j"])
B = tensor.from_numpy(B_np, ["j", "k"])

results = {}
for backend in tensor.list_available_backends():
    tensor.set_backend(backend)
    results[backend] = tensor.contract(A, B).numpy()

# All backends agree element-wise within tolerance:
ref = results['reference']
for backend, arr in results.items():
    if backend == 'reference': continue
    assert np.allclose(ref, arr, atol=1e-5), f'{backend} disagrees with reference'
```

This is the same QO-1 guarantee the C++ test suite enforces, exposed to the Python user as a one-page demo. The Phase 6.5 M2 exit criterion is exactly this assert passing on a `pip install tensor-named-axis[all]` build.

## 6. Where this fits

- **C++ users** select the backend at CMake configure time via `-DTENSOR_KERNEL_BACKEND={reference,eigen,webgpu}`. Their build embeds one backend; switching requires a rebuild. (Educational artifact: the build itself is the experiment.)
- **Python users** select at install time via `pip install ...[extras]` and at runtime via `set_backend()`. The wheel matrix ships one `_tensor_native_<backend>.so` per backend; the Python adapter dispatches.

Both surfaces are projections of the same C++ Domain. The numerical answer is the same; only the *kernel* differs.

## 7. Cross-references

- [`docs/arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md`](../../arc42/09-decisions/0019-phase-6-5-runtime-backend-selection-via-extras.md) — the decision record (packaging trade-off).
- [`docs/impl-plans/2026-05-13_phase-6-5-set-backend.md`](../../impl-plans/2026-05-13_phase-6-5-set-backend.md) — the four milestones to ship this.
- [`docs/detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md) — the C++ port the three backends implement.
- [`docs/api-contract/python-public-surface.md`](../../api-contract/python-public-surface.md) §2 — the (forward-pinned) public-symbol contract.
- [`tutorials/08_swappable-backends.ipynb`](../../../tutorials/08_swappable-backends.ipynb) — the C++ side of the same teaching arc.
