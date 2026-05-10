workspace "tensor" "Header-only C++20/23 educational library for named-axis tensor algebra." {

    !identifiers hierarchical

    model {

        # ─── External actors (people) ────────────────────────────────────────
        learner = person "Learner" "Self-taught C++ programmer or researcher exploring named-axis tensor algebra and modern C++."
        instructor = person "Instructor" "University / bootcamp instructor assigning tensor / ML coursework."

        # ─── External systems ────────────────────────────────────────────────
        toolchain = softwareSystem "Host C++ Toolchain" "GCC 11+ / Clang 13+ / MSVC 19.30+ + CMake 3.25+ + vcpkg." "External"
        jupyterStack = softwareSystem "Jupyter + xeus-cling" "Jupyter Lab/Notebook with xeus-cling C++20 kernel." "External"
        webgpuRuntime = softwareSystem "WebGPU Runtime" "Dawn (desktop) or wgpu-native — system-installed GPU driver, no proprietary toolchain." "External"
        browser = softwareSystem "Web Browser" "Chrome / Firefox / Safari with WebGPU support; renders Jupyter Book and (later) browser-side demos." "External"
        ghPages = softwareSystem "GitHub Pages" "Hosts the Jupyter Book site rendered from tutorials/." "External"
        upstreamPentaglyph = softwareSystem "pentaglyph-docs (upstream)" "Documentation kit vendored as git subtree under libs/pentaglyph-docs." "External"

        # ─── The system under design ─────────────────────────────────────────
        tensor = softwareSystem "tensor" "Header-only C++20/23 named-axis tensor algebra library + bundled tutorials." {

            core = container "tensor::core" "Named-axis tensor types, expression templates, mdspan interop." "C++20 headers" "Library"
            autograd = container "tensor::autograd" "Tape-based reverse-mode automatic differentiation typed against named-axis tensors." "C++20 headers" "Library"
            gpu = container "tensor::gpu" "WebGPU codegen + runtime adapter; emits WGSL kernels from named-axis expressions." "C++20 headers + WGSL" "Library"
            tex = container "tensor::tex" "consteval LaTeX-subset parser (Einstein notation) + LaTeX output renderer." "C++20 headers" "Library"
            tutorials = container "tutorials/" "Jupyter notebooks demonstrating the library, executed in CI per release." "Jupyter (xeus-cling C++20)" "Tutorial"
            book = container "Jupyter Book site" "Static site generated from tutorials/, deployed to GitHub Pages." "Jupyter Book / HTML" "Site"
            lyx = container "lyx-export (Phase 3+)" "LyX module that exports tensor-bearing documents to the consteval _tex DSL." "LyX module" "Tool"
        }

        # ─── L1 (System Context) relations ───────────────────────────────────
        learner -> tensor "Builds against, reads, and runs tutorials of"
        instructor -> tensor "Assigns tutorials and exercises from"

        tensor -> toolchain "Is consumed by — header-only, built into the learner's project via" "CMake + vcpkg"
        tensor -> jupyterStack "Tutorials run inside" "xeus-cling kernel"
        tensor -> webgpuRuntime "Dispatches GPU kernels through" "Dawn / wgpu-native (Phase 3+)"
        tensor -> ghPages "Jupyter Book site is published to" "GitHub Pages"
        tensor -> upstreamPentaglyph "Vendored documentation kit pulled from" "git subtree pull"

        learner -> jupyterStack "Opens tutorials in"
        learner -> browser "Reads the Jupyter Book site in"
        instructor -> browser "Reviews course materials in"

        # ─── L2 (Container) relations ────────────────────────────────────────
        tensor.autograd -> tensor.core "Wraps tensors and operations from"
        tensor.gpu -> tensor.core "Lowers expressions from" "WGSL codegen"
        tensor.tex -> tensor.core "Produces expression graphs consumed by"
        tensor.tutorials -> tensor.core "Imports and demos"
        tensor.tutorials -> tensor.autograd "Imports and demos (Phase 2+)"
        tensor.tutorials -> tensor.gpu "Imports and demos (Phase 3+)"
        tensor.tutorials -> tensor.tex "Imports and demos"
        tensor.book -> tensor.tutorials "Renders to static HTML from"
        tensor.lyx -> tensor.tex "Exports LyX content into the DSL of (Phase 3+)"

        tensor.gpu -> webgpuRuntime "Issues GPU dispatches via" "WebGPU API (Dawn / wgpu-native)"
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
            element "Library" {
                background "#1168bd"
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
            element "Tool" {
                background "#cccccc"
                color "#000000"
            }
        }

        theme default
    }
}
