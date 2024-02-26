import numpy as np
from os import path
from sys import argv
from voxcell import VoxelData
argc = len(argv)


def loadext(filename):
    _, ext = path.splitext(filename)
    if ext == ".img":
        return np.fromfile(filename, dtype=np.float32)
    elif ext == ".nrrd":
        return VoxelData.load_nrrd(filename).raw
    else:
        raise ValueError("unrecognized file extension {}".fmt(ext))


img_file = argv[1]
savefile = argv[2]

msk_file = None
if argc > 3:
    msk_file = argv[3]

img = loadext(img_file)

if msk_file is None:
    valid_msk = (img != -1)
else:
    msk = loadext(msk_file)
    assert(img.shape == msk.shape)
    valid_msk = (msk == 1)

vals = img[valid_msk]
vals = vals[~np.isnan(vals)]  # remove NaNs

np.savetxt(savefile,vals,"%.16f")
