import numpy as np
import sys

import flatmap_util as fmutil

flatmap_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])

fmap_vd, fmask = fmutil.load_flatmap(flatmap_nrrd)
fx_d, fy_d = fmutil.get_discrete_flat_coordinates(fmap_vd.raw, pixel_res, fmask)
pix_uniq = np.unique(np.vstack((fx_d, fy_d)).T, axis=0)

print(pixel_res,len(pix_uniq) / (pixel_res * pixel_res))
