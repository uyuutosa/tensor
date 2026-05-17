import jax.numpy as jnp
print(jnp.argmax(jnp.zeros((32, 10)), axis=0).shape)
