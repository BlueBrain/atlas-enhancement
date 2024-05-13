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
fmap_vd = VoxelData(fmap_d, (1, 1))
del fmask

# get pre-images
voxels = fmutil.get_preimages_vox(fmap_vd, pixel_res)
del fmap_vd  # FREE

label_unique = np.array(list(voxels.keys()))  # used flat pixels

# unravel used pixel locations
ix = tuple([np.array([x % pixel_res for x in label_unique]),\
            np.array([x // pixel_res for x in label_unique])])

# 3D neighborhood
ball = nd.generate_binary_structure(3,3)  # 26-connected


def get_lccratio (vox):
    lmsk, nvox = fmutil.get_preimage_mask_vox(vox)        # NOTE: must be ON = 1 ==========
    if lmsk is None:  # empty                             #                               ||
        return 0                                          #                               ||
    prelab, nc = nd.label(lmsk, ball)                     #                               ||
    sizes = nd.sum_labels(lmsk, prelab, range(1,nc + 1))  # volumes as sums over labels <==
    largest_cc_to_total = np.max(sizes) / np.sum(sizes)
    return largest_cc_to_total


# compute
desc = "Pre-image connectedness"
if ncpu == 1:
    result = [get_lccratio(voxels[x]) for x in tqdm(label_unique, desc=desc)]
else:
    from joblib import Parallel, delayed
    from tqdm_joblib_bridge import tqdm_joblib
    with tqdm_joblib(tqdm(desc=desc, total=len(label_unique))) as progress_bar:
        with Parallel(n_jobs=ncpu, prefer='threads') as parallel:
            result = parallel(delayed(get_lccratio)(voxels[x]) for x in label_unique)

del voxels  # FREE

# setup result image
res = np.zeros((pixel_res, pixel_res), dtype=np.float32)
res[ix] = result

# save image
res_vd = VoxelData(res, (1,1))
res_vd.save_nrrd(output_nrrd)
