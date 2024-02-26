import numpy as np
import pandas as pd
import voxcell as vc


def indices_to_mask(indices, depth):
    mask = np.zeros(depth.shape, dtype=bool)
    mask[tuple(indices.T)] = True
    return mask


def positions_to_mask(positions, annotation):
    mask = np.zeros(annotation.shape)
    indices = annotation.positions_to_indices(positions)

    for indx in indices:
        mask[indx[0], indx[1], indx[2]] = True

    return mask
