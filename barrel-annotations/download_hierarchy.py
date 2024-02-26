"""
This script downloads a file containing the Allen Brain Atlas 10um hierarchy. "1.json"
"""

import json
import urllib.request

path = "http://api.brain-map.org/api/v2/structure_graph_download/1.json"


with urllib.request.urlopen(path) as url:
    data = json.load(url)

with open("data/1.json", "w", encoding="utf-8") as out:
    json.dump(data, out, indent=1, separators=(",", ": "))
