
#include "interp_field.h"
#include <string.h>
#include <math.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>

/* 
 * Linear interpolation of a vector field
 * Input position is in Cartesian coordinates (um)
 * Output vector is normalized
 * NOTE: we check if position is in bounds
 */
#define C(j,i,k) ((size_t)(j) + (size_t)(i) * nx + (size_t)(k) * nx * ny)
void interp_field (
        const float * const restrict x, // array with x vector component
        const float * const restrict y, // array with y vector component
        const float * const restrict z, // array with z vector component
        const size_t nx, // number of voxels in x dimension
        const size_t ny, // number of voxels in y dimension
        const size_t nz, // number of voxels in z dimension
        const double vx, // voxel x size (um)
        const double vy, // voxel y size (um)
        const double vz, // voxel z size (um)
        const double ox, // origin of x coordinate (um)
        const double oy, // origin of y coordinate (um)
        const double oz, // origin of z coordinate (um)
        const double pos_in[static 3], // input XYZ position (um)
        double (*vec_out)[3] // output direction vector
        ) {

            /* normalize position to [0.0:dimension) */
        const double cx = (pos_in[0] - ox) / vx;
        /* coordinate in [0:1] w.r.t. nearest boundaries */
        const double xd = fmod(cx + 0.5,1.0);
        /* one minus the above */
        const double omxd = 1.0 - xd;
        /* array index of 000 corner */
        const size_t j = floor(cx - 0.5);
           /* normalize position to [0.0:dimension) */
        const double cy = (pos_in[1] - oy) / vy;
        /* coordinate in [0:1] w.r.t. nearest boundaries */
        const double yd = fmod(cy + 0.5,1.0);
        /* one minus the above */
        const double omyd = 1.0 - yd;
        /* array index of 000 corner */
        const size_t i = floor(cy - 0.5);
           /* normalize position to [0.0:dimension) */
        const double cz = (pos_in[2] - oz) / vz;
        /* coordinate in [0:1] w.r.t. nearest boundaries */
        const double zd = fmod(cz + 0.5,1.0);
        /* one minus the above */
        const double omzd = 1.0 - zd;
        /* array index of 000 corner */
        const size_t k = floor(cz - 0.5);
   
    if(j >= nx||i >= ny||k >= nz) {
        (*vec_out)[0] = NAN;(*vec_out)[1] = NAN;(*vec_out)[2] = NAN;        return; // outputs NAN vector
    }

    /* interpolate vector components */
            double res_x;
    {
        double v[8];

        /* Get weighted values from all corners */
        v[0] = x[C(j + 0,i + 0,k + 0)] * omxd * omyd * omzd;
        v[1] = x[C(j + 0,i + 0,k + 1)] * omxd * omyd *   zd;
        v[2] = x[C(j + 0,i + 1,k + 0)] * omxd *   yd * omzd;
        v[3] = x[C(j + 0,i + 1,k + 1)] * omxd *   yd *   zd;
        v[4] = x[C(j + 1,i + 0,k + 0)] *   xd * omyd * omzd;
        v[5] = x[C(j + 1,i + 0,k + 1)] *   xd * omyd *   zd;
        v[6] = x[C(j + 1,i + 1,k + 0)] *   xd *   yd * omzd;
        v[7] = x[C(j + 1,i + 1,k + 1)] *   xd *   yd *   zd;

        /* Pairwise summation */
        v[0] = v[0] + v[1];
        v[2] = v[2] + v[3];
        v[4] = v[4] + v[5];
        v[6] = v[6] + v[7];

        v[0] = v[0] + v[2];
        v[4] = v[4] + v[6];

        res_x = v[0] + v[4];
    }
       double res_y;
    {
        double v[8];

        /* Get weighted values from all corners */
        v[0] = y[C(j + 0,i + 0,k + 0)] * omxd * omyd * omzd;
        v[1] = y[C(j + 0,i + 0,k + 1)] * omxd * omyd *   zd;
        v[2] = y[C(j + 0,i + 1,k + 0)] * omxd *   yd * omzd;
        v[3] = y[C(j + 0,i + 1,k + 1)] * omxd *   yd *   zd;
        v[4] = y[C(j + 1,i + 0,k + 0)] *   xd * omyd * omzd;
        v[5] = y[C(j + 1,i + 0,k + 1)] *   xd * omyd *   zd;
        v[6] = y[C(j + 1,i + 1,k + 0)] *   xd *   yd * omzd;
        v[7] = y[C(j + 1,i + 1,k + 1)] *   xd *   yd *   zd;

        /* Pairwise summation */
        v[0] = v[0] + v[1];
        v[2] = v[2] + v[3];
        v[4] = v[4] + v[5];
        v[6] = v[6] + v[7];

        v[0] = v[0] + v[2];
        v[4] = v[4] + v[6];

        res_y = v[0] + v[4];
    }
       double res_z;
    {
        double v[8];

        /* Get weighted values from all corners */
        v[0] = z[C(j + 0,i + 0,k + 0)] * omxd * omyd * omzd;
        v[1] = z[C(j + 0,i + 0,k + 1)] * omxd * omyd *   zd;
        v[2] = z[C(j + 0,i + 1,k + 0)] * omxd *   yd * omzd;
        v[3] = z[C(j + 0,i + 1,k + 1)] * omxd *   yd *   zd;
        v[4] = z[C(j + 1,i + 0,k + 0)] *   xd * omyd * omzd;
        v[5] = z[C(j + 1,i + 0,k + 1)] *   xd * omyd *   zd;
        v[6] = z[C(j + 1,i + 1,k + 0)] *   xd *   yd * omzd;
        v[7] = z[C(j + 1,i + 1,k + 1)] *   xd *   yd *   zd;

        /* Pairwise summation */
        v[0] = v[0] + v[1];
        v[2] = v[2] + v[3];
        v[4] = v[4] + v[5];
        v[6] = v[6] + v[7];

        v[0] = v[0] + v[2];
        v[4] = v[4] + v[6];

        res_z = v[0] + v[4];
    }
   
    /* normalize output vector */
    const double norm2 = res_x * res_x + res_y * res_y + res_z * res_z;
    const double norm = sqrt(norm2);
    const double norm_checked = norm > NORM_EPSILON ? norm : 1.0;

            (*vec_out)[0] = res_x / norm_checked;
           (*vec_out)[1] = res_y / norm_checked;
           (*vec_out)[2] = res_z / norm_checked;
   
#if DEBUG > 1
    info("%g %g %g |"
            " %g %g %g |"
            " %g %g %g |"
            " %g %g %g |"
            " %zu %zu %zu |"
            " %g %g %g",
            pos_in[0],pos_in[1],pos_in[2],
            cx,cy,cz,
            xd,yd,zd,
            omxd,omyd,omzd,
            j,i,k,
            (*vec_out)[0],(*vec_out)[1],(*vec_out)[2]);
#endif
}
#undef C

