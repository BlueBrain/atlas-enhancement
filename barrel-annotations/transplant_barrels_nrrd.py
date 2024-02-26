"""
Transplant of pregenerated barrel annotations into the annotation_10.nrrd file. This script overwrittes parts of the SSp-bfd (primary somatosensory barrel cortex) with new barel column in the mouse annotations in place, following the indices in hierarchy.json.
    
Input files:
- annotation_barrels.nrrd
- annotation_10.nrrd
- hierarchy.json

Output files:
- annotation_barrels_10.nrrd
"""
import numpy as np
import pandas as pd
from voxcell import VoxelData

barrel = VoxelData.load_nrrd("data/annotation_barrels.nrrd")
annotation = VoxelData.load_nrrd("data/annotation_10.nrrd")

annotation_ = annotation.raw
indices = np.argwhere(barrel.raw > 0)
mask = np.zeros(annotation.shape)

for indx in indices:
    mask[indx[0], indx[1], indx[2]] = True
mask = mask.astype(bool)

annotation_[mask] = barrel.raw[mask]

barrel_nrrd = VoxelData(
    annotation_, annotation.voxel_dimensions, offset=annotation.offset
)

barrel_nrrd.save_nrrd("data/annotation_barrels_10.nrrd")
