import numpy as np
token0 = np.zeros((16, 64)); token1 = np.zeros((16, 64))
result = np.concatenate([token0, token1], axis=0)  # rank-2, intent was rank-3
print(result.shape)
