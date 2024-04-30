## relative depth of projection surface
PROJECTION_SURFACE_DSTAR := 0.5

## side of projection surface
PROJECTION_SURFACE_SIDE := top

## extra arguments to surface reconstruction algorithm (PCAlpha)
### number of neighbors
### number of samples
### number of iterations
RECONSTRUCT_SURFACE_EXTRA := 12 300 4

## number of mesh refinement steps
NREFINE := 3

## change flattening algorithm
FLATTEN_MESH_BIN := $(SOURCE_CODE_ROOT)/surf_cgal/build/Flatten_Authalic_Iterative_convex

## extra arguments to flattening algorithm (FlattenAuthalicIterative_Convex)
### border points
### number of iterations
### offset of first corner along boundary
### corners .selection.txt file
FLATTEN_MESH_EXTRA := border_points.txt 10 -1840# offset = -230 * 2 ** NREFINE

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
PIXEL_RESOLUTION := 256

## color map for flat view of annotations
FLATVIEW_ANNOTATIONS_COLORMAP := '\#a6cee3,\#fb9a99,\#6a3d9a,\#ffff99,\#fdbf6f,\#fdbf6f,\#ff7f00,\#cab2d6,\#6a3d9a,\#a6cee3,\#b2df8a,\#1f78b4,\#cab2d6,\#b2df8a,\#33a02c,\#1f78b4,\#fb9a99,\#e31a1c,\#fb9a99,\#1f78b4,\#ff7f00,\#fdbf6f,\#ff7f00,\#ffff99,\#cab2d6,\#33a02c,\#e31a1c,\#6a3d9a,\#ffff99,\#fdbf6f,\#a6cee3,\#b2df8a,\#e31a1c,\#a6cee3,\#e31a1c,\#33a02c,\#6a3d9a,\#cab2d6,\#b2df8a,\#33a02c,\#fb9a99,\#1f78b4,\#ff7f00'

## extra arguments to heatmap plots
PREIMAGE_SIZE_DISCRETE_PLOT_EXTRA := --minval=0.0 -c sns:viridis
PREIMAGE_SIZE_UNIFORMITY_PLOT_EXTRA := --minval=-1.0 --maxval=3.0 -c sns:coolwarm
PREIMAGE_CONNECTEDNESS_PLOT_EXTRA := --minval=0 --maxval=1 -c sns:viridis
PAIRWISE_PREIMAGE_CONTINUITY_PLOT_EXTRA := --minval=0.0 --maxval=1.0 -c cmc:buda
PREIMAGE_RADIUS_PLOT_EXTRA := --minval=0.0 -c sns:magma
PREIMAGE_CONICITY_PLOT_EXTRA := --minval=-3E-6 --maxval=3E-6 -c cmc:vik

## applications
HEXGRID_DIAMETER := 0.03
