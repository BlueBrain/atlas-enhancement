"""Use the outer-layer-shells to find the border positions between layers for each barrel column"""
import numpy as np
import pandas as pd
import voxcell as vc
from tqdm import tqdm
from scipy import ndimage
from utils import indices_to_mask, positions_to_mask


def is_adjacent_voxel(voxel1, voxel2, distance=1):
    """
    Check if two voxels are adjacent within a specified distance.

    Args:
    - voxel1 (numpy.ndarray): Coordinates of the first voxel.
    - voxel2 (numpy.ndarray): Coordinates of the second voxel.
    - distance (int, optional): Maximum distance for voxels to be considered adjacent. Defaults to 1.

    Returns:
    - bool: True if voxels are adjacent within the specified distance, False otherwise.
    """
    return np.linalg.norm(voxel1 - voxel2) <= distance


def get_adjacent_voxels(inspected_volume, next_volume, distance=1):
    """
    Retrieve adjacent voxels between two volumes.

    Args:
    - inspected_volume (pandas.DataFrame): DataFrame containing coordinates of voxels in the inspected volume.
    - next_volume (pandas.DataFrame): DataFrame containing coordinates of voxels in the next volume.
    - distance (int, optional): Maximum distance for voxels to be considered adjacent. Defaults to 1.

    Returns:
    - numpy.ndarray: Array of coordinates representing adjacent voxels.
    """
    adjacent_voxels = set()
    for n, (x, y, z) in next_volume.iterrows():
        v1 = np.array([x, y, z])

        for m, (x_, y_, z_) in inspected_volume.iterrows():
            v2 = np.array([x_, y_, z_])

            if is_adjacent_voxel(v1, v2):
                adjacent_voxels.add(tuple(v2))

    return np.array(list(adjacent_voxels))


def merge_shell(shell, column_mask, depth):
    """
    Merge two binary shells and retrieve the merged indices and positions.

    Args:
    - shell (numpy.ndarray): Binary array representing the first shell.
    - column_mask (numpy.ndarray): Binary array representing the column mask.
    - depth (DepthData): Depth data representing the spatial arrangement.

    Returns:
    - numpy.ndarray: Array of merged indices.
    - pandas.DataFrame: DataFrame containing merged positions.
    """
    mask_ = np.logical_and(shell, column_mask)
    x, y, z = np.where(mask_ == True)

    merged_indices = np.array([x, y, z]).T

    merged_positions = depth.indices_to_positions(
        pd.DataFrame(merged_indices, columns=["x", "y", "z"])
    )
    return merged_indices, merged_positions


annotation = vc.VoxelData.load_nrrd("../data/annotation_10.nrrd")
depth = vc.VoxelData.load_nrrd("../data/depth.nrrd")
region_map = vc.RegionMap.load_json("../data/hierarchy.json")

barrels_positions = pd.read_feather("../data/annotated_barrels.feather")
barrels_positions = barrels_positions[barrels_positions.hemisphere == "right"]


masks = {
    "1": indices_to_mask(
        pd.read_feather("depth_files/layer_1_shell.feather").values, depth
    ),
    "2/3": indices_to_mask(
        pd.read_feather("depth_files/layer_23_shell.feather").values, depth
    ),
    "4": indices_to_mask(
        pd.read_feather("depth_files/layer_4_shell.feather").values, depth
    ),
    "5": indices_to_mask(
        pd.read_feather("depth_files/layer_5_shell.feather").values, depth
    ),
    "6a": indices_to_mask(
        pd.read_feather("depth_files/layer_6a_shell.feather").values, depth
    ),
    "6b": indices_to_mask(
        pd.read_feather("depth_files/layer_6b_shell.feather").values, depth
    ),
}

borders = [
    ["1", "2/3"],
    ["2/3", "4"],
    ["4", "5"],
    ["5", "6a"],
    ["6a", "6b"],
]


for barrel, column_positions in tqdm(barrels_positions.groupby("barrel")):
    column_mask = positions_to_mask(column_positions[["x", "y", "z"]].values, depth)

    border_positions = []
    for border_x, border_y in tqdm(borders):
        indices_x, _ = merge_shell(masks[border_x], column_mask, depth)
        indices_y, _ = merge_shell(masks[border_y], column_mask, depth)

        adjacent = get_adjacent_voxels(pd.DataFrame(indices_x), pd.DataFrame(indices_y))
        adjacent_positions = pd.DataFrame(
            depth.indices_to_positions(adjacent), columns=["x", "y", "z"]
        )

        adjacent_positions["layer"] = border_x
        adjacent_positions["depth"] = depth.raw[
            indices_to_mask(adjacent, depth).astype(bool)
        ]
        border_positions.append(adjacent_positions)

    indices_y, _ = merge_shell(masks["6b"], column_mask, depth)
    adjacent_positions = pd.DataFrame(
        depth.indices_to_positions(indices_y), columns=["x", "y", "z"]
    )
    adjacent_positions["layer"] = "6b"
    adjacent_positions["depth"] = depth.raw[
        indices_to_mask(indices_y, depth).astype(bool)
    ]
    border_positions.append(adjacent_positions)

    border_positions = pd.concat(border_positions)
    border_positions.reset_index(drop=True).to_feather(
        f"depth_files/border_positions_{barrel}.feather"
    )
