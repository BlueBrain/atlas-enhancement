"""Get the outer-shell of each layer within SSp-bfd (voxels on the borders of the layer).)"""
import json
import numpy as np
import pandas as pd
import voxcell as vc
from tqdm import tqdm
from scipy import ndimage


def get_shell(indices, depth, return_mask=False):
    """
    Generate a shell around specified indices in a 3D depth array using Exact Euclidean distance transform.

    Args:
    - indices (numpy.ndarray): Array containing indices specifying the shell.
    - depth (numpy.ndarray): 3D array representing the depth.
    - return_mask (bool, optional): Flag indicating whether to return the shell mask. Default is False.

    Returns:
    - numpy.ndarray: Array of indices representing the shell if return_mask is False.
    - numpy.ndarray: Boolean mask representing the shell if return_mask is True.
    """
    mask = np.zeros(depth.shape, dtype=bool)
    mask[tuple(indices.T)] = True

    edt, inds = ndimage.distance_transform_edt(mask.astype(int), return_indices=True)

    x, y, z = np.where(edt == 1)
    edt_indices = np.array([x, y, z]).T

    if return_mask:
        edt_mask = np.zeros(depth.shape, dtype=bool)
        edt_mask[tuple(edt_indices.T)] = True
        return edt_mask

    return edt_indices


def layer_indices_single_hemisphere(layer, region_map, annotation, depth):
    """
    Retrieve indices corresponding to a specific layer within a single hemisphere.

    Args:
    - layer (int): Layer identifier.
    - region_map (RegionMap): Mapping of regions.
    - annotation (AnnotationData): Annotation data containing region information.
    - depth (DepthData): Depth data representing the spatial arrangement.

    Returns:
    - numpy.ndarray: Array of indices corresponding to the specified layer.
    """
    layer_id = np.array(
        list(
            region_map.find(
                attr="acronym", value=f"SSp-bfd{layer}", with_descendants=True
            )
        )
    )
    mask = np.isin(annotation.raw, layer_id)

    indices = np.array(np.where(mask)).T
    positions = depth.indices_to_positions(indices)
    positions = pd.DataFrame(positions, columns=["x", "y", "z"])
    positions = positions[positions.z < 6000]
    layer_indices = depth.positions_to_indices(positions).values

    return layer_indices


annotation_barrels = vc.VoxelData.load_nrrd("../data/annotation_barrels_10.nrrd")
annotation = vc.VoxelData.load_nrrd("../data/annotation_10.nrrd")
depth = vc.VoxelData.load_nrrd("../data/depth.nrrd")
region_map = vc.RegionMap.load_json("../data/hierarchy.json")


for name, layer in tqdm(
    zip(["1", "23", "4", "5", "6a", "6b"], ["1", "2/3", "4", "5", "6a", "6b"])
):
    indices = layer_indices_single_hemisphere(layer, region_map, annotation, depth)
    shell_layer = get_shell(indices, depth, return_mask=False)
    shell_layer = pd.DataFrame(shell_layer, columns=["x", "y", "z"])
    shell_layer.reset_index(drop=True).to_feather(
        f"depth_files/layer_{name}_shell.feather"
    )
