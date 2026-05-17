# `static_catch_suite/` — Per-entry format for the catch-rate benchmark

Each subdirectory is **one entry** of the suite. The directory name
encodes the bug class:

- `T1_<descriptor>/` — transposed contraction
- `T2_<descriptor>/` — silent broadcast
- `T3_<descriptor>/` — reduction over wrong axis
- `T4_<descriptor>/` — layout swap
- `T5_<descriptor>/` — stack vs concat confusion
- `control_<descriptor>/` — a control program that uses the same
  operators correctly (used for false-positive rate)

Each entry directory contains **seven files**:

| File                     | Purpose                                                              |
| ------------------------ | -------------------------------------------------------------------- |
| `reference.json`         | The intended correct output (shape + values); used to score `Silent` |
| `einops.py`              | Idiomatic einops translation of the (bug or control) program          |
| `einx.py`                | Idiomatic einx translation                                            |
| `haliax.py`              | Idiomatic Haliax translation (JAX-backed)                             |
| `penzai.py`              | Idiomatic Penzai translation (JAX-backed)                             |
| `tensor_static.cpp`      | Idiomatic tensor translation using `TypedTensor<T, "i", ...>`         |
| `tensor_dynamic.cpp`     | Idiomatic tensor translation using `DynamicTensor` + `_tex` Evaluator |

The harness's `run_static_catch_suite()` iterates these directories,
compiles/invokes each translation, and emits one `CatchRecord` per
(library, entry) pair.

## Scoring contract

For each translation:

1. Attempt to compile (C++) / parse + trace (Python with JIT decorators).
   - Fails → outcome `CT`, diagnostic excerpt = first 200 chars of
     compiler/tracer output.
2. If compilation succeeded, attempt to run.
   - Raises exception → outcome `RT`, diagnostic excerpt = exception
     str().
3. If run succeeded, compare result to `reference.json` at the
   recorded tolerance (numerical: ℓ∞ ≤ 1e-6; structural: shape match).
   - Match → outcome `Pass` (expected for control entries; for bug
     entries indicates instrumentation defect and the entry is
     retracted with a note).
   - Mismatch → outcome `Silent`.

## Suite size

Final B-stage suite: $5 \text{ classes} \times 10 \text{ instances} = 50$
bug entries + 25 control entries × 6 translations each (5 libraries +
tensor_static and tensor_dynamic counted separately) = 450 programs.

The current A-stage skeleton ships one worked example
(`T1_transposed_contraction/`) so the format is concrete.
