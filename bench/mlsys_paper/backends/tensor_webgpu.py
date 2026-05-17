"""tensor.webgpu backend (GPU, cupy stand-in for Dawn/WebGPU).

The real WebGPU substrate routes through Dawn. On this runner we have
CUDA via cupy, which is the closest accessible GPU substrate that
shares the same "ship the data to the device, run kernel, fetch back"
model as WebGPU. The B-stage swap targets actual WebGPU via Dawn; the
timing characteristics (data-transfer overhead + GPU kernel +
fetch-back) are qualitatively shared.
"""
from __future__ import annotations

import resource

NAME = "tensor.webgpu"

try:
    import cupy as cp
    _AVAILABLE = True
except ImportError:
    _AVAILABLE = False


def _check():
    if not _AVAILABLE:
        raise RuntimeError("cupy not installed; WebGPU stand-in unavailable")


def _rodrigues_batch_gpu(rotvecs):
    thetas = cp.linalg.norm(rotvecs, axis=1)
    out = cp.tile(cp.eye(3), (rotvecs.shape[0], 1, 1))
    nonzero = thetas > 1e-9
    k = rotvecs[nonzero] / thetas[nonzero, None]
    K = cp.zeros((int(nonzero.sum()), 3, 3))
    K[:, 0, 1] = -k[:, 2]; K[:, 0, 2] = k[:, 1]
    K[:, 1, 0] = k[:, 2];  K[:, 1, 2] = -k[:, 0]
    K[:, 2, 0] = -k[:, 1]; K[:, 2, 1] = k[:, 0]
    sin_th = cp.sin(thetas[nonzero])[:, None, None]
    cos_th = cp.cos(thetas[nonzero])[:, None, None]
    out[nonzero] = cp.eye(3) + sin_th * K + (1 - cos_th) * (K @ K)
    return out


def _project_gpu(cameras, landmarks):
    R = _rodrigues_batch_gpu(cameras[:, :3])
    t = cameras[:, 3:]
    p_cam = cp.einsum("vij,lj->vli", R, landmarks) + t[:, None, :]
    return cp.stack([p_cam[..., 0] / p_cam[..., 2],
                     p_cam[..., 1] / p_cam[..., 2]], axis=-1)


def residual(cameras, landmarks, observations):
    _check()
    return cp.asnumpy(
        _project_gpu(cp.asarray(cameras), cp.asarray(landmarks))
        - cp.asarray(observations)
    )


def gradient(cameras, landmarks, observations):
    _check()
    eps = 1e-4
    cam_d = cp.asarray(cameras).copy()
    lm_d = cp.asarray(landmarks); obs_d = cp.asarray(observations)
    base = _project_gpu(cam_d, lm_d) - obs_d
    grad = cp.zeros_like(cam_d)
    for k in range(cam_d.shape[1]):
        cam_d[:, k] += eps
        up = _project_gpu(cam_d, lm_d) - obs_d
        cam_d[:, k] -= eps
        grad[:, k] = (up - base).reshape(cam_d.shape[0], -1).sum(axis=1) / eps
    return cp.asnumpy(grad)


def residual_and_gradient(cameras, landmarks, observations):
    return residual(cameras, landmarks, observations), gradient(cameras, landmarks, observations)


def peak_rss_mb() -> float:
    rss = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024.0
    if _AVAILABLE:
        # Add cupy device memory pool footprint.
        rss += cp.get_default_memory_pool().used_bytes() / (1024**2)
    return rss
