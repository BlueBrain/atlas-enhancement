import numpy as np
from sys import argv, exit, stderr

argc = len(argv)
ncomp_file = argv[1]
fxlen = int(argv[2])

ncomp = np.fromfile(ncomp_file, dtype=np.float32).reshape((fxlen, fxlen))

assert(np.min(ncomp) >= 0.0)

ncomp1 = np.sum(ncomp == 1)

print('{} {}'.format(fxlen,ncomp1 / ncomp.size))
