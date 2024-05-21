
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "interp_field.h"
#include "streamline.h"

#include <gsl/gsl_errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>

#ifndef info0
#define info0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[39m","flatpath", "INFO"   )\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "INFO"   )))
#endif
#ifndef warn0
#define warn0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;11m","flatpath", "WARN"   )\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "WARN"   )))
#endif
#ifndef error0
#define error0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;9m","flatpath", "ERROR"   )\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "ERROR"   )))
#endif
#ifndef verbose0
#define verbose0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;12m","flatpath", "VERB"   )\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "VERB"   )))
#endif
#ifndef debug0
#define debug0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: " "%s: " M "\x1B[39m" "\n","\x1B[38;5;13m","flatpath", "DEBUG" , __func__ )\
            : fprintf(stderr,"[%s] %5s: " "%s: " M "\n","flatpath", "DEBUG" , __func__ )))
#endif
#ifndef info
#define info(M,...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[39m","flatpath", "INFO"   ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "INFO"   ,__VA_ARGS__)))
#endif
#ifndef warn
#define warn(M,...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;11m","flatpath", "WARN"   ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "WARN"   ,__VA_ARGS__)))
#endif
#ifndef error
#define error(M,...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;9m","flatpath", "ERROR"   ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "ERROR"   ,__VA_ARGS__)))
#endif
#ifndef verbose
#define verbose(M,...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: "  M "\x1B[39m" "\n","\x1B[38;5;12m","flatpath", "VERB"   ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: "  M "\n","flatpath", "VERB"   ,__VA_ARGS__)))
#endif
#ifndef debug
#define debug(M,...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: " "%s: " M "\x1B[39m" "\n","\x1B[38;5;13m","flatpath", "DEBUG" , __func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: " "%s: " M "\n","flatpath", "DEBUG" , __func__ ,__VA_ARGS__)))
#endif

