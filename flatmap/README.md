# Flatmapping algorithm

## Code

Contents:

+ `flatpath`: C code to compute streamline-related tasks.
+ `metrics`: Python code to compute flatmap metrics.
+ `surf_cgal`: C++ code for CGAL-powered code to work with meshes.
+ `utils`: mostly Python scripts and two AWK helper scripts to perform various tasks.

To compile, gather dependencies, change directory to `code` and run `make`!

## Workflow

Contents (stages):

+ `01_stageI`: run stage I "flat mesh generation", independent from stage II.
+ `02_stageII`: run stage II "voxel projection", independent from stage I.
+ `03_stageIII`: run stage III "flatmap generation", depends on stages I and II.
+ `metrics`: run metrics for flatmap characterization, with the flatmap generated in stage III.
+ `applications`: run applications of flatmaps, with the flatmap generated in stage III.

The flatmapping workflow is implemented using Makefiles. Each stage has multiple steps, each with clearly defined `input` and `output` files. The workflow is designed to be self-contained, and no user modification of files is required (except paths to optional programs in `config.mk`).

To create a flatmap:

1. Fire up a terminal.
2. Clone this repository.
3. Compile the code as explained above.
4. Setup a Python environment using the provided `requirements.txt` file and activate it.
5. Set the environment variable `ATLAS_ENHANCEMENT_ROOT` to the absolute path of the local clone of this repository.
6. Set the environment variable `USER_DATA_ROOT` to the absolute path of the directory containing the input files.
7. Change directory to `$ATLAS_ENHANCEMENT_ROOT/flatmap/workflow`.
8. Run `make`!

### Input file requirements

+ `config.mk`: configuration file tuning the flatmapping algorithm. Use the provided `user_config_sscx.mk` or `user_config_isocortex.mk` as examples.
+ `relative_depth.nrrd`: relative depth field as 32-bit floating-point (float) NRRD dataset. Values must be 1 at the top shell and 0 at the bottom shell. Background value must be NaN.
+ `orientation_{x,y,z}.nrrd`: components of the local orientation vector as 32-bit floating-point (float) NRRD datasets. Background value must be 0. The vector must point towards the top shell (where `relative_depth.nrrd` is 1).
+ `annotations.nrrd` (optional): region labels as an integer NRRD dataset. Used to visualize the flatmap in `applications/flatview_annotations`.
+ `mask.nrrd` (optional): labeling of inner (1) and outer (0) voxels, and top (4) / sides (2) / bottom (3) shells, as an integer NRRD dataset.

+ NRRD datasets must include voxel dimensions and space origin metadata, and must have the same data dimensions. Check files in Zenodo repo for examples. Preferably generate/save these files using the Python package `voxcell`.

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

## Optional dependencies

+ Intel MKL (proprietary), to speed up iterative flattening algorithm.
+ GNU Parallel (`parallel`), to compute streamlines in parallel across a cluster.
+ gnuplot (`gnuplot`), to view diagnostic plots.
+ Some image viewer, to view generated heatmaps.
+ Some NRRD viewer (e.g., `xgrid3d` or `itk-snap`), to view generated NRRD datasets.
