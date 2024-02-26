import numpy as np
from sys import argv, exit, stderr

argc = len(argv)
nvox_file = argv[1]
fxlen = int(argv[2])

nvox = np.fromfile(nvox_file, dtype=np.float32).reshape((fxlen, fxlen))

assert(np.min(nvox) >= 0.0)

nvox1 = np.sum(nvox > 0)

print('{} {}'.format(fxlen,nvox1 / nvox.size))
