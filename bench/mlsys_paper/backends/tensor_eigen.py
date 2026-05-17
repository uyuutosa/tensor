"""tensor.eigen backend (CPU, BLAS-backed via numpy).

Stand-in for the C++ Eigen substrate that uses numpy's BLAS-backed
matmul instead of nested Python loops. A real B-stage swap would
route through the Eigen kernel; for the A-stage harness this gives
a "Eigen-like" vectorised CPU number against which the reference
backend can be compared. The named-axis structure is identical
across both backends — only the inner compute differs.
"""
from __future__ import annotations

import resource

import numpy as np

NAME = "tensor.eigen"


def _rodrigues_batch(rotvecs: np.ndarray) -> np.ndarray:
    """Vectorised Rodrigues over a (V, 3) batch."""
    thetas = np.linalg.norm(rotvecs, axis=1)
    out = np.tile(np.eye(3), (rotvecs.shape[0], 1, 1))
    nonzero = thetas > 1e-9
    k = rotvecs[nonzero] / thetas[nonzero, None]
    K = np.zeros((nonzero.sum(), 3, 3))
    K[:, 0, 1] = -k[:, 2]; K[:, 0, 2] = k[:, 1]
    K[:, 1, 0] = k[:, 2];  K[:, 1, 2] = -k[:, 0]
    K[:, 2, 0] = -k[:, 1]; K[:, 2, 1] = k[:, 0]
    sin_th = np.sin(thetas[nonzero])[:, None, None]
    cos_th = np.cos(thetas[nonzero])[:, None, None]
    out[nonzero] = np.eye(3) + sin_th * K + (1 - cos_th) * (K @ K)
    return out


def _project_vec(cameras: np.ndarray, landmarks: np.ndarray) -> np.ndarray:
    R = _rodrigues_batch(cameras[:, :3])           # (V, 3, 3)
    t = cameras[:, 3:]                              # (V, 3)
    # (V, L, 3) = (V, 3, 3) @ (L, 3).T then add (V, 1, 3)
    p_cam = np.einsum("vij,lj->vli", R, landmarks) + t[:, None, :]
    return np.stack([p_cam[..., 0] / p_cam[..., 2],
                     p_cam[..., 1] / p_cam[..., 2]], axis=-1)


def residual(cameras, landmarks, observations):
    return _project_vec(cameras, landmarks) - observations


def gradient(cameras, landmarks, observations):
    """Finite-difference Jacobian, vectorised."""
    eps = 1e-4
    base = residual(cameras, landmarks, observations)
    grad = np.zeros_like(cameras)
    for k in range(cameras.shape[1]):
        cameras[:, k] += eps
        up = residual(cameras, landmarks, observations)
        cameras[:, k] -= eps
        grad[:, k] = (up - base).reshape(cameras.shape[0], -1).sum(axis=1) / eps
    return grad


def residual_and_gradient(cameras, landmarks, observations):
    return residual(cameras, landmarks, observations), gradient(cameras, landmarks, observations)


def peak_rss_mb() -> float:
    return resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024.0
