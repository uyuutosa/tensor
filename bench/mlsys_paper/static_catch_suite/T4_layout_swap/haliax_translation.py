import jax.numpy as jnp
H = B = 8
print(jnp.zeros((H, B, 64)).reshape(B, H, 64).shape)
