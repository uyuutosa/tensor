---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# Why named axes?

> **Diátaxis quadrant**: Explanation. Understanding-oriented. The goal is to make you think differently about tensor APIs, not to teach you a specific recipe. For "how do I…?" see [`../how-to/`](../how-to/); for first hands-on, see [`../tutorials/`](../tutorials/).

## The complaint

Most tensor libraries you've used — NumPy, Eigen, PyTorch, JAX, TensorFlow — identify tensor axes by **position**. The second axis of `a` is `a.shape[1]`; the third axis is `a.shape[2]`. Broadcasting follows positional rules: NumPy aligns from the right, PyTorch pads the front, JAX uses static shape inference. Operations like `np.einsum("ij,jk->ik", A, B)` use letters as positional placeholders inside a string DSL.

This positional convention is a *cognitive tax*. Every time you reshape, transpose, broadcast, or contract, you're solving a "which axis is which?" puzzle in your head. The puzzle is easy on small examples and hard on real ones — half the bugs in production ML code are off-by-one-axis bugs that survive the test suite because the shapes happen to be square.

## The named-axis idea

If axes had **names** that travelled with the tensor at construction time, you'd:

- Never need to know "is `n_features` the first or the second axis?" The name *is* the answer.
- Write `a + b` and have it mean exactly what the math says: same name → element-wise, different names → broadcast in those new dimensions.
- Write `contract(A, B)` to mean "sum over every axis that appears in both A and B" — the named version of Einstein summation, but with the names being descriptive rather than alphabetic letters.

This is the heart of `tensor`. Every public operation respects axis names. Code that operates on a `tensor::core::DynamicTensor<double>` doesn't need to know how many axes there are or what order they're in — it asks for the axis by *name*.

## How is this different from xarray / NamedTensors in PyTorch / `NamedMapping` in Mojo?

Other libraries have tried this:

- **xarray** (Python) — named axes for scientific computing; great for time-series with `time` / `lat` / `lon` coords. But xarray is built for NumPy-shaped workloads, not differentiable programming; it has no autograd, no GPU acceleration beyond Dask, no formal type system.
- **PyTorch named tensors** — experimental for years; deprecated as of late 2024 because PyTorch's broadcasting rules are too entrenched to retrofit naming on top of.
- **Mojo `NamedMapping`** — different concept entirely (the `NamedMapping` in Mojo MAX is the *parameter* dictionary in a model, not the tensor's axes). See the [§12 disambiguation entry](../../arc42/12-glossary/overview.md).

What `tensor` does differently:

1. **Named axes are the default, not an opt-in.** There's no positional path next to a named path; named is the only path. Positional access is intentionally awkward.
2. **C++20 NTTP support makes labels compile-time-checkable.** A `TypedTensor<double, "i", "j"> + TypedTensor<double, "i", "k">` is a *compile* error — the compiler flags the label mismatch before the program ever runs. This is unavailable to Python-side approaches.
3. **The architecture is small.** `tensor::core` is a few thousand lines of header-only C++; the library reads like the math. Production-shaped libraries (PyTorch, JAX) are millions of lines and most of that complexity is positional-shape inference machinery you don't need with names.

## What you give up

Not free. Named axes cost:

- **Construction verbosity.** `tensor::core::DynamicShape{{"i", 5}, {"j", 3}}` is wordier than `np.zeros((5, 3))`. Acceptable: the construction site is where the axis identity matters most; making it explicit at the construction site lets the rest of the code be implicit.
- **Operation coverage.** `tensor` doesn't ship every operation NumPy / PyTorch ship — by design ([ADR-0010](../../arc42/09-decisions/0010-refine-positioning-to-educational-first-production-capable.md)). The library is educational-first, production-capable via backend adapters; it skips operations that don't teach something. For production-shaped workloads, the Hexagonal port lets you plug a fast adapter underneath without leaving the named-axis frame.
- **Familiarity tax for newcomers.** If you've been writing positional NumPy for 10 years, the first hour with named axes feels like learning to write left-handed. The headline notebooks (`python/notebooks/00_python-sdk-tour.ipynb`) are short by design — the goal is to get past the friction quickly.

## Why this matters now (2025–2026)

Three things shifted in 2024–2025:

1. **C++20 NTTPs landed widely**. Class-type non-type template parameters made compile-time label checking practical. The MSVC 19.30+ / GCC 11+ / Clang 13+ matrix supports it consistently.
2. **WebGPU production-ready via Dawn**. The fast path is no longer CUDA-only; a WebGPU adapter runs on every desktop GPU without vendor toolchains.
3. **The Python ML ecosystem hit a wall on positional broadcasting**. PyTorch's named-tensor experiment failed; JAX's static shape inference is sophisticated but still positional. The 2026-05-12 landscape report flagged that **no first-class named-axis tensor exists in production Python ML** — there's a wedge to open.

`tensor` is opening that wedge. The C++ Domain is the *reference implementation* (educational, citable, hackable). The Python SDK ([ADR-0018](../../arc42/09-decisions/0018-phase-6-python-sdk-entry-via-nanobind.md)) makes it reachable from where the practitioners are.

## Where to go next

- Hands-on Python: [`../tutorials/python-getting-started.md`](../tutorials/python-getting-started.md).
- Hands-on C++: [`../tutorials/cpp-named-axis-basics.md`](../tutorials/cpp-named-axis-basics.md).
- The Hexagonal architecture this rests on: [`./hexagonal-lite-rationale.md`](./hexagonal-lite-rationale.md).
- The formula-is-the-program slogan: [`./formula-is-the-program-essay.md`](./formula-is-the-program-essay.md).
- The picker between `Tensor` / `DynamicTensor` / `TypedTensor`: [`../how-to/named-tensor-types.md`](../how-to/named-tensor-types.md).
