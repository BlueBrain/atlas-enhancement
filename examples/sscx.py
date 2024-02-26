from voxcell import VoxelData, RegionMap, OrientationField
import numpy as np

# load region annotations
annot = VoxelData.load_nrrd('brain_regions.nrrd')
rmap = RegionMap.load_json('hierarchy.json')

# find voxels in region S1HL
ids = list(rmap.find('S1HL', 'acronym', with_descendants=True))
mask = np.isin(annot.raw, ids)
w = np.where(mask)
pos = annot.indices_to_positions(np.array(w).T)

# load thickness and orientation datasets
thick = VoxelData.load_nrrd('thickness.nrrd')
orient = OrientationField.load_nrrd('orientation.nrrd')

# compute mean values
mean_thickness = np.mean(thick.lookup(pos))
yvec = orient.lookup(pos)[:,:,1]  # rotated Y vectors
mean_orientation = np.mean(yvec, axis=0)

# report
print('Mean thickness of region S1HL: {} um'.format(mean_thickness))
print('Mean orientation of region S1HL: {}'.format(mean_orientation))
