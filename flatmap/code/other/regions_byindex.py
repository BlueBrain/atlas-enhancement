import sys
import numpy as np
import voxcell as vc

vd = vc.VoxelData.load_nrrd(sys.argv[1])
dat = np.loadtxt(sys.argv[2],dtype='int32')
ix = dat.reshape(-1,3)
reg = vd._lookup_by_indices(ix)

np.savetxt(sys.argv[3],reg,"%d")
