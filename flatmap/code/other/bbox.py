#!/usr/bin/env python3

import numpy as np
import voxcell as vc
import sys


def bbox(dat, bbmin, bbmax):
    bbox = dat
    bbox = bbox[:, :, range(bbmin[2], bbmax[2] + 1)]
    bbox = bbox[:, range(bbmin[1], bbmax[1] + 1), :]
    bbox = bbox[range(bbmin[0], bbmax[0] + 1), :, :]
    return bbox


atlas_hierarchy = sys.argv[1]
atlas_regions = sys.argv[2]
atlas_orientation = sys.argv[3]
parentRegion = sys.argv[4]

vd = vc.VoxelData.load_nrrd(atlas_regions)
w = np.where(vd.raw > 0)
w = np.sort(w)

# get bounds of atlas regions
bbmin = []
bbmax = []
bbdim = []
for i in [0, 1, 2]:
    bbmin.append(w[i][0])
    bbmax.append(w[i][-1])
    bbdim.append(bbmax[i] - bbmin[i] + 1)

idx = np.indices(vd.raw.shape)
indT = np.vstack(([idx[0].T], [idx[1].T], [idx[2].T])).T
pos = vd.indices_to_positions(indT + 0.5)  # voxel centers
bbpos = bbox(pos, bbmin, bbmax)

# Extract orientation vectors within bounding box
of = vc.OrientationField.load_nrrd(atlas_orientation)

bbvd = bbox(vd.raw, bbmin, bbmax)  # brain regions bounding box
bbof = bbox(of.raw, bbmin, bbmax)  # orientation bounding box
idx = np.indices(bbvd.shape)  # indices of orientation bounding box
ind = np.vstack(([idx[0]], [idx[1]], [idx[2]]))

qt = bbof[tuple(ind)]  # all quaternions in bounding box

rqt = qt.reshape((np.prod(qt.shape[:-1]), 4))
# From OrientationField.lookup
if rqt.dtype == np.int8:
    rqt = rqt / 127.0
rqt = np.roll(rqt, -1, axis=-1)
rmt = vc.quaternion.quaternions_to_matrices(rqt)

mt = rmt.reshape(tuple(list(qt.shape[:-1]) + [3, 3]))  # all matrices in bounding box
normal = mt[:, :, :, :, 1]  # all Y vectors (normals) in bounding box

x = normal[:, :, :, 0]
y = normal[:, :, :, 1]
z = normal[:, :, :, 2]

# Zero out unused vectors
x[bbvd == 0] = 0.0
y[bbvd == 0] = 0.0
z[bbvd == 0] = 0.0

# export full-sized matrices, so shape matches atlas
padding = tuple([(bbmin[i], vd.raw.shape[i] - bbmax[i] - 1) for i in [0, 1, 2]])

x = np.pad(x, padding, mode='constant', constant_values=0.0)
y = np.pad(y, padding, mode='constant', constant_values=0.0)
z = np.pad(z, padding, mode='constant', constant_values=0.0)

# Check we did it right
assert x.shape == vd.raw.shape

# Get layers
rmap = vc.RegionMap.load_json(atlas_hierarchy)
# all children of parent region
childRegions = rmap.find('@^{}$'.format(parentRegion), "acronym", with_descendants=True)

lay = np.zeros_like(vd.raw)
for i in range(1, 7):
    # search for 'layer %' in name
    ids = [x for x in childRegions if 'layer {}'.format(i) in rmap.get(x, 'name').lower()]
    msk = np.isin(vd.raw, ids)
    lay[msk] = i

# apply mask, if provided
if len(sys.argv) > 5:
    msk = vc.VoxelData.load_nrrd(sys.argv[5])
    if msk.raw.shape != vd.raw.shape:
        print('Mask shape does not match atlas, terminating')
        sys.exit(1)
    x *= msk.raw
    y *= msk.raw
    z *= msk.raw
    lay *= msk.raw

# Save to binary arrays
# Transpose so C order matches!
x.T.astype('float64').tofile('normal_x.bin')
y.T.astype('float64').tofile('normal_y.bin')
z.T.astype('float64').tofile('normal_z.bin')
lay.T.astype('uint16').tofile('layers.bin')

# Save dims, voxel size and offsets
num = [np.sum(lay > 0), '', '']
dim = np.array(vd.raw.shape)
vox = np.array([float(x).hex() for x in vd.voxel_dimensions])
off = np.array([float(x).hex() for x in vd.offset])
ndvo = np.vstack((num, dim, vox, off))
np.savetxt('dimensions.txt', ndvo, '%s')
