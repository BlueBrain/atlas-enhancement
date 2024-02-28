"""Utilities for splitting atlases. Copied from atlas-spliter to avoid a big install with dependencies."""
import itertools as it
from typing import Any, Dict, Iterator

from voxcell import RegionMap


HierarchyDict = Dict[str, Any]


class AtlasSplitterError(Exception):
    """
    Exception raised by functions of atlas_splitter.
    """

def get_isocortex_hierarchy(allen_hierachy: HierarchyDict):
    """
    Extract the hierarchy dict of the iscortex from AIBS hierarchy dict.
    Args:
        allen_hierarchy: AIBS hierarchy dict instantiated from
            http://api.brain-map.org/api/v2/structure_graph_download/1.json.
    """
    error_msg = "Wrong input. The AIBS 1.json file is expected."
    if "msg" not in allen_hierachy:
        raise AtlasSplitterError(error_msg)

    hierarchy = allen_hierachy["msg"][0]
    try:
        while hierarchy["acronym"] != "Isocortex":
            hierarchy = hierarchy["children"][0]
    except KeyError as error:
        raise AtlasSplitterError(error_msg) from error

    return hierarchy


def create_id_generator(region_map: RegionMap) -> Iterator[int]:
    """Create an identifiers generator.

    The generator produces an identifier which is different from all
    the previous ones and from the identifiers in use in `self.region_map`.

    Args:
        region_map: map to navigate the brain region hierarchy.

    Return:
        iterator providing the next id.
    """
    last = max(region_map.find("root", attr="acronym", with_descendants=True))
    return it.count(start=last + 1)


def _assert_is_leaf_node(node) -> None:
    """
    Raises an AtalasSplitterError if `node` is not a leaf node.

    Args:
        node: node of the hierarchy tree. Dict of the form
        {
            "id": 195,
                   "atlas_id": 1014,
                   "ontology_id": 1,
                   "acronym": "PL2",
                   "name": "Prelimbic area, layer 2",
                   "color_hex_triplet": "2FA850",
                   "graph_order": 240,
                   "st_level": 11,
                   "hemisphere_id": 3,
                   "parent_structure_id": 972,
                   "children": []
                  },

        }
    Raises:
        AtlasSplitterError if `node` is a not a leaf `node`.
    """
    if "children" not in node:
        raise AtlasSplitterError(f'Missing "children" key for region {node["name"]}.')
    if node["children"] != []:
        raise AtlasSplitterError(
            f'Region {node["name"]} has an unexpected "children" value: '
            f'{node["children"]}. Expected: [].'
        )
