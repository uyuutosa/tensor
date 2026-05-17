#!/usr/bin/env python3
"""Bench harness driver for the MLSys 2027 paper.

Produces results.json populating Tables 3 (static-catch) and 4 (BA case
study) of paper/mlsys-2027/main.tex.

This is the A-stage skeleton: the CLI, dataset loader, backend dispatch,
metrics collection, and JSON-schema emission are wired end-to-end against
a synthetic-data smoke-test path. The B-stage work (September 2026)
populates the real backend implementations and the 450-program static-
catch suite.

Usage:
    python bench_mlsys_paper_case_study.py [options]

See README.md for the full CLI matrix.
"""
from __future__ import annotations

import argparse
import json
import statistics
import sys
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import Iterable, Sequence

# ─── Result record schema ───────────────────────────────────────────────


@dataclass
class TimingRecord:
    """One row of the BA case-study results table."""

    backend: str  # "tensor.reference" / "tensor.eigen" / "tensor.webgpu" / "einops.numpy" / "haliax.jax"
    scene: str  # "courtyard" / "fountain_p11" / "ladybug_49" / "synthetic_small"
    n_views: int
    n_landmarks: int
    metric: str  # "t_residual_ms" / "t_gradient_ms" / "peak_rss_mb"
    median: float
    ci_lo_95: float
    ci_hi_95: float
    n_runs: int
    n_warmup: int
    notes: str = ""


@dataclass
class CatchRecord:
    """One row of the static-catch suite scoring."""

    library: str  # "einops" / "einx" / "haliax" / "penzai" / "tensor.static" / "tensor.dynamic"
    entry_id: str  # e.g. "T1_transposed_contraction"
    bug_class: str  # "T1".."T5" or "control"
    outcome: str  # "CT" / "RT" / "Silent" / "Pass"
    diagnostic_excerpt: str = ""  # first 200 chars of the compiler/runtime output


@dataclass
class ResultsBundle:
    """The full output document. One file per run."""

    metadata: dict
    timings: list[TimingRecord] = field(default_factory=list)
    catches: list[CatchRecord] = field(default_factory=list)


# ─── Statistical helpers ────────────────────────────────────────────────


def bootstrap_ci(samples: Sequence[float], confidence: float = 0.95, n_resamples: int = 2000) -> tuple[float, float]:
    """Percentile bootstrap CI for the median. Conservative, no scipy needed."""
    import random

    if len(samples) < 2:
        return (samples[0], samples[0]) if samples else (0.0, 0.0)
    medians = []
    n = len(samples)
    for _ in range(n_resamples):
        resample = [samples[random.randrange(n)] for _ in range(n)]
        medians.append(statistics.median(resample))
    medians.sort()
    lo_idx = int(((1 - confidence) / 2) * n_resamples)
    hi_idx = int((1 - (1 - confidence) / 2) * n_resamples)
    return medians[lo_idx], medians[hi_idx - 1]


# ─── Backend dispatch ───────────────────────────────────────────────────


def load_backend(name: str):
    """Lazily import a backend module from backends/<name>.py."""
    from importlib import import_module

    module_name = {
        "reference": "backends.tensor_reference",
        "eigen": "backends.tensor_eigen",
        "webgpu": "backends.tensor_webgpu",
        "einops": "backends.einops_baseline",
        "haliax": "backends.haliax_baseline",
    }[name]
    try:
        return import_module(module_name)
    except ImportError as e:
        print(f"[skip] backend {name!r} unavailable: {e}", file=sys.stderr)
        return None


# ─── BA case study runner ───────────────────────────────────────────────


def run_ba_timing(backend, scene_loader, n_runs: int = 30, n_warmup: int = 5) -> list[TimingRecord]:
    """Run the BA case study against one backend / one scene; return TimingRecords."""
    scene = scene_loader.load()
    cameras_init, landmarks_init, observations = scene.cameras, scene.landmarks, scene.observations

    # Warm-up.
    for _ in range(n_warmup):
        backend.residual_and_gradient(cameras_init, landmarks_init, observations)

    # Timed runs.
    t_r_samples: list[float] = []
    t_g_samples: list[float] = []
    peak_samples: list[float] = []
    for _ in range(n_runs):
        t0 = time.perf_counter()
        r = backend.residual(cameras_init, landmarks_init, observations)
        t1 = time.perf_counter()
        g = backend.gradient(cameras_init, landmarks_init, observations)
        t2 = time.perf_counter()
        peak = backend.peak_rss_mb()
        t_r_samples.append((t1 - t0) * 1000.0)
        t_g_samples.append((t2 - t1) * 1000.0)
        peak_samples.append(peak)

    def _record(metric: str, samples: list[float]) -> TimingRecord:
        med = statistics.median(samples)
        lo, hi = bootstrap_ci(samples)
        return TimingRecord(
            backend=backend.NAME,
            scene=scene.id,
            n_views=scene.n_views,
            n_landmarks=scene.n_landmarks,
            metric=metric,
            median=med,
            ci_lo_95=lo,
            ci_hi_95=hi,
            n_runs=n_runs,
            n_warmup=n_warmup,
        )

    return [
        _record("t_residual_ms", t_r_samples),
        _record("t_gradient_ms", t_g_samples),
        _record("peak_rss_mb", peak_samples),
    ]


