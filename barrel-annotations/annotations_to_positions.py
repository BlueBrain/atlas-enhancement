"""
Generate the barrel column positiosn from the annotation and flatmap.

This file performs the following steps:
1. Generate the barrel columns based on barrels, with use of ConvexHull around the barrel and flattened positions to measure the distances.
2. Translate the positions into the symmetric hemisphere
3. Annotate the positions with layer information

Input files:
- 'data/barrel_positions.feather'
- 'data/annotation_10.nrrd'
- 'data/1.json'
- 'data/flatmap_float.nrrd'

Output files:
- 'data/barrel_columns_positions.feather'
- 'data/barrel_columns_positions_both_hemispheres.feather'
- 'data/annotated_barrels.feather
"""

import numpy as np
import pandas as pd
import voxcell as vc
from tqdm import tqdm
from utils import (
    flatmapped_distance,
    region_flatmapped,
    symmetric_positions,
    region_positions,
)

flat_barrel_positions = pd.read_feather("data/barrel_positions.feather")
annotation = vc.VoxelData.load_nrrd("data/annotation_10.nrrd")
regionmap = vc.RegionMap.load_json("data/1.json")
flatmap = vc.VoxelData.load_nrrd("data/flatmap_float.nrrd")

# Extract barrel columns
bfdflatmap, _ = region_flatmapped("SSp-bfd", regionmap, flatmap, annotation)

barrel_columns = []
for barrel, positions in tqdm(flat_barrel_positions.groupby("barrel")):
    bfdflatmap["distance"] = flatmapped_distance(positions, bfdflatmap)
    barrel_column = bfdflatmap[bfdflatmap.distance <= 1e-3]
    barrel_column_positions = annotation.indices_to_positions(
        barrel_column[list("xyz")]
    )
    barrel_column_positions["barrel"] = barrel
    barrel_column_positions["x_flat"] = barrel_column["x_flat"]
    barrel_column_positions["y_flat"] = barrel_column["y_flat"]
    barrel_columns.append(barrel_column_positions)

barrel_columns = pd.concat(barrel_columns)
barrel_columns.reset_index(drop=True).to_feather(
    "data/barrel_columns_positions.feather"
)


# Translate the positions into the symmetric hemisphere
barrel_columns_both_hemispheres = symmetric_positions(barrel_columns, annotation)
barrel_columns_both_hemispheres.reset_index(drop=True)[
    ["x", "y", "z", "barrel", "hemisphere"]
].to_feather("data/barrel_columns_positions_both_hemispheres.feather")

# Annotate the positions
annotated_barrels = []
for layer in tqdm(["1", "2/3", "4", "5", "6a", "6b"]):
    layer_positions = region_positions(f"SSp-bfd{layer}", regionmap, annotation)

    for barrel, positions in tqdm(barrel_columns_both_hemispheres.groupby("barrel")):
        subset = pd.merge(
            positions,
            layer_positions,
            on=["x", "y", "z"],
            how="inner",
        )
        subset["layer"] = layer
        subset["barrel"] = barrel
        annotated_barrels.append(subset)


annotated_barrels = pd.concat(annotated_barrels)
annotated_barrels.reset_index(drop=True).to_feather("data/annotated_barrels.feather")
