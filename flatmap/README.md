# Flatmapping algorithm

## Code

### Contents

+ `flatpath`: C code to compute streamlines and streamline-derived quantities.
+ `metrics`: Python code to compute flatmap metrics.
+ `surf_cgal`: C++ code using CGAL to work with meshes (flattening, etc.).
+ `utils`: Python helper scripts to perform various tasks.

### Compilation

1. Gather dependencies (listed below).
2. Change directory to `code`.
3. Set environment variable `CGAL_ROOT` to the location of CGAL source code.
4. Run `make`!

## Workflow

### Stages

The flatmapping workflow is implemented using Makefiles. Each stage has multiple steps, each with clearly defined `input` and `output` files. The workflow is designed to be self-contained, and no user modification of files is required (except possibly paths to binaries in `config.mk`).

+ `01_stageI`: run stage I "flat mesh generation", independent from stage II.
+ `02_stageII`: run stage II "voxel projection", independent from stage I.
+ `03_stageIII`: run stage III "flatmap generation", depends on stages I and II.
+ `postproc`: perform post-processing tasks, with the flatmap generated in stage III.
+ `metrics`: compute metrics for flatmap characterization, with the flatmap generated in stage III.
+ `applications`: implement applications of flatmaps, with the flatmap generated in stage III.

### Flatmap generation

1. Fire up a terminal.
2. Clone this repository.
3. Compile the code as explained above.
4. Setup a Python environment using the provided `requirements.txt` file and activate it.
5. Set the environment variable `ATLAS_ENHANCEMENT_ROOT` to the location of the local clone of this repository.
6. Set the environment variable `USER_DATA_ROOT` to the location of the directory containing the input files.
7. Change directory to `$ATLAS_ENHANCEMENT_ROOT/flatmap/workflow`.
8. Run `make`!

### Data visualization in flat space / flat view generation

Input data must be in NRRD format with same dimensions as flatmap, e.g. data registered to CCFv3 at 10 μm when using mouse isocortex flatmap at 10 μm.

As part of the workflow:

1. Change directory to `applications/flatview_3d_data`.
2. Run `make FLATVIEW_DATA_INPUT_FILE="<path to input data>"`.
3. Extra arguments to `flatplot` can be passed in the variable `FLATPLOT_DATA_EXTRA`.
4. Find output image under `output/`.

Stand-alone:

1. The [flatplot](code/utils/flatplot.py) script can be found in `$ATLAS_ENHANCEMENT_ROOT/flatmap/code/utils/flatplot.py`.
2. Run it like `flatplot <flatmap NRRD> <data NRRD> <output prefix>`.
3. Some common options are:
   + `--flatpix`: specify pixel resolution of output image
   + `--autospan`: get value range from data, instead of [0,1]
   + `--colormap`: set color map from `colorcet` as `cet:*`, from `seaborn` as `sns:*`, from `cmcrameri` as `cmc:*`, or as a comma-separated list of hexadecimal RGB colors (`#XXXXXX`)
   + `--reduction`: specify aggregation function, e.g. `max`, `mean`, etc.
   + `--layers`: path to layers NRRD (1-based integers), to be used with `--split` to plot each layer separately, or `--only-layer` to plot a specific layer.
4. For full CLI options, run `flatplot --help`.

### Input file requirements

NRRD datasets must include voxel dimensions and space origin metadata, and must have the same data dimensions. Check files in Zenodo repo for examples. Preferably generate/save these files using the Python package `voxcell`.

+ `config.mk`: configuration file for the flatmapping algorithm. For example, see the provided [user_config_sscx.mk](examples/user_config_sscx.mk) or [user_config_isocortex.mk](examples/user_config_isocortex.mk).
+ `relative_depth.nrrd`: relative depth field as 32-bit floating-point NRRD dataset. Values must be 1 at the top shell and 0 at the bottom shell. Background value must be NaN.
+ `orientation_{x,y,z}.nrrd`: components of the local orientation vector as 32-bit floating-point NRRD datasets. Background value must be 0. The vector must point towards the top shell (where `relative_depth.nrrd` is 1).
+ `annotations.nrrd` (optional): sequential region labels as an integer NRRD dataset. Used to visualize region annotations in flat space in `applications/flatview_annotations`.
+ `mask.nrrd` (optional): labeling of inner (1) and outer (0) voxels, and top (4) / sides (2) / bottom (3) shells, as an integer NRRD dataset.
+ `hemispheres.nrrd` (optional): labeling of right (1) and left (2) hemispheres, as an integer NRRD dataset. Used to mirror flatmaps in `postproc/both_hemispheres`.

## Dependencies

+ GNU Make (`make`)
+ C compiler (`gcc` or `clang`)
+ C++ compiler (`g++` or `clang++`)
+ CMake (`cmake`)
+ CGAL v5.6+ ([website](https://www.cgal.org))
+ Boost (`libboost-dev`)
+ Eigen3 (`libeigen3-dev`)
+ GMP (`libgmp-dev`)
+ MPFR (`libmpfr-dev`)
+ GMSH (`gmsh`, [website](http://gmsh.info))
+ Python packages in `requirements.txt`

### Custom convex border parameterization in CGAL

Requires copying the following files from [my branch](https://github.com/seirios/cgal/tree/border-parameterization/Surface_mesh_parameterization/include/CGAL/Surface_mesh_parameterization) (until https://github.com/CGAL/cgal/pull/8180 is merged):

+ `Convex_border_parameterizer_3.h`
+ `Parametric_curve_border_parameterizer_3.h`

into `include/CGAL/Surface_mesh_parameterization` in the CGAL source distribution.

## Optional dependencies

+ Intel MKL (proprietary), to speed up iterative mesh flattening with PARDISO solver.
+ GNU Parallel (`parallel`), to compute streamlines in parallel.
+ gnuplot (`gnuplot`), to view diagnostic plots.
+ Some image viewer (e.g., `feh`), to view generated heatmaps.
+ Some NRRD viewer (e.g., `xgrid3d` or `itk-snap`), to view generated NRRD datasets.