/*
 * Wrapper of interpolated orientation field for use with GSL ODE solver
 */
int dirfield(double t, const double y[], double f[], void *params) {
    (void)t;
    struct interp_params *p = (struct interp_params*)params;
    double vec_out[3];
    interp_field(p->x,p->y,p->z,
                 p->nx,p->ny,p->nz,
                 p->vx,p->vy,p->vz,
                 p->ox,p->oy,p->oz,
                 y,&vec_out);
    memcpy(f,vec_out,3 * sizeof(double));
    return GSL_SUCCESS;
}

/*
 * Get relative depth at position
 */
double get_rdepth(const double x, const double y, const double z, const void *params) {
    const struct interp_params *p = (const struct interp_params*)params;

            /* normalize position to [0.0:dimension) */
        const double cx = (x - p->ox) / p->vx;
        /* array index of voxel */
        const size_t j = floor(cx);
           /* normalize position to [0.0:dimension) */
        const double cy = (y - p->oy) / p->vy;
        /* array index of voxel */
        const size_t i = floor(cy);
           /* normalize position to [0.0:dimension) */
        const double cz = (z - p->oz) / p->vz;
        /* array index of voxel */
        const size_t k = floor(cz);
   
    if(j >= p->nx||i >= p->ny||k >= p->nz) {
        return NAN;
    }

    size_t ix = (size_t)j + (size_t)i * p->nx + (size_t)k * p->nx * p->ny;

#if DEBUG > 2
    info("L %g %g %g [%zu, %zu, %zu] -> %g",
            x,y,z,j,i,k,p->rdepth[ix]);
#endif

    return p->rdepth[ix];
}

/*
 * Get relative depth at spline location
 */
double eval_spline_rdepth(const double t, const void *params) {
    const struct interp_params *p = (const struct interp_params*)params;

            const double x = gsl_spline_eval(p->splx,t,p->accx); // evaluate spline
           const double y = gsl_spline_eval(p->sply,t,p->accy); // evaluate spline
           const double z = gsl_spline_eval(p->splz,t,p->accz); // evaluate spline
   
    return get_rdepth(x,y,z,params);
}

/*
 * Get size of derivative along streamline (measure length increment)
 */
double get_spline_dlen(const double t, const void *params) {
    const struct interp_params *p = (const struct interp_params*)params;

            const double x_0 = gsl_spline_eval(p->splx,t,p->accx); // evaluate spline at t
        const double x = gsl_spline_eval(p->splx,t + p->dt,p->accx); // evaluate spline at t + dt
           const double y_0 = gsl_spline_eval(p->sply,t,p->accy); // evaluate spline at t
        const double y = gsl_spline_eval(p->sply,t + p->dt,p->accy); // evaluate spline at t + dt
           const double z_0 = gsl_spline_eval(p->splz,t,p->accz); // evaluate spline at t
        const double z = gsl_spline_eval(p->splz,t + p->dt,p->accz); // evaluate spline at t + dt
   
    if(isnan(x) || isnan(y) || isnan(z)) return 0.0;

    double len = gsl_hypot3(x - x_0,y - y_0,z - z_0); // length of segment from t -> t + dt
#if DEBUG > 1
    fprintf(stderr,"%g: %g %g %g -> %g: %g %g %g = %g\n",t,x_0,y_0,z_0,t + p->dt,x,y,z,dist);
#endif

    return len;
}
