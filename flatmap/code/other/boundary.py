#!/bin/env python3

import voxcell as vc
import numpy as np
from scipy.ndimage.morphology import distance_transform_edt
import sys

atlas_hierarchy = sys.argv[1]
atlas_regions = sys.argv[2]
layer = int(sys.argv[3])
side = sys.argv[4]
output = sys.argv[5]
parentRegion = sys.argv[6]

# TODO: get rid of generating layers, instead read from compressed matrix
# with gzip.open(filename, 'rb') as f:
#   lay = np.frombuffer(f.read(), dtype=np.uint16)

rmap = vc.RegionMap.load_json(atlas_hierarchy)
vd = vc.VoxelData.load_nrrd(atlas_regions)
# all children of parent region
childRegions = rmap.find('@^{}$'.format(parentRegion), "acronym", with_descendants=True)

# Get layers
lay = np.zeros_like(vd.raw)
for i in range(1, 7):
    # search for 'layer %' in name
    ids = [x for x in childRegions if 'layer {}'.format(i) in rmap.get(x, 'name').lower()]
    msk = np.isin(vd.raw, ids)
    lay[msk] = i

# TODO: beware of boundaries with more than one connected component!

# Usage of distance_transform_edt from example code by Luc Guyot
if side == 'top':
    # recover boundary between layer and previous (top)
    top = np.zeros_like(lay)
    edt = distance_transform_edt(((lay >= layer) | (lay == 0)), return_distances=True)
    top[(edt == 1) & (lay == layer)] = 1
    boundary = top
else:  # default is bottom
    # recover boundary between layer and next (bottom)
    bottom = np.zeros_like(lay)
    edt = distance_transform_edt((lay <= layer), return_distances=True)
    bottom[(edt == 1) & (lay == layer)] = 1
    boundary = bottom

# apply mask, if provided
if len(sys.argv) > 7:
    msk = vc.VoxelData.load_nrrd(sys.argv[7])
    if msk.raw.shape != vd.raw.shape:
        print('Mask shape does not match atlas, terminating')
        sys.exit(1)
    boundary *= msk.raw

# save boundary for inspection
vd.with_data(boundary.astype('uint8')).save_nrrd('boundary.nrrd')
w = np.array(np.where(boundary == 1)).T + 0.5  # add 0.5 to get voxel centers
pos = vd.indices_to_positions(w)
np.savetxt(output, pos, "%.12f")
