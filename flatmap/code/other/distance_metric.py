import numpy as np
from sys import argv

argc = len(argv)
fit_r0_file = argv[1]
fit_r1_file = argv[2]
fit_err_file = argv[3]
fxlen = int(argv[4])

fit_r0 = np.fromfile(fit_r0_file, dtype=np.float32).reshape((fxlen, fxlen))
fit_r1 = np.fromfile(fit_r1_file, dtype=np.float32).reshape((fxlen, fxlen))
fit_err = np.fromfile(fit_err_file, dtype=np.float32).reshape((fxlen, fxlen))

valid_msk = (fit_err != -1)  # sentinel for failed fit

rratio = np.full((fxlen, fxlen), -1, dtype=np.float32)           # -1 is sentinel for failed fit
rratio[valid_msk] = 1.0 - fit_r0[valid_msk] / fit_r1[valid_msk]  # 1 - r0 / r1 in [0,1]
rratio.astype(np.float32).tofile('rratio_{}.img'.format(fxlen))  # no transpose

rmean = np.full((fxlen, fxlen), -1, dtype=np.float32)            # -1 is sentinel for failed fit
rmean[valid_msk] = (fit_r0[valid_msk] + fit_r1[valid_msk]) / 2.0 # average radius
rmean.astype(np.float32).tofile('rmean_{}.img'.format(fxlen))    # no transpose
