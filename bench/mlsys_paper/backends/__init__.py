"""Backend runner Protocol.

Every backend module under bench/mlsys_paper/backends/ exposes a
top-level NAME (str) plus the methods declared in BackendRunner. The
driver dispatches by attribute lookup, not by inheritance, so a
subprocess-bridged backend (e.g. the C++ tensor backends called via
nanobind) is interchangeable with a pure-Python one (einops, haliax).
"""
from __future__ import annotations

from typing import Protocol, runtime_checkable


@runtime_checkable
class BackendRunner(Protocol):
    """Each backend exposes these. Inputs are uniform (numpy arrays);
    the backend converts to its native tensor type internally."""

    NAME: str

    def residual(self, cameras, landmarks, observations):
        """Forward pass producing the (V, L, 2) reprojection residual."""
        ...

    def gradient(self, cameras, landmarks, observations):
        """Backward / Jacobian pass producing dL/d(cameras, landmarks)."""
        ...

    def residual_and_gradient(self, cameras, landmarks, observations):
        """Combined forward + backward; used during warm-up."""
        ...

    def peak_rss_mb(self) -> float:
        """Peak resident-set memory observed since the last
        residual_and_gradient call, in megabytes."""
        ...
