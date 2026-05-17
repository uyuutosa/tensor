import jax.numpy as jnp
token0 = jnp.zeros((16, 64)); token1 = jnp.zeros((16, 64))
result = jnp.concatenate([token0, token1], axis=0)  # rank-2 not rank-3
print(result.shape)
