#!/bin/env python3

import voxcell as vc
import numpy as np
import sys

vd = vc.VoxelData.load_nrrd(sys.argv[1])

fmap = np.full(list(vd.raw.shape)+[2],-1)

dat = np.loadtxt(sys.argv[2],dtype = np.float64)

w = dat[:,[0,1,2]]
w = np.uint16(w)
w = tuple(w.T)

find_max = False
use_float = False

scale = 0
perc_usage = 0.90 # meaningful default
if len(sys.argv) > 4:
    strarg = sys.argv[4]
    try:
        stri = int(strarg)
    except ValueError:  # not integer argument
        perc_usage = float(strarg)
        if perc_usage > 1.0:  # find max instead
            find_max = True
        elif perc_usage < 0.0:  # use float instead
            use_float = True
    else:  # integer argument
        scale = stri

def pixel_usage(scale):
    pos = np.uint16(np.floor(dat[:,[3,4]] * (scale - 1E-9)))
    z = np.zeros((scale+1,scale+1))
    z[tuple(pos.T)] = 1
    n = np.sum(z)
    fn = n/((scale+1)**2)
    return fn, n

if find_max:
    fnopt = 0
    scaleopt = 1
    scalemax = 400
    if scale == 0: # optimize scale to use maximum % of pixels
        scale = 2
        while scale < scalemax:
            scale += 2
            fn, n = pixel_usage(scale)
            print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))
            if fn > fnopt:
                fnopt = fn
                scaleopt = scale
    scale = scaleopt
    print('Using max {}% pixels at scale {}'.format(fnopt * 100.0,scaleopt))
elif not use_float:
    if scale == 0: # optimize scale to use at least perc_usage% pixels
        scale = 32 # start here to avoid local small optima
        fn, n = pixel_usage(scale)
        print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))
        # coarse search
        while n/((scale + 1) ** 2) > perc_usage:
            scale *= 2
            fn, n = pixel_usage(scale)
            print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))
        scale = scale // 2 # previous value
        # fine search
        while n/((scale + 1) ** 2) > perc_usage:
            scale += 2
            fn, n = pixel_usage(scale)
            print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))
        scale -= 2
        fn, n = pixel_usage(scale)
        print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))
    else: # use given scale
        fn, n = pixel_usage(scale)
        print('Using {}% pixels at scale {}'.format(fn * 100.0,scale))

# Final discretization
if use_float:
    print('Floating-point flatmap')
    fmap = np.float32(fmap)
    pos = dat[:,[3,4]]
    fmax = 1.0
else:
    fn, n = pixel_usage(scale)
    print('Final: using {}% pixels at scale {}'.format(fn * 100.0,scale))
    pos = np.uint16(np.floor(dat[:,[3,4]] * (scale - 1E-9)))
    fmax = np.max(pos)
    assert(fmax == scale - 1)

if(len(sys.argv) > 5):
    angle = int(sys.argv[5])
    if not angle in [0,90,180,270]:
        raise ValueError('Angle must be 0, 90, 180 or 270')
    print('Rotating {} degrees CCW'.format(angle))
    if angle == 90:
        tmp = pos[:,0].copy()
        pos[:,0] = fmax - pos[:,1]  # x -> -y
        pos[:,1] = tmp              # y -> x
    elif angle == 180:
        pos[:,0] = fmax - pos[:,0]  # x -> -x
        pos[:,1] = fmax - pos[:,1]  # y -> -y
    elif angle == 270:
        tmp = pos[:,0].copy()
        pos[:,0] = pos[:,1]         # x -> y
        pos[:,1] = fmax - tmp       # y -> -x

if len(sys.argv) > 6:
    fliph = int(sys.argv[6])
    if not fliph in [0,1]:
        raise ValueError('Flip horizontal must be 0 or 1')
    if fliph == 1:
        print('Flipping horizontally after rotation')
        pos[:,0] = fmax - pos[:,0]  # x -> -x

if len(sys.argv) > 7:
    flipv = int(sys.argv[7])
    if not fliph in [0,1]:
        raise ValueError('Flip vertical must be 0 or 1')
    if flipv == 1:
        print('Flipping vertically after rotation')
        pos[:,1] = fmax - pos[:,1]  # y -> -y

out = vc.VoxelData(fmap,vd.voxel_dimensions,vd.offset)
out.raw[w] = pos

out.save_nrrd(sys.argv[3])
