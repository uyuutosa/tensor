"""T3 reduction over wrong axis — einops translation.

einops.reduce(x, "b c -> c", "sum") collapses `b` correctly when the
pattern names match the operand's intended layout, but there is no
mechanism to assert "this operand really is (batch, classes)" — a
caller that wrote the pattern backwards reduces over the wrong axis
and the result has the wrong meaning at the right rank.
"""
import einops
import numpy as np

logits = np.zeros((16, 10))      # (batch, classes)
# Bug: pattern reduces over `c` (classes) but the variable name
# (loss_per_class) reveals the writer thought they kept classes.
loss_per_class = einops.reduce(logits, "b c -> b", "sum")
print(loss_per_class.shape)        # (16,) — wrong axis collapsed
