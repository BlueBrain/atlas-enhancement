import multiprocessing
import voxcell as vc
import numpy as np
import sys

# https://codereview.stackexchange.com/a/28210
# return index of closest node in pLOisoR
def closest(node):
    deltas = pLOisoR - node
    dist_2 = np.einsum('ij,ij->i', deltas, deltas)
    return np.argmin(dist_2)

aHI = vc.VoxelData.load_nrrd(sys.argv[1]) # HI-res annotation from AIBS CCF2017
aLO = vc.VoxelData.load_nrrd(sys.argv[2]) # LO-res annotation from AIBS CCF2017

ids = np.loadtxt(sys.argv[3]) # all Isocortex descendant IDs

wHIiso = np.where(np.isin(aHI.raw, list(ids[:]))) # Isocortex voxels at HIum
ixHIiso = np.array(wHIiso).T
ixHIisoR = ixHIiso[ixHIiso[:,2] >= int(aHI.shape[2]/2)] # Right hemisphere only
pHIisoR = aHI.indices_to_positions(ixHIisoR)

wLOiso = np.where(np.isin(aLO.raw, list(ids[:]))) # Isocortex voxels at LOum
ixLOiso = np.array(wLOiso).T
ixLOisoR = ixLOiso[ixLOiso[:,2] >= int(aLO.shape[2]/2)] # Right hemisphere only
pLOisoR = aLO.indices_to_positions(ixLOisoR)

# Run in parallel: find closest point at LOum for each point at HIum
pool = multiprocessing.Pool()
cix = np.array(pool.map(closest, pHIisoR)) # indices of closest points in pLOisoR to pHIisoR
cpLOisoR = pLOisoR[cix] # the actual closest points
res = np.hstack((pHIisoR,cpLOisoR))

np.save(sys.argv[4],res) # save this lengthy computation!
