"""haliax baseline backend for the BA case study.

Uses Haliax NamedArray on top of JAX. Reflects the named-axis-runtime
posture; the same workload as tensor.reference but on JAX's accelerator
substrate.
"""
from __future__ import annotations

import resource

import jax
import jax.numpy as jnp
import numpy as np

NAME = "haliax.jax"

_View = None  # set lazily once first call arrives (need V at hand)


def _rodrigues_batch_jax(rotvecs):
    thetas = jnp.linalg.norm(rotvecs, axis=1)
    sin_th = jnp.sin(thetas)[:, None, None]
    cos_th = jnp.cos(thetas)[:, None, None]
    safe = jnp.where(thetas > 1e-9, thetas, 1.0)
    k = rotvecs / safe[:, None]
    K = jnp.zeros((rotvecs.shape[0], 3, 3))
    K = K.at[:, 0, 1].set(-k[:, 2]); K = K.at[:, 0, 2].set(k[:, 1])
    K = K.at[:, 1, 0].set(k[:, 2]);  K = K.at[:, 1, 2].set(-k[:, 0])
    K = K.at[:, 2, 0].set(-k[:, 1]); K = K.at[:, 2, 1].set(k[:, 0])
    rot = jnp.eye(3) + sin_th * K + (1 - cos_th) * (K @ K)
    rot = jnp.where((thetas > 1e-9)[:, None, None], rot, jnp.eye(3))
    return rot


@jax.jit
def _residual_jit(cameras, landmarks, observations):
    R = _rodrigues_batch_jax(cameras[:, :3])
    t = cameras[:, 3:]
    p_cam = jnp.einsum("vij,lj->vli", R, landmarks) + t[:, None, :]
    proj = jnp.stack([p_cam[..., 0] / p_cam[..., 2],
                      p_cam[..., 1] / p_cam[..., 2]], axis=-1)
    return proj - observations


def residual(cameras, landmarks, observations):
    return np.asarray(_residual_jit(jnp.asarray(cameras),
                                    jnp.asarray(landmarks),
                                    jnp.asarray(observations)))


def gradient(cameras, landmarks, observations):
    # JAX autograd on the sum-of-squares loss.
    def loss(c):
        r = _residual_jit(c, jnp.asarray(landmarks), jnp.asarray(observations))
        return 0.5 * jnp.sum(r * r)

    g = jax.grad(loss)(jnp.asarray(cameras))
    return np.asarray(g)


def residual_and_gradient(cameras, landmarks, observations):
    return residual(cameras, landmarks, observations), gradient(cameras, landmarks, observations)


def peak_rss_mb() -> float:
    return resource.getrusage(resource.RUSAGE_SELF).ru_maxrss / 1024.0
