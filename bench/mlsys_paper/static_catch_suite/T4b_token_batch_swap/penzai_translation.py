import jax.numpy as jnp
T = B = 32
print(jnp.zeros((T, B, 256)).reshape(B, T, 256).shape)
