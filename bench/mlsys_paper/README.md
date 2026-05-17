# `bench/mlsys_paper/` — Benchmark harness for the MLSys 2027 paper

Reproducibility package for `paper/mlsys-2027/main.tex`. Produces every
number that appears in `Table 3` (static-catch rate, §6.1) and `Table 4`
(bundle-adjustment case study, §6.2).

> **Status: skeleton + protocols, full implementation scheduled for
> September 2026.** The directory structure, harness interface,
> `results.json` schema, and one worked static-catch entry (T1) ship
> now so reviewers can verify the protocol design before the numbers
> land. The full 450-program suite and the BA case-study runner are
> filled in by the B-stage work tracked in the paper's
> "Status of results" paragraph.

## Layout

```
bench/mlsys_paper/
├── README.md                         (this file)
├── bench_mlsys_paper_case_study.py   main driver (cli entrypoint)
├── results/
│   ├── README.md                     results.json schema documentation
│   └── results.json                  (generated; one record per measurement)
├── datasets/
│   ├── __init__.py
│   ├── downloader.py                 fetch ETH3D / Strecha / BAL on demand
│   └── loader.py                     unified scene → arrays interface
├── backends/
│   ├── __init__.py                   common BackendRunner protocol
│   ├── tensor_reference.py
│   ├── tensor_eigen.py
│   ├── tensor_webgpu.py
│   ├── einops_baseline.py
│   └── haliax_baseline.py
├── static_catch_suite/
│   ├── README.md                     suite format + scoring protocol
│   └── T1_transposed_contraction/    one worked example
│       ├── reference.json            intended output
│       ├── einops.py
│       ├── einx.py
│       ├── haliax.py
│       ├── penzai.py
│       ├── tensor_static.cpp
│       └── tensor_dynamic.cpp
└── plotting/
    └── render_tables.py              results.json → LaTeX tables
```

## Running

```bash
# CPU substrates only (no GPU required, ~10 minutes on a laptop):
python bench_mlsys_paper_case_study.py \
    --substrates reference,eigen,einops,haliax \
    --scenes synthetic_small,ladybug_49 \
    --output results/results.json

# Full run with GPU (requires Dawn-built tensor + RTX-class GPU):
python bench_mlsys_paper_case_study.py \
    --substrates all \
    --scenes courtyard,fountain_p11,ladybug_49 \
    --output results/results.json

# Static-catch suite only (no datasets needed):
python bench_mlsys_paper_case_study.py --static-catch-only \
    --output results/static_catch.json
```

## Environment

Pinned conda environment in `environment.yml` (sibling of this file)
plus a `Dockerfile.webgpu` for the Dawn-required lane. Required:

- Python 3.11
- `tensor` library installed in editable mode (`pip install -e ../../python`)
- `einops>=0.8`, `haliax>=1.6` (will be installed via conda env)
- For the WebGPU lane: CMake-built `tensor` with
  `-DTENSOR_KERNEL_BACKEND=webgpu` and Dawn r6814+

## Schema

See [`results/README.md`](results/README.md) for the per-record
schema. The plotting script generates LaTeX-table fragments that drop
into `paper/mlsys-2027/main.tex` at the marked `% AUTO-GENERATED:`
comments.

## See also

- `paper/mlsys-2027/main.tex` §6 — the methodology this harness
  implements.
- `tutorials/09_bundle-adjustment.ipynb` — the 1-D pedagogical
  version of the BA case study.
