import voxcell as vc
import numpy as np
import sys

argc = len(sys.argv)
atlas_nrrd = sys.argv[1]
flatmap_txt = sys.argv[2]
output_nrrd = sys.argv[3]

angle = None
if argc > 4:
    angle = int(sys.argv[4])
    if not angle in [0, 90, 180, 270]:
        raise ValueError('Angle must be 0, 90, 180 or 270')

flipx = None
if argc > 5:
    flipx = int(sys.argv[5])
    if not flipx in [0, 1]:
        raise ValueError('Flip horizontal must be 0 or 1')

flipy = None
if argc > 6:
    flipy = int(sys.argv[6])
    if not flipx in [0, 1]:
        raise ValueError('Flip vertical must be 0 or 1')

vd = vc.VoxelData.load_nrrd(atlas_nrrd)

fmap = np.full(list(vd.raw.shape) + [2], -1, dtype=np.float32)

dat = np.loadtxt(flatmap_txt, dtype=np.float64)

w = dat[:, [0, 1, 2]]
pos = dat[:, [3, 4]]

if angle is not None and angle != 0:
    print('Rotating {} degrees CCW'.format(angle))
    if angle == 90:
        tmp = pos[:,0].copy()
        pos[:,0] = fmax - pos[:,1]  # x -> -y
        pos[:,1] = tmp              # y -> x
    elif angle == 180:
        pos[:,0] = fmax - pos[:,0]  # x -> -x
        pos[:,1] = fmax - pos[:,1]  # y -> -y
    elif angle == 270:
        tmp = pos[:,0].copy()
        pos[:,0] = pos[:,1]         # x -> y
        pos[:,1] = fmax - tmp       # y -> -x

if flipx == 1:
    print('Flipping horizontally after rotation')
    pos[:,0] = fmax - pos[:,0]  # x -> -x

if flipy == 1:
    print('Flipping vertically after rotation')
    pos[:,1] = fmax - pos[:,1]  # y -> -y

fmap[tuple(np.uint64(w).T)] = pos
out = vc.VoxelData(fmap,vd.voxel_dimensions,vd.offset)

out.save_nrrd(output_nrrd)
