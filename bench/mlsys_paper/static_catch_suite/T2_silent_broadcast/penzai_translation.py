import jax.numpy as jnp
from penzai import pz
x = pz.nx.wrap(jnp.zeros((32, 128))).tag("batch", "feature")
bias_T = pz.nx.wrap(jnp.zeros((128, 1))).tag("feature", "broadcast")
y = x.untag("batch", "feature").data_array + bias_T.untag("feature", "broadcast").data_array.T
print(y.shape)
