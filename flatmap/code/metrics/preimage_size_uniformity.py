from voxcell import VoxelData
import numpy as np
import sys

import flatmap_util as fmutil

flatmap_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])
output_nrrd = sys.argv[3]

fmap_vd, fmask = fmutil.load_flatmap(flatmap_nrrd)
fx_d, fy_d = fmutil.get_discrete_flat_coordinates(fmap_vd.raw, pixel_res, fmask)
pix_uniq, counts = np.unique(np.vstack((fx_d, fy_d)).T, axis=0, return_counts=True)

n_eligible = np.sum(fmask)
frac = counts / n_eligible
frac_uniform = 1.0 / (pixel_res * pixel_res)

res = np.zeros((pixel_res, pixel_res), dtype=np.float32)
res[tuple(pix_uniq.T)] = frac / frac_uniform

res_vd = VoxelData(res, (1,1))
res_vd.save_nrrd(output_nrrd)
