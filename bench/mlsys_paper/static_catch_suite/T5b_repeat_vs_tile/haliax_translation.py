import jax.numpy as jnp
print(jnp.tile(jnp.zeros((32, 64)), (2, 1)).shape)
