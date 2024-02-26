import numpy as np
from voxcell import VoxelData
import sys

from scipy.ndimage import distance_transform_edt

rdepth_nrrd = sys.argv[1]
thresh = float(sys.argv[2])
side = sys.argv[3]
output_nrrd = sys.argv[4]

if side not in ['top','bottom']:
    raise ValueError('side must be "top" or "bottom"')

vd = VoxelData.load_nrrd(rdepth_nrrd)
rdepth = vd.raw

bnd = np.zeros_like(rdepth, dtype=np.uint8)

if side == 'top':
    flt = (rdepth >= thresh) | np.isnan(rdepth)
    edt = distance_transform_edt(flt, return_distances=True)
    bnd[(edt == 1) & (rdepth >= thresh)] = 1
else:
    flt = (rdepth <= thresh) | np.isnan(rdepth)
    edt = distance_transform_edt(flt, return_distances=True)
    bnd[(edt == 1) & (rdepth <= thresh)] = 1

vd.with_data(bnd).save_nrrd(output_nrrd)
