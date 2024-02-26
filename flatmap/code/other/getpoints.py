#!/bin/env python3

import voxcell as vc
import numpy as np
import sys

hierarchy_json = sys.argv[1]
brain_regions_nrrd = sys.argv[2]
output = sys.argv[3]

include = []
if len(sys.argv) > 4:
    include = sys.argv[4]
    include = include.split(",")

exclude = []
if len(sys.argv) > 5:
    exclude = sys.argv[5]
    exclude = exclude.split(",")

rmap = vc.RegionMap.load_json(hierarchy_json)
vd = vc.VoxelData.load_nrrd(brain_regions_nrrd)

if len(include) != 0:
    incl = [list(rmap.find('@^{}$'.format(x),attr='acronym',with_descendants=True)) for x in include]
    ids = [x for sub in incl for x in sub]
    print('Including {} members of regions: {}'.format(len(ids),include))
else: # include all
    include = ['ALL']
    ids = [list(rmap.find('@.*',attr='acronym',with_descendants=True))]

if len(exclude) != 0:
    excl = [list(rmap.find('@^{}$'.format(x),attr='acronym',with_descendants=False))[0] for x in exclude]
    ids = [x for x in ids if x not in excl]
    print('Excluding {} regions: {}'.format(len(excl),exclude))

w = np.where(np.isin(vd.raw, ids))

msk = np.zeros_like(vd.raw,dtype='uint8')
msk[w] = 1

vd.with_data(msk).save_nrrd(output)
