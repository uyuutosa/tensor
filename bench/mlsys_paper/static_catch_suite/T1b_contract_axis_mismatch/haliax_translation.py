import jax.numpy as jnp
print(jnp.einsum("ik,kj->ij", jnp.zeros((3, 4)), jnp.zeros((4, 5))).shape)
