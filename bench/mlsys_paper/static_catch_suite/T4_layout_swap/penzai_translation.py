import jax.numpy as jnp
H = B = 8
upstream = jnp.zeros((H, B, 64))
view = upstream.reshape(B, H, 64)  # no-op shape, wrong semantics
print(view.shape)
