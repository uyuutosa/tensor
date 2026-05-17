import jax.numpy as jnp
logits = jnp.zeros((16, 10))
loss_per_class = logits.sum(axis=0)  # writer meant axis="classes"; got batch reduction
print(loss_per_class.shape)
