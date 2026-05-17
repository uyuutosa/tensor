"""tensor.reference backend driver (synthetic-data smoke path).

Implements BackendRunner against the tensor library's `reference`
KernelBackend adapter (the portable nested-loop CPU substrate, see
include/tensor/core/backend/reference.hpp). At A-stage this driver
runs the BA reprojection in plain numpy so the harness completes
end-to-end; the B-stage swap replaces the inner residual call with
`tensor::tex::Evaluator` invoking the C++ reference backend through
the Python bindings, which gives a real wall-clock measurement of the
named-axis machinery.
"""
from __future__ import annotations

import resource

import numpy as np

NAME = "tensor.reference"


def _rodrigues(rotvec: np.ndarray) -> np.ndarray:
    theta = np.linalg.norm(rotvec)
    if theta < 1e-9:
        return np.eye(3)
    k = rotvec / theta
    K = np.array([[0, -k[2], k[1]], [k[2], 0, -k[0]], [-k[1], k[0], 0]])
    return np.eye(3) + np.sin(theta) * K + (1 - np.cos(theta)) * (K @ K)


def _project(cameras: np.ndarray, landmarks: np.ndarray) -> np.ndarray:
    V = cameras.shape[0]
    L = landmarks.shape[0]
    out = np.empty((V, L, 2))
    for v in range(V):
        R = _rodrigues(cameras[v, :3])
        t = cameras[v, 3:]
        p_cam = (R @ landmarks.T).T + t
        out[v, :, 0] = p_cam[:, 0] / p_cam[:, 2]
        out[v, :, 1] = p_cam[:, 1] / p_cam[:, 2]
    return out


def residual(cameras: np.ndarray, landmarks: np.ndarray, observations: np.ndarray) -> np.ndarray:
    """r_{vl} = π(R_v p_l + t_v) - o_{vl}.

    A-stage: plain numpy. B-stage: route through tensor.tex.Evaluator
    bound to the reference KernelBackend so we measure the named-axis
    Evaluator's actual cost.
    """
    return _project(cameras, landmarks) - observations


def gradient(cameras: np.ndarray, landmarks: np.ndarray, observations: np.ndarray):
    """Numerical-Jacobian placeholder.

    A-stage: finite-difference d_r/d_camera for one camera so the
    harness has something to time. B-stage: replaces with the
    autograd-derived Jacobian assembled via the contract operator
    on the reference KernelBackend.
    """
    eps = 1e-4
    base = residual(cameras, landmarks, observations)
    grad = np.zeros_like(cameras)
    for v in range(cameras.shape[0]):
        for k in range(6):
            cameras[v, k] += eps
            up = residual(cameras, landmarks, observations)
            cameras[v, k] -= eps
            grad[v, k] = np.sum((up - base) * base) / eps
    return grad


def residual_and_gradient(cameras, landmarks, observations):
    return residual(cameras, landmarks, observations), gradient(cameras, landmarks, observations)


def peak_rss_mb() -> float:
    """Peak RSS observed for this process in MB. Linux + macOS only."""
    rusage = resource.getrusage(resource.RUSAGE_SELF)
    # ru_maxrss is KB on Linux, bytes on macOS; we assume Linux for CI.
    return rusage.ru_maxrss / 1024.0
