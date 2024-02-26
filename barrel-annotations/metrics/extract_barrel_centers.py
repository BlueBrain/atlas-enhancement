"""
Extract barrel centers from annotated barrels positions.
"""
import numpy as np
import pandas as pd

positions = pd.read_feather("../data/annotated_barrels.feather")
barrels = positions[positions.layer == "4"]

barrel_centers = {}
for barrel, df in barrels.groupby("barrel"):
    geometric_mean = df[list("xyz")].mean().values
    barrel_centers[barrel] = geometric_mean

barrel_centers = (
    pd.DataFrame(barrel_centers, index=list("xyz"))
    .T.reset_index()
    .rename(columns={"index": "barrel"})
)
barrel_centers.to_feather("data/barrel_centers.feather")
