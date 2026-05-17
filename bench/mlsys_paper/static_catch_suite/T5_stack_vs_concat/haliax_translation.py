import jax.numpy as jnp
print(jnp.concatenate([jnp.zeros((16, 64)), jnp.zeros((16, 64))], axis=0).shape)
