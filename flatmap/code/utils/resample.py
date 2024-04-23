from voxcell import VoxelData
import flatmap_util as fmutil
import numpy as np
import sys

flatmap_nrrd = sys.argv[1]
rmask_nrrd = sys.argv[2]
output_nrrd = sys.argv[3]

interp = True
if len(sys.argv) > 4:
    interp = True if sys.argv[4] in ["true", "True"] else\
                (False if sys.argv[4] in ["false", "False"] else int(sys.argv[4]))

fmap, _ = fmutil.load_flatmap(flatmap_nrrd)
rmask = VoxelData.load_nrrd(rmask_nrrd)

w = np.where(rmask.raw)
idx = np.array(w).T
pos = rmask.indices_to_positions(idx)
flatpos = fmutil.lookup(pos, fmap, True, interp)
newfmap = np.full(rmask.shape + (2,), [-1,-1], dtype='float32')
newfmap[w] = flatpos
rmask.with_data(newfmap).save_nrrd(output_nrrd)
