# `results/` — Output schema for `bench_mlsys_paper_case_study.py`

The harness writes one JSON file per run with the following structure
(every field is required unless noted optional):

```jsonc
{
  "metadata": {
    "harness_version": "0.1.0-skeleton",
    "paper_section_references": {
      "static_catch":  "§6.1, Table 3",
      "ba_case_study": "§6.2, Table 4"
    },
    "n_runs": 30,
    "n_warmup": 5
  },
  "timings": [
    {
      "backend":    "tensor.reference",     // or tensor.eigen / tensor.webgpu / einops.numpy / haliax.jax
      "scene":      "courtyard",            // or fountain_p11 / ladybug_49 / synthetic_small
      "n_views":    50,
      "n_landmarks": 200,
      "metric":     "t_residual_ms",        // or t_gradient_ms / peak_rss_mb
      "median":     12.345,
      "ci_lo_95":   12.110,
      "ci_hi_95":   12.612,
      "n_runs":     30,
      "n_warmup":   5,
      "notes":      ""                       // optional, free-text
    }
  ],
  "catches": [
    {
      "library":             "tensor.static",       // or einops / einx / haliax / penzai / tensor.dynamic
      "entry_id":            "T1_transposed_contraction",
      "bug_class":           "T1",                  // T1..T5 or "control"
      "outcome":             "CT",                  // CT / RT / Silent / Pass
      "diagnostic_excerpt":  "static assertion failed: TypedTensor: axis labels must match at compile time..."
    }
  ]
}
```

## Aggregation

Tables 3 and 4 in the paper are rendered by `plotting/render_tables.py`,
which reads `results.json` and produces LaTeX-table fragments suitable for
direct inclusion at the marked `% AUTO-GENERATED:` comments in
`paper/mlsys-2027/main.tex`.

Aggregation logic:

- **Table 3 (static-catch)**: group catches by `library`, count outcomes by
  `bug_class`. CT % = (CT count) / 50 for bug instances; FP % =
  (RT + CT count) / 25 for control instances.
- **Table 4 (BA case study)**: group timings by `(backend, scene)`, pivot
  on `metric`. Report `median ± half-CI` per cell.

## Provenance

Every record carries `n_runs` and `n_warmup` so the bibliography
discipline survives downstream copies. Records from different harness
versions (`metadata.harness_version`) should not be mixed in the same
table without an explicit re-aggregation note.
