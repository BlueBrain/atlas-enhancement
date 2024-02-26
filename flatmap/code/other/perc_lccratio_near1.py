import numpy as np
from sys import argv, exit, stderr

argc = len(argv)
lccratio_file = argv[1]
fxlen = int(argv[2])

lccratio = np.fromfile(lccratio_file, dtype=np.float32).reshape((fxlen, fxlen))

assert(np.min(lccratio) >= 0.0)

lccratio1 = np.sum(np.abs(lccratio - 1.0) <= 0.05)

print('{} {}'.format(fxlen,lccratio1 / lccratio.size))
