import flatmap_util as fmutil
import sys

flatmap_nrrd = sys.argv[1]
output_prefix = sys.argv[2]

fmap, _ = fmutil.load_flatmap(flatmap_nrrd)
fmap.with_data(fmap.raw[:,:,:,0]).save_nrrd('{}/flat_x.nrrd'.format(output_prefix))
fmap.with_data(fmap.raw[:,:,:,1]).save_nrrd('{}/flat_y.nrrd'.format(output_prefix))
