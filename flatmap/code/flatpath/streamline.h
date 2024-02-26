#include <stdlib.h>
#include <stdbool.h>

#include <gsl/gsl_odeiv2.h>
#include <gsl/gsl_roots.h>

int compute_streamline(
        gsl_odeiv2_driver *, // GSL ODE solver driver
        const double[3], // initial position
        const size_t, // streamline index
        const double, // integration step size
        const size_t, // maximum number of iterations
        const size_t, // number of integration steps per iteration
        const struct interp_params *, // interpolator parameters
        size_t *, // number of points in streamline
        double **// streamline points
        );

int streamline_intersection(
        const size_t, // point index
        const size_t, // number of points in streamline
        const double *, // points in streamline
        const double[3], // initial position
        struct interp_params *, // parameters
        const bool, // only full streamlines?
        gsl_root_fsolver *, // root solver
        double[3] // output intersection
        );

int streamline_measure(
        const size_t, // point index
        const size_t, // number of points in streamline
        const double *, // points in streamline
        const double[3], // initial position
        struct interp_params *, // parameters
        const bool, // height or depth?
        const size_t, // number of integration steps
        double * // output value
        );
