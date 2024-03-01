## relative depth of projection surface
PROJECTION_SURFACE_DSTAR := 0.5

## side of projection surface
PROJECTION_SURFACE_SIDE := top

## extra arguments to surface reconstruction algorithm (PCAlpha)
### number of neighbors
### number of samples
### number of iterations
RECONSTRUCT_SURFACE_EXTRA := 16 300 6

## number of mesh refinement steps
NREFINE := 3

## extra arguments to flattening algorithm (FlattenAuthalic)
### offset first corner along boundary
### corners .selection.txt file
FLATTEN_MESH_EXTRA := 0 #empty

## how many streamlines computed per run?
FLATPATH_BLOCKSIZE := 100000

## use full streamlines only?
FLATPATH_FULL_STREAMLINES := 1

## streamline integration time step
FLATPATH_TIMESTEP := 0.5

## streamline maximum iterations
FLATPATH_MAXITER := 5000

## streamline epsilon-close to zero
FLATPATH_EPSILON_ZERO := 0.5

## streamline epsilon-close to one
FLATPATH_EPSILON_ONE := 0.5

## extra arguments to flatpath
FLATPATH_EXTRA := #empty

## extra arguments to flatmap maker
### rotate by given angle (90, 180 or 270)
### flip X?
### flip Y?
FLATMAP_EXTRA := 0 0 0

## pixel resolution of discretized flatmap
PIXEL_RESOLUTION := 160

## color map for flat view of annotations
FLATVIEW_ANNOTATIONS_COLORMAP := '\#a6cee3,\#1f78b4,\#b2df8a,\#33a02c,\#fb9a99,\#e31a1c,\#fdbf6f,\#ff7f00,\#cab2d6,\#6a3d9a,\#ffff99'

## extra arguments to heatmap plots
PREIMAGE_SIZE_DISCRETE_PLOT_EXTRA := --minval=0.0 -c sns:viridis
PREIMAGE_SIZE_UNIFORMITY_PLOT_EXTRA := --minval=-1.0 --maxval=3.0 -c sns:coolwarm
PREIMAGE_CONNECTEDNESS_PLOT_EXTRA := --minval=0 --maxval=1 -c sns:viridis
PAIRWISE_PREIMAGE_CONTINUITY_PLOT_EXTRA := --minval=0.0 --maxval=1.0 -c cmc:buda
PREIMAGE_RADIUS_PLOT_EXTRA := --minval=0.0 -c sns:magma
PREIMAGE_CONICITY_PLOT_EXTRA := --minval=-5E-7 --maxval=5E-7 -c cmc:vik

## applications
HEXGRID_DIAMETER := 0.07
