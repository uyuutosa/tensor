"""T1 transposed contraction — Penzai NamedArray translation."""
import jax.numpy as jnp
from penzai import pz

B, H, T, D = 2, 4, 4, 16
Q = pz.nx.wrap(jnp.zeros((B, T, H, D))).tag("batch", "time", "head", "dim")
K = pz.nx.wrap(jnp.zeros((B, H, T, D))).tag("batch", "head", "time", "dim")
# Buggy contraction: writer thought Q has (B,H,T,D). With T==H the
# shapes line up but the semantics are wrong; Penzai's named dot does
# detect the name mismatch in some paths, but here we untag to mimic
# the common workflow of dropping names for performance.
scores = jnp.einsum("bhtd,bhsd->bhts",
                    Q.untag("batch", "time", "head", "dim").data_array,
                    K.untag("batch", "head", "time", "dim").data_array)
print(scores.shape)
