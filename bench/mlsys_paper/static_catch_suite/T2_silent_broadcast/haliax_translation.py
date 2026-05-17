import haliax as hax
import jax.numpy as jnp
Batch = hax.Axis("batch", 32); Feat = hax.Axis("feature", 128)
x = hax.named(jnp.zeros((32, 128)), (Batch, Feat))
bias_T = hax.named(jnp.zeros((128, 1)), (Feat, hax.Axis("broadcast", 1)))
y = x.array + bias_T.array.T
print(y.shape)
