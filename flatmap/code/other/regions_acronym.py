import sys
import numpy as np
import voxcell as vc

rmap = vc.RegionMap.load_json(sys.argv[1])
ids = np.loadtxt(sys.argv[2])
acrs = []
for x in ids:
    try:
        acr = rmap.get(x,attr='acronym')
        acrs.append(acr)
    except:
        acrs.append(None)

np.savetxt(sys.argv[3],acrs,"%s")