int compute_streamline(
        gsl_odeiv2_driver *d, // GSL ODE solver driver
        const double yinit[3], // initial position
        size_t ix, // streamline index
        const double h, // integration step size
        const size_t maxiter, // maximum number of iterations
        const size_t nstep, // number of integration steps per iteration
        const struct interp_params *p, // interpolator parameters
        size_t *sn_out, // number of points in streamline
        double **sx_out // streamline points
        ) {
    int retcode = 0;

                /* forward time */
        size_t iflast = maxiter;
        double savef[maxiter][3];
        memset(savef,0x0,maxiter * 3 * sizeof(double));
        {
            double t = 0.0;
            double y[3] = { yinit[0], yinit[1], yinit[2] };
            bool break_next = false;
            for(size_t i=0;i<maxiter;i++) {
                int s = gsl_odeiv2_driver_apply_fixed_step(d,&t,1 * h,nstep,y); // advance position

                if(s != GSL_SUCCESS) {
                    error("driver returned %d for streamline %zu (%g %g %g)",
                          s,ix,yinit[0],yinit[1],yinit[2]);
                    iflast = i;
                    break;
                }

                double layer = get_rdepth(y[0],y[1],y[2],p); // layer at new position

                if(break_next ||
                        isnan(y[0]) || isnan(y[1]) || isnan(y[2]) || isnan(layer)) {
#if DEBUG
                    info("stopping forward at iteration %zu: %g %g %g -> %g",
                         i,y[0],y[1],y[2],layer);
#endif
                    iflast = i;
                    break;
                }

#if DEBUG
                info("iteration %zu: %g %g %g -> %g",
                     i,y[0],y[1],y[2],layer);
#endif

                memcpy(savef[i],y,3 * sizeof(double)); // store position along streamline
            }
        }
        if(iflast == maxiter) {
            warn("streamline %zu (%g %g %g) may exceed %zu forward iterations",
                 ix,yinit[0],yinit[1],yinit[2],iflast);
        }
           /* backwards time */
        size_t iblast = maxiter;
        double saveb[maxiter][3];
        memset(saveb,0x0,maxiter * 3 * sizeof(double));
        {
            double t = 0.0;
            double y[3] = { yinit[0], yinit[1], yinit[2] };
            bool break_next = false;
            for(size_t i=0;i<maxiter;i++) {
                int s = gsl_odeiv2_driver_apply_fixed_step(d,&t,-1 * h,nstep,y); // advance position

                if(s != GSL_SUCCESS) {
                    error("driver returned %d for streamline %zu (%g %g %g)",
                          s,ix,yinit[0],yinit[1],yinit[2]);
                    iblast = i;
                    break;
                }

                double layer = get_rdepth(y[0],y[1],y[2],p); // layer at new position

                if(break_next ||
                        isnan(y[0]) || isnan(y[1]) || isnan(y[2]) || isnan(layer)) {
#if DEBUG
                    info("stopping backwards at iteration %zu: %g %g %g -> %g",
                         i,y[0],y[1],y[2],layer);
#endif
                    iblast = i;
                    break;
                }

#if DEBUG
                info("iteration %zu: %g %g %g -> %g",
                     i,y[0],y[1],y[2],layer);
#endif

                memcpy(saveb[i],y,3 * sizeof(double)); // store position along streamline
            }
        }
        if(iblast == maxiter) {
            warn("streamline %zu (%g %g %g) may exceed %zu backwards iterations",
                 ix,yinit[0],yinit[1],yinit[2],iblast);
        }
   
    { // store streamline
        size_t sn = iblast + 1 + iflast; // number of points in streamline
        double *sx = malloc(3 * sn * sizeof(double));

        size_t k = 0;
        /* backward steps in reverse */
        for(size_t i=0;i<iblast;i++) {
            size_t j = iblast - 1 - i;
            if(saveb[j][0] == 0.0 || saveb[j][1] == 0.0 || saveb[j][2] == 0.0)
                error("zero found in backward streamline %zu",ix);
            sx[3 * k + 0] = saveb[j][0];sx[3 * k + 1] = saveb[j][1];sx[3 * k + 2] = saveb[j][2];            k++;
        }
        /* initial point */
        sx[(size_t)0 + 3 * k] = yinit[0];sx[(size_t)1 + 3 * k] = yinit[1];sx[(size_t)2 + 3 * k] = yinit[2];        k++;
        /* forward steps */
        for(size_t i=0;i<iflast;i++) {
            if(savef[i][0] == 0.0 || savef[i][1] == 0.0 || savef[i][2] == 0.0)
                error("zero found in forward streamline %zu",ix);
            sx[3 * k + 0] = savef[i][0];sx[3 * k + 1] = savef[i][1];sx[3 * k + 2] = savef[i][2];            k++;
        }
        assert(k == sn);

        *sn_out = sn;
        *sx_out = sx;
    }

    return retcode;
}

#define NEAR_ZERO(x) (fabs(x) < p->eps_zero)
#define NEAR_ONE(x) (fabs(1.0 - (x)) < p->eps_one)



/*
 * This function equals
 * -0.5 at relative depth < projection surface
 * and +0.5 at relative depth >= projection surface,
 * so it attains its zero at the projection surface
 */
static double streamln(const double t, void *params) {
    const struct interp_params *p = (const struct interp_params*)params;

    const double rdepth = eval_spline_rdepth(t,params);
    const double f = (p->proj_side == BOTTOM)
        ? (rdepth <= p->proj_rdepth ? -0.5 : 0.5) // bottom
        : (rdepth <  p->proj_rdepth ? -0.5 : 0.5) // top
        ;

    return f;
}

