"""Scene loading interface.

Real scenes (ETH3D 'courtyard', Strecha 'fountain_p11', BAL 'ladybug_49')
are fetched via datasets/downloader.py and converted to a uniform
Scene tuple. A synthetic 'synthetic_small' scene is included here so
the harness runs end-to-end without external data on a CI machine
or laptop.
"""
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import numpy as np


@dataclass
class Scene:
    """Uniform interface across ETH3D / Strecha / BAL / synthetic."""

    id: str
    n_views: int
    n_landmarks: int
    cameras: np.ndarray  # shape (V, 6): (rotvec[3], translation[3]) per view
    landmarks: np.ndarray  # shape (L, 3): (x, y, z) per landmark
    observations: np.ndarray  # shape (V, L, 2): (u, v) normalised image coord


@dataclass
class SyntheticSmallLoader:
    """5-view × 20-landmark scene with Gaussian noise on observations.

    Used for the harness smoke test; the named-axis structure is the
    same as the real scenes, so this exercises the backend dispatch +
    metric collection path without needing external data.
    """

    id: str = "synthetic_small"
    n_views: int = 5
    n_landmarks: int = 20
    noise_sigma: float = 0.001

    def load(self) -> Scene:
        rng = np.random.default_rng(42)
        cameras = rng.normal(scale=0.1, size=(self.n_views, 6))
        landmarks = rng.normal(loc=[0.0, 0.0, 5.0], scale=1.0, size=(self.n_landmarks, 3))
        # Project landmarks through each camera (pinhole, identity intrinsics).
        observations = np.empty((self.n_views, self.n_landmarks, 2))
        for v in range(self.n_views):
            # Rodrigues for the rotation matrix.
            rotvec = cameras[v, :3]
            t = cameras[v, 3:]
            theta = np.linalg.norm(rotvec)
            if theta > 1e-9:
                k = rotvec / theta
                K = np.array([[0, -k[2], k[1]], [k[2], 0, -k[0]], [-k[1], k[0], 0]])
                R = np.eye(3) + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)
            else:
                R = np.eye(3)
            p_cam = (R @ landmarks.T).T + t
            observations[v, :, 0] = p_cam[:, 0] / p_cam[:, 2]
            observations[v, :, 1] = p_cam[:, 1] / p_cam[:, 2]
        observations += rng.normal(scale=self.noise_sigma, size=observations.shape)
        return Scene(
            id=self.id,
            n_views=self.n_views,
            n_landmarks=self.n_landmarks,
            cameras=cameras,
            landmarks=landmarks,
            observations=observations,
        )


@dataclass
class SyntheticPaperScaleLoader:
    """50-view × 200-landmark synthetic scene matching the paper's
    nominal BA workload. Same geometry as synthetic_small, just larger
    so the timing numbers exercise the substrates' scaling
    characteristics meaningfully."""

    id: str = "synthetic_paper_scale"
    n_views: int = 50
    n_landmarks: int = 200
    noise_sigma: float = 0.001

    def load(self) -> Scene:
        loader = SyntheticSmallLoader(
            id=self.id,
            n_views=self.n_views,
            n_landmarks=self.n_landmarks,
            noise_sigma=self.noise_sigma,
        )
        return loader.load()


def load_scene(name: str):
    """Return a loader object whose .load() produces a Scene."""
    if name == "synthetic_small":
        return SyntheticSmallLoader()
    if name == "synthetic_paper_scale":
        return SyntheticPaperScaleLoader()
    # Real-scene loaders (ETH3D / Strecha / BAL) dispatch through
    # datasets.downloader.
    raise NotImplementedError(
        f"scene loader for {name!r} not yet implemented; available: "
        "synthetic_small (5×20), synthetic_paper_scale (50×200)"
    )
