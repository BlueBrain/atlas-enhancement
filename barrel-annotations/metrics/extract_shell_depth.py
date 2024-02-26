"""Calculate depth based on the boarders of each barrel column"""
import pandas as pd
from tqdm import tqdm

barrels_positions = pd.read_feather("../data/annotated_barrels.feather")
barrels_names = barrels_positions.barrel.unique()
barrels_layers = barrels_positions.layer.unique()


depth_df = []
for barrel in tqdm(barrels_names):
    barrel_depth = pd.read_feather(f"depth_files/border_positions_{barrel}.feather")

    for layer in barrels_layers:
        layer_depth = barrel_depth[barrel_depth.layer == layer]

        df = pd.DataFrame(
            [
                barrel,
                layer,
                layer_depth.depth.mean(),
                layer_depth.depth.std(),
            ],
            index=["barrel", "layer", "depth_mean", "depth_std"],
        ).T
        depth_df.append(df)

depth_df = pd.concat(depth_df)
depth_df.reset_index(drop=True).to_feather("data/depth.feather")
