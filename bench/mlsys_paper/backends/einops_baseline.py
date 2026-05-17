"""einops baseline backend for the BA case study.

Expresses the projection contraction via einops.einsum on top of
numpy. Demonstrates how the named-axis discipline ports across the
two libraries (same dimension labels, different host machinery).
"""
from __future__ import annotations

import resource

import einops
import numpy as np

NAME = "einops.numpy"


def _rodrigues_batch(rotvecs):
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


def residual(cameras, landmarks, observations):
    R = _rodrigues_batch(cameras[:, :3])
    t = cameras[:, 3:]
    # einops named contraction: R has axes (view, row, col); landmarks (landmark, col)
    p_cam = einops.einsum(R, landmarks, "view row col, landmark col -> view landmark row") + \
            einops.rearrange(t, "view row -> view 1 row")
    u = p_cam[..., 0] / p_cam[..., 2]
    v = p_cam[..., 1] / p_cam[..., 2]
    return np.stack([u, v], axis=-1) - observations


def gradient(cameras, landmarks, observations):
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
