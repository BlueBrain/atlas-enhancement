import numpy as np
from voxcell import VoxelData
import sys

from scipy.ndimage import distance_transform_edt


def is_background(array, val = np.nan):
    return np.isnan(array) if np.isnan(val) else (array == val)


# CLI interface
rdepth_nrrd = sys.argv[1]
thresh = float(sys.argv[2])

side = sys.argv[3]
if side not in ['top','bottom']:
    raise ValueError('side must be "top" or "bottom"')

output_nrrd = sys.argv[4]

bkg_val = np.nan
if len(sys.argv) > 5:
    bkg_val = np.float32(sys.argv[5])


# load data
vd = VoxelData.load_nrrd(rdepth_nrrd)
rdepth = vd.raw

# get bounding box
xyz = ['x', 'y', 'z']
w = np.where(~is_background(rdepth,bkg_val))
bnds = {v: (np.min(w[i]), np.max(w[i])) for i, v in enumerate(xyz) }
shape = tuple([v[1] - v[0] + 1 for k, v in bnds.items()])
shape_pad = tuple([x + 2 for x in shape])  # add padding
wnew = tuple([w[i] - bnds[v][0] for i, v in enumerate(xyz)])
wnew_pad = tuple([x + 1 for x in wnew])

# bounding box sized (plus 1-padding)
rdepth_bnd = np.full(shape_pad, bkg_val, dtype=np.float32)
rdepth_bnd[wnew_pad] = rdepth[w]
msk_bkg = is_background(rdepth_bnd,bkg_val)

# costly EDT
bnd = np.zeros_like(rdepth_bnd, dtype=np.uint8)
if side == 'top':
    msk = (rdepth_bnd >= thresh)
    flt = msk | msk_bkg
    edt = distance_transform_edt(flt, return_distances=True)
    bnd[(edt == 1) & msk] = 1
else:
    msk = (rdepth_bnd <= thresh)
    flt = msk | msk_bkg
    edt = distance_transform_edt(flt, return_distances=True)
    bnd[(edt == 1) & msk] = 1

# copy back to full-size
bnd_full = np.zeros_like(rdepth, dtype=np.uint8)
bnd_full[w] = bnd[wnew]

# save result
vd.with_data(bnd_full).save_nrrd(output_nrrd)
