workspace "tensor" "Header-only C++20/23 named-axis differentiable tensor library aspiring to canonical-reference quality (ADR-0015)." {

    !identifiers hierarchical

    model {

        # ─── External actors (people) ────────────────────────────────────────
        learner = person "Learner" "Self-taught C++ programmer or researcher exploring named-axis tensor algebra and modern C++."
        instructor = person "Instructor" "University / bootcamp instructor assigning tensor / ML coursework."

        # ─── External systems ────────────────────────────────────────────────
        toolchain = softwareSystem "Host C++ Toolchain" "GCC 11+ / Clang 13+ / MSVC 19.30+ + CMake 3.25+ + vcpkg." "External"
        jupyterStack = softwareSystem "Jupyter + xeus-cpp" "Jupyter Lab/Notebook with xeus-cpp 0.10+ C++20 kernel (ADR-0014 §3; xeus-cling kept as legacy smoke for 00_intro)." "External"
        webgpuRuntime = softwareSystem "WebGPU Runtime (Dawn)" "Google's Dawn implementation of the WebGPU spec, installed via vcpkg port 20260410.140140 (ADR-0014 §1 + ADR-0016). System Vulkan / Metal / D3D12 driver underneath." "External"
        browser = softwareSystem "Web Browser" "Chrome / Firefox / Safari with WebGPU support; renders Jupyter Book and (later) browser-side demos." "External"
        ghPages = softwareSystem "GitHub Pages" "Hosts the Jupyter Book site rendered from tutorials/." "External"
        upstreamPentaglyph = softwareSystem "pentaglyph-docs (upstream)" "Documentation kit vendored as git subtree under libs/pentaglyph-docs." "External"

        # ─── The system under design ─────────────────────────────────────────
        tensor = softwareSystem "tensor" "Header-only C++20/23 named-axis differentiable tensor library + bundled tutorials." {

            # Hexagonal classification per ADR-0009: Domain / DrivingAdapter / DrivenAdapter.
            # The KernelBackend port (ADR-0011) admits three concrete adapters
            # — reference (canonical), eigen (SIMD + GEMM), webgpu (Dawn) —
            # selected at CMake configure time via TENSOR_KERNEL_BACKEND.
            core = container "tensor::core" "Named-axis tensor types, expression templates, mdspan interop. Owns concepts.hpp declaring the KernelBackend port (ADR-0011)." "C++20 headers" "Domain"
            autograd = container "tensor::autograd" "Tape-based reverse-mode automatic differentiation typed against named-axis tensors. Extends the Domain hexagon." "C++20 headers" "Domain"
            tex = container "tensor::tex" "consteval LaTeX-subset parser (Einstein notation) + LaTeX output renderer + Evaluator. Implements ExpressionSource / ExpressionSink ports." "C++20 headers" "DrivingAdapter"
            referenceBackend = container "tensor::core::backend::reference" "Canonical CPU implementation of the KernelBackend port; the reference against which every other adapter cross-validates." "C++20 headers" "DrivenAdapter"
            eigenBackend = container "tensor::core::backend::eigen" "Eigen 3.4 SIMD + GEMM adapter for the KernelBackend port; delegates non-trivial ops to reference per ADR-0011's documented scope." "C++20 headers + Eigen" "DrivenAdapter"
            webgpuBackend = container "tensor::core::backend::webgpu" "Dawn-backed WebGPU adapter for the KernelBackend port. As of 2026-05-12, 12 of 15 methods dispatch real GPU compute on float (4 binary + 4 unary + 1 contract + 3 broadcast); the rest delegate to reference. Talks to Dawn directly via webgpu_cpp.h (ADR-0016)." "C++20 headers + WGSL" "DrivenAdapter"
            lyx = container "lyx-export" "LyX module + Python translator that exports tensor-bearing LyX documents to the consteval _tex DSL. CI smoke verifies the golden-file diff." "LyX module + Python 3" "DrivingAdapter"
            tutorials = container "tutorials/" "Six Jupyter notebooks (00_intro, 01_formula-is-the-program, 05_autograd-from-scratch, 06_webgpu-acceleration, 07_mlp-on-toy, 08_swappable-backends). Outside the hexagon — demos, not adapters." "Jupyter (xeus-cpp / xeus-cling C++20)" "Tutorial"
            book = container "Jupyter Book site" "Static site generated from book/_toc.yml referencing tutorials/ + arc42 + detailed-design + reports. Deployed to GitHub Pages." "Jupyter Book / HTML" "Site"
        }

        # ─── L1 (System Context) relations ───────────────────────────────────
        learner -> tensor "Builds against, reads, and runs tutorials of"
        instructor -> tensor "Assigns tutorials and exercises from"

        tensor -> toolchain "Is consumed by — header-only, built into the learner's project via" "CMake + vcpkg"
        tensor -> jupyterStack "Tutorials run inside" "xeus-cpp / xeus-cling kernel"
        tensor -> webgpuRuntime "Dispatches GPU kernels through" "Dawn (webgpu_cpp.h)"
        tensor -> ghPages "Jupyter Book site is published to" "GitHub Pages"
        tensor -> upstreamPentaglyph "Vendored documentation kit pulled from" "git subtree pull"

        learner -> jupyterStack "Opens tutorials in"
        learner -> browser "Reads the Jupyter Book site in"
        instructor -> browser "Reviews course materials in"

        # ─── L2 (Container) relations ────────────────────────────────────────
        tensor.autograd -> tensor.core "Wraps tensors and operations from"
        tensor.tex -> tensor.core "Produces expression graphs consumed by"
        # The three KernelBackend adapters implement the port declared in core.
        tensor.referenceBackend -> tensor.core "Implements the KernelBackend port declared by"
        tensor.eigenBackend -> tensor.core "Implements the KernelBackend port declared by"
        tensor.webgpuBackend -> tensor.core "Implements the KernelBackend port declared by"
        tensor.eigenBackend -> tensor.referenceBackend "Delegates out-of-scope methods to" "private ref_ member"
        tensor.webgpuBackend -> tensor.referenceBackend "Delegates non-float / non-simple-GEMM / reduce / unbroadcast to" "private ref_ member"
        tensor.tutorials -> tensor.core "Imports and demos"
        tensor.tutorials -> tensor.autograd "Imports and demos"
        tensor.tutorials -> tensor.webgpuBackend "Imports and demos (tutorial 06 design walkthrough)"
        tensor.tutorials -> tensor.eigenBackend "Imports and demos (tutorial 08 Hexagonal payoff)"
        tensor.tutorials -> tensor.tex "Imports and demos"
        tensor.book -> tensor.tutorials "Renders to static HTML from"
        tensor.lyx -> tensor.tex "Exports LyX content into the DSL of"

        tensor.webgpuBackend -> webgpuRuntime "Issues GPU dispatches via" "WebGPU API (Dawn)"
        tensor.tutorials -> jupyterStack "Are executed by"
        tensor.book -> ghPages "Is deployed to"
    }

    views {

        systemContext tensor "L1-SystemContext" "C4 Level 1 — System Context for tensor." {
            include *
            autoLayout lr
        }

        container tensor "L2-Container" "C4 Level 2 — Containers inside tensor." {
            include *
            autoLayout tb
        }

        # ─── Component-level views (L3) — uncomment when detailed design lands ─
        #
        # component tensor.core "L3-Component-Core" "Components inside the core container." {
        #     include *
        # }

        styles {
            element "Person" {
                shape Person
                background "#08427b"
                color "#ffffff"
            }
            element "External" {
                background "#999999"
                color "#ffffff"
            }
            # Hexagonal layer styles (ADR-0009) — three colour bands so the L2
            # diagram doubles as a hexagon: Domain in the centre, drivers on
            # one side, driven adapters on the other.
            element "Domain" {
                background "#1168bd"
                color "#ffffff"
                shape Hexagon
            }
            element "DrivingAdapter" {
                background "#2e7d32"
                color "#ffffff"
            }
            element "DrivenAdapter" {
                background "#c62828"
                color "#ffffff"
            }
            element "Tutorial" {
                background "#0d8b8b"
                color "#ffffff"
            }
            element "Site" {
                background "#7e57c2"
                color "#ffffff"
            }
        }

        theme default
    }
}