int streamline_intersection(
        const size_t ix, // point index
        const size_t sn, // number of points in streamline
        const double *sx, // points in streamline
        const double yinit[3], // initial position
        struct interp_params *p, // parameters
        const bool full_only, // only full streamlines?
        gsl_root_fsolver *rs, // root solver
        double isec_o[3] // output intersection
        ) {
    double isec[3] = { NAN, NAN, NAN };

                gsl_interp_accel *accx = gsl_interp_accel_alloc();
        gsl_spline *splx = gsl_spline_alloc(gsl_interp_linear, sn);
           gsl_interp_accel *accy = gsl_interp_accel_alloc();
        gsl_spline *sply = gsl_spline_alloc(gsl_interp_linear, sn);
           gsl_interp_accel *accz = gsl_interp_accel_alloc();
        gsl_spline *splz = gsl_spline_alloc(gsl_interp_linear, sn);
   
    double tinit = -1.0; // initial dummy value
    double *t = malloc(sn * sizeof(double));
    double *x = malloc(sn * sizeof(double));double *y = malloc(sn * sizeof(double));double *z = malloc(sn * sizeof(double));    for(size_t i=0;i<sn;i++) {
        t[i] = (double)i / (sn - 1);
        x[i] = sx[3 * i + 0];y[i] = sx[3 * i + 1];z[i] = sx[3 * i + 2];        if(x[i] == yinit[0] && y[i] == yinit[1] && z[i] == yinit[2])
            tinit = t[i]; // update parameter value for initial point
    }
    if(false) {
        if(tinit == -1.0) { error("No tinit found for streamline %zu",ix); }
        else if(tinit == 0.0 || tinit == 1.0) warn("Bad tinit = %g found for streamline %zu",tinit,ix);
    }

    gsl_spline_init(splx,t,x,sn);gsl_spline_init(sply,t,y,sn);gsl_spline_init(splz,t,z,sn);    p->splx=splx; p->accx=accx;p->sply=sply; p->accy=accy;p->splz=splz; p->accz=accz;
#ifdef DEBUG
    /* print full streamline */
    for(size_t i=0;i<sn;i++)
        fprintf(stderr,"%g %g %g %g %g\n",
                t[i],x[i],y[i],z[i],get_rdepth(x[i],y[i],z[i],p));
#endif

    /* find root */
    const int max_iter = 300;
    gsl_function F = { .function=&streamln, .params=p };
    int status = gsl_root_fsolver_set(rs,&F,0.0,1.0);

    if(status != GSL_SUCCESS ||
            (full_only && (!NEAR_ZERO(eval_spline_rdepth(0.0,p))
                           || !NEAR_ONE(eval_spline_rdepth(1.0,p))))) { // init fail
#if DEBUG
        error("Init fail: %g (%.6f : %s) %g (%.6f : %s) NAN",
                streamln(0.0,p),eval_spline_rdepth(0.0,p),
                NEAR_ZERO(eval_spline_rdepth(0.0,p)) ? "true" : "false",
                streamln(1.0,p),eval_spline_rdepth(1.0,p),
                NEAR_ONE(eval_spline_rdepth(1.0,p)) ? "true" : "false");
        warn("GSL: %s (%d)",gsl_strerror(status),status);
#endif
    } else { // init success
#if DEBUG
        info("Init success: %g (%g) %g (%g)",
                streamln(0.0,p),eval_spline_rdepth(0.0,p),
                streamln(1.0,p),eval_spline_rdepth(1.0,p));
#endif
        int iter = 0;
        double root;
        do {
            iter++;
            status = gsl_root_fsolver_iterate(rs);
            root = gsl_root_fsolver_root(rs);
            double x_lo = gsl_root_fsolver_x_lower(rs);
            double x_hi = gsl_root_fsolver_x_upper(rs);
            status = gsl_root_test_interval(x_lo, x_hi, 0.0, 1e-8);

#if DEBUG
            if(status == GSL_SUCCESS) fprintf(stderr,"Converged:\n");
            fprintf(stderr,"%5d [%g = %g, %g = %g] %g = %g (%g)\n",
                    iter,
                    x_lo, streamln(x_lo,p),
                    x_hi, streamln(x_hi,p),
                    root, streamln(root,p),
                    x_hi - x_lo);
#endif
        }
        while(status == GSL_CONTINUE && iter < max_iter);

        if(status == GSL_SUCCESS) { // root found
            isec[0] = gsl_spline_eval(splx,root,accx);isec[1] = gsl_spline_eval(sply,root,accy);isec[2] = gsl_spline_eval(splz,root,accz);        }
    }

                gsl_spline_free(splx);
        gsl_interp_accel_free(accx);
        free(x);
           gsl_spline_free(sply);
        gsl_interp_accel_free(accy);
        free(y);
           gsl_spline_free(splz);
        gsl_interp_accel_free(accz);
        free(z);
       free(t);
    p->splx=NULL; p->accx=NULL;p->sply=NULL; p->accy=NULL;p->splz=NULL; p->accz=NULL;
    isec_o[0] = isec[0];
    isec_o[1] = isec[1];
    isec_o[2] = isec[2];

    return 0;
}

