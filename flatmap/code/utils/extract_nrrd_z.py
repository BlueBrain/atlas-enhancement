from voxcell import VoxelData
import sys

input_nrrd = sys.argv[1]
z_index = int(sys.argv[2])
output_nrrd = sys.argv[3]

vd = VoxelData.load_nrrd(input_nrrd)
res = vd.raw[z_index,:,:]
res_vd = VoxelData(res, (1,1))
res_vd.save_nrrd(output_nrrd)
