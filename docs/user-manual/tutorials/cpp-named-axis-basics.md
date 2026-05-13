---
status: Stable
owner: tensor
last-reviewed: 2026-05-14
---

# C++ named-axis basics — your first `DynamicTensor` in 15 minutes

> **Diátaxis quadrant**: Tutorial. The reader is a *C++ learner*; the goal is producing the feeling "I can use this library", not encyclopedic coverage. For a reference, see [`../reference/cpp-namespace-overview.md`](../reference/cpp-namespace-overview.md).

Estimated time: 15 minutes (10 of build + 5 of code-along).

## Prerequisites

- A C++20 compiler (GCC ≥ 11 / Clang ≥ 15 / MSVC ≥ 19.30 / AppleClang ≥ 14).
- CMake ≥ 3.25, Ninja (or accept the platform default generator).
- vcpkg with `VCPKG_ROOT` set to your clone.

If you only want to *read* this code without building, the bundled Jupyter Book runs the same examples in the browser at <https://uyuutosa.github.io/tensor/>.

## Step 1 — build the project

```bash
git clone https://github.com/uyuutosa/tensor.git
cd tensor
cmake --preset=default
cmake --build --preset=default
```

What this gives you:

- A header-only `tensor::tensor` CMake interface library (consumable from your own CMake project via `target_link_libraries(your_target PRIVATE tensor::tensor)`).
- A `tests/` binary that exercises the library against the canonical 2016 outputs.
- A `bench/` binary (if you add `-DTENSOR_BUILD_BENCH=ON`).

Smoke-test:

```bash
ctest --preset=default
```

All tests should pass.

## Step 2 — first tensor

In your own `main.cpp`:

```cpp
#include <tensor/core/dynamic_tensor.hpp>
#include <tensor/core/format.hpp>
#include <iostream>

int main() {
    using namespace tensor::core;

    auto a = DynamicTensor<double>(
        DynamicShape{{"i", 5}},
        {1.0, 2.0, 3.0, 4.0, 5.0}
    );
    std::cout << a << "\n";
}
```

Compile against the installed library:

```bash
g++ -std=c++20 main.cpp -I include -lfmt -o demo
./demo
```

You'll see the ASCII-box rendering — the same format the [2016 README](https://github.com/uyuutosa/tensor/blob/main/archive/legacy-2016/README.md) used.

## Step 3 — Einstein-broadcast addition

```cpp
auto b = DynamicTensor<double>(
    DynamicShape{{"j", 2}},
    {10.0, 20.0}
);
auto c = a + b;   // disjoint labels → rank-2 outer-sum tensor
std::cout << "rank: " << c.shape().rank() << "\n";  // → 2
std::cout << c << "\n";
```

The axis labels (`"i"` vs `"j"`) decide that this is **outer-broadcast**, producing a 5×2 result. Same labels would give element-wise.

## Step 4 — the compile-time path

The header-only library also offers a compile-time NTTP-labelled tensor for users who want the labels checked at *compile time*:

```cpp
#include <tensor/core/typed_tensor.hpp>
#include <tensor/core/label_tag.hpp>

using namespace tensor::core::literals;   // brings "i"_ax into scope

TypedTensor<double, "i", "j"> A{{2, 3}, {1, 2, 3, 4, 5, 6}};
TypedTensor<double, "i", "j"> B{{2, 3}, {10, 20, 30, 40, 50, 60}};
auto C = A + B;   // OK — same label set
TypedTensor<double, "i", "k"> D{{2, 3}, {0, 0, 0, 0, 0, 0}};
// auto E = A + D;   // ← would static_assert fail at compile time
```

The `_ax` UDL is the entry point for compile-time labels; see [`../how-to/named-tensor-types.md`](../how-to/named-tensor-types.md) for the decision guide on when to use `Tensor`, `DynamicTensor`, or `TypedTensor`.

## Step 5 — the `_tex` UDL: the formula is the program

```cpp
#include <tensor/tex/tex.hpp>

using namespace tensor::tex;

auto expr = R"(c_{ij} = a_i b_j)"_tex;   // parsed at compile time

Evaluator<double> ev;
ev.bind("a", a);
ev.bind("b", b);
auto c2 = ev.evaluate(expr);

std::cout << c2 << "\n";   // identical to `c` from Step 3
```

The LaTeX string is a real parse; the resulting `Expression` is the AST; `Evaluator::evaluate` runs the AST against the bound named tensors. *The formula is the program* — that's the headline feature (G-3, [ADR-0005](../../arc42/09-decisions/0005-adopt-tex-lyx-as-authoring-surface.md)).

## Step 6 — where to go next

| If you want to…                                          | Read                                                                                                  |
| -------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| See the equivalent Python surface                         | [`./python-getting-started.md`](./python-getting-started.md)                                          |
| Build your own autograd                                   | [`tutorials/05_autograd-from-scratch.ipynb`](../../../tutorials/05_autograd-from-scratch.ipynb)        |
| See the WGSL kernels under WebGPU                         | [`tutorials/06_webgpu-acceleration.ipynb`](../../../tutorials/06_webgpu-acceleration.ipynb)            |
| Train a small model end-to-end                            | [`tutorials/07_mlp-on-toy.ipynb`](../../../tutorials/07_mlp-on-toy.ipynb)                             |
| Compare backends (reference vs Eigen)                     | [`tutorials/08_swappable-backends.ipynb`](../../../tutorials/08_swappable-backends.ipynb)              |
| See the full C++ namespace surface                        | [`../reference/cpp-namespace-overview.md`](../reference/cpp-namespace-overview.md)                    |
| Understand the Hexagonal-lite architecture                | [`../explanation/hexagonal-lite-rationale.md`](../explanation/hexagonal-lite-rationale.md)            |

## What you did NOT learn here (on purpose)

- The full `KernelBackend` port surface and how to write your own adapter. See [`../../detailed-design/kernel-backend-port.md`](../../detailed-design/kernel-backend-port.md).
- Mdspan interop, vendor-specific build flags, contributor workflow. See `CONTRIBUTING.md` and `book/`.
- Production-style error handling. The educational frame keeps diagnostics short.
