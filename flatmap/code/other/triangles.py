#!/bin/env python3

import voxcell as vc
import numpy as np
from scipy.spatial import ConvexHull
from scipy.ndimage.measurements import label, labeled_comprehension
import matplotlib.pyplot as plt
from pathlib import Path
import json
import sys

flatmap = sys.argv[1]
atlas_hierarchy = sys.argv[2]
atlas_regions = sys.argv[3]
regions_json = sys.argv[4]

fmap = vc.VoxelData.load_nrrd(flatmap)

x = fmap.raw[:, :, :, 0]  # x coordinate in flatland
y = fmap.raw[:, :, :, 1]  # y coordinate in flatland

xdim = np.max(x)  # x dimension of flatland
ydim = np.max(y)  # y dimension of flatland

wx = np.array(np.where(x >= 0)).T
wy = np.array(np.where(y >= 0)).T

chk = np.equal(wx, wy).all()
if not chk:
    raise ValueError('Bad pixel in flatmap, only one coordinate >= 0')

ind = wx
flatp = fmap._lookup_by_indices(ind)  # flatmap points

# Get region information
rmap = vc.RegionMap.load_json(atlas_hierarchy)
vd = vc.VoxelData.load_nrrd(atlas_regions)

jreg = json.loads(Path(regions_json).read_text())
regions = [jreg[key] for key in jreg]

reglst = [x for sub in regions for x in sub]
assoc = {x : i for i, x in enumerate(reglst)}

regs = vd._lookup_by_indices(ind)  # regions
cols = []
for x in regs:
    acrs = rmap.get(x, 'acronym', with_ascendants=True)
    isec = list(set(acrs) & assoc.keys())
    if len(isec) == 0:  # region not found
        col = -1  # set to -1
    elif len(isec) == 1:  # unique value
        col = assoc[isec[0]]  # id of region for this voxel
    else:  # non-unique intersection -> try parent
        try:
            col = assoc[acrs[1]]
        except KeyError:
            col = -1
    cols.append(col)

# pars = np.array([rmap.get(x, 'acronym', with_ascendants=True)[1] for x in regs])  # parent regions
# upars = list(np.unique(pars))  # unique parents
# assoc = {}
# [assoc.update({x : i}) for i, x in enumerate(upars)] # assign numbers to unique parent regions
# cols = np.array([assoc[x] for x in pars])  # parent region numbers

xyc = np.vstack((flatp.T, cols)).T


# Compute area of polygon formed by three vertices in convex hull of x
def area(a, b, c, x, ch):
    pts = np.vstack((x[ch.vertices[a]], x[ch.vertices[b]], x[ch.vertices[c]]))
    x = pts[:, 0]
    y = pts[:, 1]
    return 0.5 * np.abs(np.dot(x, np.roll(y, 1)) - np.dot(y, np.roll(x, 1)))


# Maximum-area triangle (https://stackoverflow.com/a/1621913)
def maxtriangle(x, ch):
    # Assume points have been sorted already, as 0...(n-1)
    A = 0
    B = 1
    C = 2
    # The "best" triple of points
    bA = A
    bB = B
    bC = C
    while True:  # loop A

        while True:  # loop B
            while area(A, B, C, x, ch) <= area(A, B, (C + 1) % n, x, ch):  # loop C
                C = (C + 1) % n
            if area(A, B, C, x, ch) <= area(A, (B + 1) % n, C, x, ch):
                B = (B + 1) % n
                continue
            else:
                break

        if area(A, B, C, x, ch) > area(bA, bB, bC, x, ch):
            bA = A
            bB = B
            bC = C

        A = (A + 1) % n
        if A == B:
            B = (B + 1) % n
        if B == C:
            C = (C + 1) % n
        if A == 0:
            break

    return bA, bB, bC


colors = ['red', 'green', 'blue']

tri = {}
has_correct = False
if len(sys.argv) > 5:  # has correct triangles
    tri = json.loads(Path(sys.argv[5]).read_text())
    has_correct = True

for reg in regions:
    name = '_'.join(reg)
    vox = np.empty((0, 2))
    for _ in reg:
        r = assoc[_]
        v = xyc[np.where(xyc[:, 2] == r)][:, [0, 1]]
        vox = np.vstack((vox, v))
    uvox = np.unique(vox, axis=0)

    if not has_correct:
        # Take largest connected component in uvox,
        # as a strategy to deal with outliers
        z = np.zeros((xdim + 1, ydim + 1))
        z[tuple(np.int16(uvox).T)] = 1
        lab, nlab = label(z)  # connected components (CC)
        slab = labeled_comprehension(z, lab, np.arange(1, nlab + 1), np.sum, int, 0)  # size of CCs
        imax = np.argmax(slab)  # index of largest CC
        vvox = np.array(np.where(lab == imax + 1)).T

        ch = ConvexHull(vvox)
        n = len(ch.vertices)
        bA, bB, bC = maxtriangle(vvox, ch)
        # initial colors
        best = {c : tuple(vvox[ch.vertices[i]]) for i, c in zip((bA, bB, bC), colors)}
        tri.update({name : best})

    fig = plt.figure(figsize=(5, 5))
    plt.suptitle(reg)
    plt.xlim((0, xdim))
    plt.ylim((0, ydim))

    z = np.zeros((xdim + 1, ydim + 1))  # 0 is unmapped
    z[tuple(xyc[np.where(xyc[:, 2] == -1)][:, [0, 1]].T)] = 1  # 1 is mapped but unused
    z[tuple(xyc[np.where(xyc[:, 2] >= 0)][:, [0, 1]].T)] = 2  # 2 is mapped and used
    z[tuple(np.int16(uvox).T)] = 3  # 3 is mapped, used and highlighted
    plt.imshow(z.T, origin='lower')

    for c in colors:
        plt.scatter(tri[name][c][0], tri[name][c][1], c=c, marker='s', zorder=2)

    plt.savefig('{}.png'.format(name), dpi=100)
    plt.close()


# Workaround to JSON serialize int64
# From https://stackoverflow.com/a/50577730
def json_serialize(o):
    if isinstance(o, np.int64):
        return int(o)
    raise TypeError


if not has_correct:
    print(json.dumps(tri, indent=2, sort_keys=True, default=json_serialize))
