#!/bin/env python3

import voxcell as vc
import numpy as np
import sys

fmap = vc.VoxelData.load_nrrd(sys.argv[1])

x = fmap.raw[:,:,:,0] # x coordinate in flatland
y = fmap.raw[:,:,:,1] # y coordinate in flatland

wx = np.array(np.where(x >= 0)).T
wy = np.array(np.where(y >= 0)).T

chk = np.equal(wx,wy).all()
if not chk:
    raise ValueError('Have bad pixels in flatmap with only one coordinate >= 0')

ind = wx
flatp = fmap._lookup_by_indices(ind) # flatmap points

# Get region information
rmap = vc.RegionMap.load_json(sys.argv[2])
vd = vc.VoxelData.load_nrrd(sys.argv[3])

# Load regions from file
reglst = np.loadtxt(sys.argv[4],dtype=np.str)
assoc = { x[0] : int(x[1]) for x in reglst }

regs = vd._lookup_by_indices(ind) # regions
cols = []
for x in regs:
    try:
        acrs = rmap.get(x,'acronym',with_ascendants=True)
    except KeyError:
        # Workaround for Allen annotations as BBP splits layer 2/3 into layer 2 and 3,
        # adding 20000 and 30000 to the region ID, respectively
        # Here we always add 20000 because we don't care about the layer but the parent region
        x += 20000
        acrs = rmap.get(x,'acronym',with_ascendants=True)
    
    isec = list(set(acrs) & assoc.keys())
    if len(isec) == 0: # region not found
        col = -1
    elif len(isec) == 1: # unique value
        col = assoc[isec[0]] # id of region for this voxel
    else: # non-unique intersection -> try parent
        try:
            col = assoc[acrs[1]]
        except KeyError:
            col = -1
    cols.append(col)

#pars = np.array([rmap.get(x,'acronym',with_ascendants=True)[1] for x in regs]) # parent regions
#upars = list(np.unique(pars)) # unique parents
#assoc = { x : i for i,x in enumerate(upars) } # assign numbers to unique parent regions
#cols = np.array([assoc[x] for x in pars]) # parent region numbers

parentRegion = sys.argv[6] # parent region
# all children of parent region
childRegions = rmap.find('@^{}$'.format(parentRegion),"acronym",with_descendants=True)

# Get layer information
lay = np.zeros_like(vd.raw)
for i in range(1,7):
    ids = [x for x in childRegions if 'layer {}'.format(i) in rmap.get(x,'name').lower()] # search for 'layer %' in name
    msk = np.isin(vd.raw, ids)
    lay[msk] = i

layer = lay[tuple(ind.T)]

xycl = np.vstack((flatp.T,cols,layer)).T # append region number to flatmap points
np.savetxt(sys.argv[5],xycl,"%d")
