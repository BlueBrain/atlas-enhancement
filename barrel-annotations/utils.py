import numpy as np
import pandas as pd
import voxcell as vc
from scipy.spatial import ConvexHull
from shapely.geometry import Point, Polygon


def positions_to_mask(positions, annotation):
    mask = np.zeros(annotation.shape)
    indices = annotation.positions_to_indices(positions)

    for indx in indices:
        mask[indx[0], indx[1], indx[2]] = True

    return mask


def region_positions(region, regionmap, annotation):
    """Get positions of the voxels in the region and its descendants"""
    indices = list(regionmap.find(region, attr="acronym", with_descendants=True))
    indices_mask = np.isin(annotation.raw, indices)

    indices = np.array(np.where(indices_mask)).T
    return pd.DataFrame(annotation.indices_to_positions(indices), columns=list("xyz"))


def symmetric_positions(one_hemisphere, annotation):
    """Extract the positions of the voxels in the other hemisphere based on the positions in one hemisphere"""
    positions_all = []
    for name in one_hemisphere.barrel.unique():
        positions = one_hemisphere[one_hemisphere.barrel == name]
        mask = positions_to_mask(positions[["x", "y", "z"]].values, annotation).astype(
            np.uint8
        )

        positions_flipped = annotation.indices_to_positions(
            np.transpose(np.nonzero(mask[:, :, ::-1].astype(int)))
        )
        positions_flipped = pd.DataFrame(positions_flipped, columns=list("xyz"))

        positions.loc[:, "hemisphere"] = "left"
        positions_flipped.loc[:, "hemisphere"] = "right"
        positions_flipped.loc[:, "barrel"] = name
        positions_flipped.loc[:, "barrel"] = name

        positions_all.append(positions)
        positions_all.append(positions_flipped)

    return pd.concat(positions_all)


def region_flatmapped(region, regionmap, flatmap, annotation):
    """Get positions of the voxels in the region and its descendants, get teh flatmap coordinates of the voxels"""
    indices = list(regionmap.find(region, attr="acronym", with_descendants=True))

    indices_mask = np.isin(annotation.raw, indices)
    indices = np.array(np.where(indices_mask)).T

    flatix = flatmap.positions_to_indices(annotation.indices_to_positions(indices))
    flatreg = flatmap.raw[tuple(flatix.T)]
    good = flatreg[:, 0] != -1
    flatreg_mapped = flatreg[good]
    regix_mapped = indices[good]

    region_indices = pd.DataFrame(
        np.hstack((regix_mapped, flatreg_mapped)),
        columns=["x", "y", "z", "x_flat", "y_flat"],
    )
    region_positions = annotation.indices_to_positions(region_indices[list("xyz")])
    region_positions["x_flat"] = region_indices["x_flat"]
    region_positions["y_flat"] = region_indices["y_flat"]

    return region_indices, region_positions


def flatmapped_distance(barrel_positions, flatmap):
    """Get distance from barrel coordinates to barrel column based on the flattened coordinates"""
    hull = ConvexHull(barrel_positions[["x_flat", "y_flat"]].values)
    hullpoints = barrel_positions.iloc[hull.vertices]

    polygon = Polygon(list(hullpoints[["x_flat", "y_flat"]].to_records(index=False)))
    points = flatmap[["x_flat", "y_flat"]].to_records(index=False)

    distance = []
    for point in points:
        p1 = Point(list(point))
        distance.append(polygon.distance(p1))

    return distance


def merge_barrel_flatmap_positions(positions, annotation, flatmap):
    """Merge the barrel coordinates with the flatmap coordinates"""
    barrel_indices = pd.DataFrame(
        annotation.positions_to_indices(positions[list("xyz")].values),
        columns=list("xyz"),
    ).astype(float)
    barrel = pd.merge(barrel_indices, flatmap, how="inner", on=list("xyz"))
    barrel_positions = annotation.indices_to_positions(barrel[list("xyz")])
    barrel_positions["x_flat"] = barrel["x_flat"]
    barrel_positions["y_flat"] = barrel["y_flat"]

    return barrel_positions