# ─── Static-catch suite runner ──────────────────────────────────────────


def run_static_catch_suite(suite_root: Path) -> list[CatchRecord]:
    """Scan suite_root for entries, score each library translation, return records."""
    records: list[CatchRecord] = []
    for entry_dir in sorted(suite_root.iterdir()):
        if not entry_dir.is_dir():
            continue
        entry_id = entry_dir.name
        bug_class = entry_id.split("_", 1)[0]  # e.g. "T1"
        # Each translation file is named <library>.{py,cpp}; reference.json
        # holds the intended output.
        for translation in sorted(entry_dir.iterdir()):
            if translation.name == "reference.json":
                continue
            library = translation.stem
            outcome, diag = _score_translation(translation, entry_dir / "reference.json")
            records.append(
                CatchRecord(
                    library=library,
                    entry_id=entry_id,
                    bug_class=bug_class,
                    outcome=outcome,
                    diagnostic_excerpt=diag[:200],
                )
            )
    return records


def _score_translation(translation: Path, reference: Path) -> tuple[str, str]:
    """Compile/run translation, compare to reference, return (outcome, diagnostic_excerpt).

    B-stage: this is where the actual compile/run/compare logic goes.
    The A-stage skeleton documents the contract: the result is one of
    'CT' / 'RT' / 'Silent' / 'Pass' and the diagnostic is the first
    200 chars of compiler or runtime output.
    """
    # TODO(B-stage): implement compile (for .cpp) / exec (for .py) /
    # tolerance comparison against reference.json.
    return "PENDING", f"scoring not yet implemented for {translation.name}"


# ─── CLI ────────────────────────────────────────────────────────────────


def main(argv: Sequence[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--substrates",
        default="reference,einops",
        help="Comma-separated backend list, or 'all'.",
    )
    parser.add_argument(
        "--scenes",
        default="synthetic_small",
        help="Comma-separated scene list, or 'all'.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("results/results.json"),
        help="Output results.json path.",
    )
    parser.add_argument("--n-runs", type=int, default=30)
    parser.add_argument("--n-warmup", type=int, default=5)
    parser.add_argument(
        "--static-catch-only",
        action="store_true",
        help="Skip BA case study; run only the static-catch suite.",
    )
    parser.add_argument(
        "--suite-root",
        type=Path,
        default=Path("static_catch_suite"),
        help="Static-catch suite root directory.",
    )
    args = parser.parse_args(argv)

    bundle = ResultsBundle(
        metadata={
            "harness_version": "0.1.0-skeleton",
            "paper_section_references": {
                "static_catch": "§6.1, Table 3",
                "ba_case_study": "§6.2, Table 4",
            },
            "n_runs": args.n_runs,
            "n_warmup": args.n_warmup,
        }
    )

    # Static-catch suite.
    suite_root = args.suite_root
    if suite_root.is_dir():
        bundle.catches.extend(run_static_catch_suite(suite_root))
    else:
        print(f"[warn] suite root {suite_root} not found, skipping static-catch", file=sys.stderr)

    # BA case study (skipped if --static-catch-only).
    if not args.static_catch_only:
        from datasets.loader import load_scene  # type: ignore[import-not-found]

        substrate_names = (
            ["reference", "eigen", "webgpu", "einops", "haliax"]
            if args.substrates == "all"
            else args.substrates.split(",")
        )
        scene_names = (
            ["courtyard", "fountain_p11", "ladybug_49", "synthetic_small"]
            if args.scenes == "all"
            else args.scenes.split(",")
        )
        for scene_name in scene_names:
            try:
                scene_loader = load_scene(scene_name)
            except Exception as e:  # noqa: BLE001
                print(f"[skip] scene {scene_name!r}: {e}", file=sys.stderr)
                continue
            for backend_name in substrate_names:
                backend = load_backend(backend_name)
                if backend is None:
                    continue
                bundle.timings.extend(
                    run_ba_timing(backend, scene_loader, n_runs=args.n_runs, n_warmup=args.n_warmup)
                )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(asdict(bundle), indent=2))
    print(f"wrote {args.output} ({len(bundle.timings)} timings, {len(bundle.catches)} catches)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
