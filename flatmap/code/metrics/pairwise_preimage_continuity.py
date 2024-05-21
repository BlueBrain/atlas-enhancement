import numpy as np

from voxcell import VoxelData
from os import cpu_count
import sys

from tqdm import tqdm
import scipy.ndimage as nd

import flatmap_util as fmutil

flatmap_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])
output_nrrd = sys.argv[3]

ncpu = cpu_count() // 2
if len(sys.argv) > 4:
    ncpu = int(sys.argv[4])

# load and discretize flat map
fmap_vd, fmask = fmutil.load_flatmap(flatmap_nrrd)
fmap_d = fmutil.discretize_flatmap(fmap_vd, fmask, pixel_res)
fmap_vd = fmap_vd.with_data(fmap_d)
del fmask

# get pre-images
voxels = fmutil.get_preimages_vox(fmap_vd, pixel_res)
del fmap_vd  # FREE

label_unique = np.array(list(voxels.keys()))  # used flat pixels

# 3D neighborhood
ball = nd.generate_binary_structure(3,3)  # 26-connected

# label pairs
pairs_horiz = [(x + pixel_res * y, (x + 1) + pixel_res * y) \
               for y in range(0, pixel_res) for x in range(0, pixel_res - 1)]
pairs_vert  = [(x + pixel_res * y, x + pixel_res * (y + 1)) \
               for x in range(0, pixel_res) for y in range(0, pixel_res - 1)]


def get_pair_lccratio (vox):
    lmsk, nvox = fmutil.get_preimage_mask_vox(vox)
    if lmsk is None:  # empty
        return 0
    prelab, nc = nd.label(lmsk, ball)
    sizes = nd.sum_labels(lmsk, prelab, range(1,nc + 1))
    largest_cc_to_total = np.max(sizes) / np.sum(sizes)
    return largest_cc_to_total


# compute
desc_h = "Pairwise pre-image continuity (X)"
desc_v = "Pairwise pre-image continuity (Y)"
if ncpu == 1:
    result_h = [get_pair_lccratio(voxels[x[0]] + voxels[x[1]]) for x in tqdm(pairs_horiz, desc=desc_h)]
    result_v = [get_pair_lccratio(voxels[y[0]] + voxels[y[1]]) for y in tqdm(pairs_vert, desc=desc_v)]
else:
    from joblib import Parallel, delayed
    from tqdm_joblib_bridge import tqdm_joblib
    with tqdm_joblib(tqdm(desc=desc_h, total=len(pairs_horiz))) as progress_bar:
        with Parallel(n_jobs=ncpu, prefer='threads') as parallel:
            result_h = parallel(delayed(get_pair_lccratio)(voxels[x[0]] + voxels[x[1]]) for x in pairs_horiz)
    with tqdm_joblib(tqdm(desc=desc_v, total=len(pairs_vert))) as progress_bar:
        with Parallel(n_jobs=ncpu, prefer='threads') as parallel:
            result_v = parallel(delayed(get_pair_lccratio)(voxels[y[0]] + voxels[y[1]]) for y in pairs_vert)

# setup result image
nc_l = np.full((pixel_res, pixel_res), 1.0, dtype=np.float32)  # lccratio in pre-image w/left neighbor
nc_r = np.full((pixel_res, pixel_res), 1.0, dtype=np.float32)  # lccratio in pre-image w/right neighbor
nc_t = np.full((pixel_res, pixel_res), 1.0, dtype=np.float32)  # lccratio in pre-image w/top neighbor
nc_b = np.full((pixel_res, pixel_res), 1.0, dtype=np.float32)  # lccratio in pre-image w/bottom neighbor

## store pairwise results
for p, r in zip(pairs_horiz, result_h):            # same result for:
    nc_l[p[1] % pixel_res, p[1] // pixel_res] = r  # left neighbor of right pixel
    nc_r[p[0] % pixel_res, p[0] // pixel_res] = r  # right neighbor of left pixel

for p, r in zip(pairs_vert, result_v):             # same result for:
    nc_t[p[1] % pixel_res, p[1] // pixel_res] = r  # top neighbor of bottom pixel
    nc_b[p[0] % pixel_res, p[0] // pixel_res] = r  # bottom neighbor of top pixel

## compute minimum ratio (worse-case) of largest CC to total over pairwise pre-images with 4-neighbors
res = np.minimum.reduce([nc_l, nc_r, nc_t, nc_b])

# save image
res_vd = VoxelData(res, (1,1))
res_vd.save_nrrd(output_nrrd)
