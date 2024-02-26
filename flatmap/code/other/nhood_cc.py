import numpy as np

from voxcell import VoxelData
from sys import argv, exit, stderr
from os import cpu_count
from tqdm import tqdm

import scipy.ndimage as nd

import flatmap_util as fmutil

argc = len(argv)
if argc < 3:
    print('Usage: {} <flatmap> <flat xlen>'.format(argv[0]),file = stderr)
    exit()

flatmap_file = argv[1]
fxlen = int(argv[2])

ncpu = cpu_count() // 2
if argc > 3:
    ncpu = int(argv[3])

# load flat map
fmap_vd = VoxelData.load_nrrd(flatmap_file)
fmap = fmap_vd.raw
assert(fmap.dtype == np.float32)  # we need float flatmap

fx, fy = fmutil.get_discrete_flat_coordinates(fmap, fxlen)

labels = fx + fxlen * fy  # ravel all
label_unique = np.unique(labels[labels >= 0])  # used flat pixels
ball = nd.generate_binary_structure(3,3)  # all neighbors in 3D

pairs_horiz = [(x + fxlen * y, (x + 1) + fxlen * y) for y in range(0, fxlen) for x in range(0, fxlen - 1)]
pairs_vert  = [(x + fxlen * y, x + fxlen * (y + 1)) for x in range(0, fxlen) for y in range(0, fxlen - 1)]


def get_pair_ncomp (pair):
    lmsk, nvox = fmutil.get_preimage_mask(pair, labels)
    if lmsk is None:  # empty
        return 0
    _, nc = nd.label(lmsk, ball)
    return nc


if ncpu == 1:
    result_h = [get_pair_ncomp(x) for x in tqdm(pairs_horiz, desc="Computing pairwise CCs (horiz)")]
    result_v = [get_pair_ncomp(x) for x in tqdm(pairs_vert, desc="Computing pairwise CCs (vert)")]
else:
    from joblib import Parallel, delayed
    from tqdm_joblib_bridge import tqdm_joblib
    with tqdm_joblib(tqdm(desc="Computing pairwise CCs (horiz)", total=len(pairs_horiz))) as progress_bar:
        with Parallel(n_jobs=ncpu, prefer='threads') as parallel:
            result_h = parallel(delayed(get_pair_ncomp)(x) for x in pairs_horiz)
    with tqdm_joblib(tqdm(desc="Computing pairwise CCs (vert)", total=len(pairs_vert))) as progress_bar:
        with Parallel(n_jobs=ncpu, prefer='threads') as parallel:
            result_v = parallel(delayed(get_pair_ncomp)(x) for x in pairs_vert)

nc_l = np.zeros((fxlen, fxlen), dtype=np.float32)  # num CCs in pre-image w/left neighbor
nc_r = np.zeros((fxlen, fxlen), dtype=np.float32)  # num CCs in pre-image w/right neighbor
nc_t = np.zeros((fxlen, fxlen), dtype=np.float32)  # num CCs in pre-image w/top neighbor
nc_b = np.zeros((fxlen, fxlen), dtype=np.float32)  # num CCs in pre-image w/bottom neighbor

for p, r in zip(pairs_horiz, result_h):
    nc_l[p[1] % fxlen, p[1] // fxlen] = r  # left neighbor of right pixel
    nc_r[p[0] % fxlen, p[0] // fxlen] = r  # right neighbor of left pixel

for p, r in zip(pairs_vert, result_v):
    nc_t[p[1] % fxlen, p[1] // fxlen] = r  # top neighbor of bottom pixel
    nc_b[p[0] % fxlen, p[0] // fxlen] = r  # bottom neighbor of top pixel

# nc_l.T.astype(np.float32).tofile('nhoodcc_l_{}.img'.format(fxlen))
# nc_r.T.astype(np.float32).tofile('nhoodcc_r_{}.img'.format(fxlen))
# nc_t.T.astype(np.float32).tofile('nhoodcc_t_{}.img'.format(fxlen))
# nc_b.T.astype(np.float32).tofile('nhoodcc_b_{}.img'.format(fxlen))

# OUTPUT maximum (worse) number of connected components in pairwise pre-images with 4-neighbors
nc = np.maximum.reduce([nc_l, nc_r, nc_t, nc_b])
nc.T.astype(np.float32).tofile('nhoodcc_{}.img'.format(fxlen))
