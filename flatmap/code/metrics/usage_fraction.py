import voxcell as vc
import numpy as np
import sys

flatmap_nrrd = sys.argv[1]
pixel_res = int(sys.argv[2])

fmap_vd = vc.VoxelData.load_nrrd(flatmap_nrrd)
fmap = fmap_vd.raw
msk = fmap[:,:,:,0] > -1
pix_uniq = np.unique(fmap[msk],axis=0)

print(pixel_res,len(pix_uniq) / (pixel_res * pixel_res))
