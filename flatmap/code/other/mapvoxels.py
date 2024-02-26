import numpy as np
import sys

# Load mapping between hi-res and lo-res voxels
mapping = np.load(sys.argv[1])

# Setup a dict
assoc = { tuple(np.int32(x[range(0,3)])) : tuple(np.int32(x[range(3,6)])) for x in mapping }

sfmap = np.loadtxt(sys.argv[2]) # Load text sorted flatmap at 25um resolution
fvox = np.int32(sfmap[:,(0,1,2)])
mpd = np.array([assoc[tuple(x)] for x in fvox])
mfmap = np.hstack((mpd,sfmap[:,range(3,sfmap.shape[1])])) # Mapped flatmap

np.savetxt(sys.argv[3],mfmap,"%g") # Save text mapped flatmap