int streamline_measure(
        const size_t ix, // point index
        const size_t sn, // number of points in streamline
        const double *sx, // points in streamline
        const double yinit[3], // initial position
        struct interp_params *p, // parameters
        const bool do_height, // height or depth?
        const size_t nstep_path, // number of integration steps
        double *val_o // output value
        ) {
    double val = NAN;

                gsl_interp_accel *accx = gsl_interp_accel_alloc();
        gsl_spline *splx = gsl_spline_alloc(gsl_interp_linear, sn);
           gsl_interp_accel *accy = gsl_interp_accel_alloc();
        gsl_spline *sply = gsl_spline_alloc(gsl_interp_linear, sn);
           gsl_interp_accel *accz = gsl_interp_accel_alloc();
        gsl_spline *splz = gsl_spline_alloc(gsl_interp_linear, sn);
   
    double tinit = -1.0; // initial dummy value
    double *t = malloc(sn * sizeof(double));
    double *x = malloc(sn * sizeof(double));double *y = malloc(sn * sizeof(double));double *z = malloc(sn * sizeof(double));    for(size_t i=0;i<sn;i++) {
        t[i] = (double)i / (sn - 1);
        x[i] = sx[3 * i + 0];y[i] = sx[3 * i + 1];z[i] = sx[3 * i + 2];        if(x[i] == yinit[0] && y[i] == yinit[1] && z[i] == yinit[2])
            tinit = t[i]; // update parameter value for initial point
    }
    if(true) {
        if(tinit == -1.0) { error("No tinit found for streamline %zu",ix); }
        else if(tinit == 0.0 || tinit == 1.0) warn("Bad tinit = %g found for streamline %zu",tinit,ix);
    }

    gsl_spline_init(splx,t,x,sn);gsl_spline_init(sply,t,y,sn);gsl_spline_init(splz,t,z,sn);    p->splx=splx; p->accx=accx;p->sply=sply; p->accy=accy;p->splz=splz; p->accz=accz;
    if(NEAR_ZERO(eval_spline_rdepth(0.0,p))
            && NEAR_ONE(eval_spline_rdepth(1.0,p))) { // full streamline
        // height starts at 0.0 and goes in the positive direction
        // depth starts at 1.0 and goes in the negative direction
        p->dt = do_height
             ? 1.0 / nstep_path // positive dt
             : - (1.0 - tinit) / nstep_path; // negative dt
        double t = do_height ? 0.0 : 1.0; // starting point

        double sum = 0.0;
        for(size_t i = 0; i < nstep_path; ++i) { // integrate path length
            sum += get_spline_dlen(t,p);
            t += p->dt;
        }

        val = sum;
    }

                gsl_spline_free(splx);
        gsl_interp_accel_free(accx);
        free(x);
           gsl_spline_free(sply);
        gsl_interp_accel_free(accy);
        free(y);
           gsl_spline_free(splz);
        gsl_interp_accel_free(accz);
        free(z);
       free(t);
    p->splx=NULL; p->accx=NULL;p->sply=NULL; p->accy=NULL;p->splz=NULL; p->accz=NULL;
    *val_o = val;

    return 0;
}
