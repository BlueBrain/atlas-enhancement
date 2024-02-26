"""
Generate the nrrd file containing only the new barrel annotation ids within SSp-bfd (primary somatosensory barrel cortex).
This file will extract all of the barrel columns annotations in SSp-bfd, remove faulty V1 subregions and create a new annotations array with zeros outside of the barrel columns. 

This file can be transplanted directly into the original atlas annotation_10.nrrd 
(this file can be also downloaded from Zenodo).
"""
import json
import numpy as np
import pandas as pd
from tqdm import tqdm
from voxcell import RegionMap, VoxelData
from somatosensory_barrels import get_hierarchy_by_acronym

hierarchy_path = "data/hierarchy.json"
annotation = VoxelData.load_nrrd("data/annotation_barrels_10.nrrd")

region_map = RegionMap.load_json(hierarchy_path)
with open(hierarchy_path, "r", encoding="utf-8") as h_file:
    hierarchy = json.load(h_file)

hierarchy_ = get_hierarchy_by_acronym(hierarchy, region_map, start_acronym="SSp-bfd")

barrel_cortex_ids = region_map.find("SSp-bfd", attr="acronym", with_descendants=True)
barrel_cortex_ids = np.array(list(barrel_cortex_ids))
barrel_cortex_ids_ = barrel_cortex_ids[
    barrel_cortex_ids > 600000000
]  # remove the faulty V1 subregions

barrel_annotation = np.zeros_like(annotation.raw)
for index in tqdm(barrel_cortex_ids_):
    indices_mask = np.isin(annotation.raw, index)
    barrel_annotation[indices_mask] = index

barrel_nrrd = VoxelData(
    barrel_annotation, annotation.voxel_dimensions, offset=annotation.offset
)
barrel_nrrd.save_nrrd("data/annotation_barrels.nrrd")
