#include <stdlib.h>

#include <gsl/gsl_spline.h>

/* vectors smaller than this are not normalized */
#define NORM_EPSILON 1E-12

enum proj_side {
    TOP,
    BOTTOM
};

struct interp_params {
    float *rdepth; // relative depth
    float *x,*y,*z; // orientation field
    size_t nx; // matrix X dimension
    size_t ny; // matrix Y dimension
    size_t nz; // matrix Z dimension
    double vx,vy,vz; // voxel dimensions
    double ox,oy,oz; // coordinate offsets
    double proj_rdepth; // projection depth
    double eps_zero,eps_one; // epsilons
    double dt; // spline parameter delta
    gsl_spline *splx,*sply,*splz; // splines per coordinate
    gsl_interp_accel *accx,*accy,*accz; // interpolation accelerators
    enum proj_side proj_side; // projection surface side
};

void interp_field (
        const float * const restrict, // array with x vector component
        const float * const restrict, // array with y vector component
        const float * const restrict, // array with z vector component
        const size_t, // number of voxels in x dimension
        const size_t, // number of voxels in y dimension
        const size_t, // number of voxels in z dimension
        const double, // voxel x size (um)
        const double, // voxel y size (um)
        const double, // voxel z size (um)
        const double, // origin of x coordinate (um)
        const double, // origin of y coordinate (um)
        const double, // origin of z coordinate (um)
        const double[static 3], // input XYZ position (um)
        double (*)[3] // output direction vector
        );

int dirfield(
        double,
        const double [],
        double [],
        void *
        );

double get_rdepth(
        const double,
        const double,
        const double,
        const void *
        );

double eval_spline_rdepth(
        const double,
        const void *
        );

double get_spline_dlen(
        const double,
        const void*
        );
