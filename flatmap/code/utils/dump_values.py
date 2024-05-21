from voxcell import VoxelData
import numpy as np
import sys

img_nrrd = sys.argv[1]
savefile = sys.argv[2]

msk_nrrd = None
if len(sys.argv) > 3:
    msk_nrrd = sys.argv[3]

img = VoxelData.load_nrrd(img_nrrd)

if msk_nrrd is None:
    valid_msk = ~np.isnan(img.raw)
else:
    msk = VoxelData.load_nrrd(msk_nrrd)
    assert(img.shape == msk.shape)
    valid_msk = (msk.raw == 1)

vals = img.raw[valid_msk]

np.savetxt(savefile,vals,"%.16f")
