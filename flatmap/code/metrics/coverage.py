from voxcell import VoxelData
import numpy as np
import sys

import flatmap_util as fmutil

flatmap_nrrd = sys.argv[1]
rdepth_nrrd = sys.argv[2]

fmap_vd, fmask = fmutil.load_flatmap(flatmap_nrrd)

rdepth_vd = VoxelData.load_nrrd(rdepth_nrrd)
mask = ~np.isnan(rdepth_vd.raw)

n_total = np.sum(mask)
n_mapped = np.sum(fmask)
n_not_mapped = n_total - n_mapped

print('{} {}'.format(n_not_mapped,n_mapped / n_total))
