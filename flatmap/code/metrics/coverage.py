import voxcell as vc
import numpy as np
import sys

flatmap_file = sys.argv[1]
rdepth_file = sys.argv[2]

fmap_vd = vc.VoxelData.load_nrrd(flatmap_file)
fmap = fmap_vd.raw

rdepth_vd = vc.VoxelData.load_nrrd(rdepth_file)
mask = ~np.isnan(rdepth_vd.raw)

is_unmapped = lambda x: x[0] == -1 or x[1] == -1

n_total = np.sum(mask)
n_not_mapped = np.sum([is_unmapped(x) for x in fmap[mask]])

print('{} {}'.format(n_not_mapped,1.0 - n_not_mapped / n_total))
