/*
 * =====================================================================================
 *
 *       Filename:  main.cm
 *
 *    Description:  Generate streamlines by integrating orientation field, then
 *                  compute intersections with requested projection surface
 *
 *        Version:  2.0
 *        Created:  03/13/2020 07:30:24 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Sirio Bolaños Puchet (SBP), sirio.bolanospuchet@epfl.ch
 *   Organization:  EPFL Blue Brain Project
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <assert.h>
#include <ctype.h>

#include <zlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "interp_field.h"
#include "streamline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>

#ifndef info0
#define info0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[39m","flatpath","INFO",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","INFO",__func__ )))
#endif
#ifndef info
#define info(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[39m","flatpath","INFO",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","INFO",__func__ ,__VA_ARGS__)))
#endif
#ifndef verbose0
#define verbose0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;12m","flatpath","VERB",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","VERB",__func__ )))
#endif
#ifndef verbose
#define verbose(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;12m","flatpath","VERB",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","VERB",__func__ ,__VA_ARGS__)))
#endif
#ifndef warn0
#define warn0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;11m","flatpath","WARN",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","WARN",__func__ )))
#endif
#ifndef warn
#define warn(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;11m","flatpath","WARN",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","WARN",__func__ ,__VA_ARGS__)))
#endif
#ifndef error0
#define error0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;9m","flatpath","ERROR",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","ERROR",__func__ )))
#endif
#ifndef error
#define error(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;9m","flatpath","ERROR",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","flatpath","ERROR",__func__ ,__VA_ARGS__)))
#endif
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

__attribute__ ((unused))
static inline int strtof_check(const char *str, float out[static 1]) {
    if(NULL == str || '\0' == str[0]) return 1; // invalid input

    errno = 0;
    char *endptr = NULL;
    float val = strtof(str,&endptr);
    if(ERANGE == errno && (val == -HUGE_VALF || val == HUGE_VALF)) return 2; // out of range
    else if(0 != errno && 0.0 == val) return 1; // invalid input
    else if(endptr == str) return 3; // garbage at start
    else if('\0' != *endptr) return 4; // garbage at end

    *out = val;

    return 0; // alles gut
}
__attribute__ ((unused))
static inline int strtod_check(const char *str, double out[static 1]) {
    if(NULL == str || '\0' == str[0]) return 1; // invalid input

    errno = 0;
    char *endptr = NULL;
    double val = strtod(str,&endptr);
    if(ERANGE == errno && (val == -HUGE_VAL || val == HUGE_VAL)) return 2; // out of range
    else if(0 != errno && 0.0 == val) return 1; // invalid input
    else if(endptr == str) return 3; // garbage at start
    else if('\0' != *endptr) return 4; // garbage at end

    *out = val;

    return 0; // alles gut
}
__attribute__ ((unused))
static inline int strtold_check(const char *str, long double out[static 1]) {
    if(NULL == str || '\0' == str[0]) return 1; // invalid input

    errno = 0;
    char *endptr = NULL;
    long double val = strtold(str,&endptr);
    if(ERANGE == errno && (val == -HUGE_VALL || val == HUGE_VALL)) return 2; // out of range
    else if(0 != errno && 0.0 == val) return 1; // invalid input
    else if(endptr == str) return 3; // garbage at start
    else if('\0' != *endptr) return 4; // garbage at end

    *out = val;

    return 0; // alles gut
}
__attribute__ ((unused))
static inline int strtoumax_check(const char *str, uintmax_t out[static 1]) {
    if(NULL == str || '\0' == str[0]) return 1; // invalid input

    for(; '\0' != *str && isspace((unsigned char)*str); ++str); // skip leading spaces
    if('\0' == *str || '-' == *str) return 1; // negative sign or empty, invalid input

    int base = ('0' == str[0] && ('x' == str[1] || 'X' == str[1]))
        ? 16 : 10; // hex or decimal

    errno = 0;
    char *endptr;
    uintmax_t val = strtoumax(str,&endptr,base);
    if(ERANGE == errno && (UINTMAX_MAX == val)) return 2; // out of range
    else if(0 != errno && 0 == val) return 1; // invalid input
    else if(endptr == str) return 3; // garbage at start
    else if('\0' != *endptr) return 4; // garbage at end

    *out = val;

    return 0; // alles gut
}

__attribute__ ((unused))
static inline int strtouint_check(const char *str, unsigned out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT_MAX) return 2; // out of range
    *out = (unsigned)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoul_check(const char *str, unsigned long out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)ULONG_MAX) return 2; // out of range
    *out = (unsigned long)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoull_check(const char *str, unsigned long long out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)ULLONG_MAX) return 2; // out of range
    *out = (unsigned long long)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtosize_check(const char *str, size_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)SIZE_MAX) return 2; // out of range
    *out = (size_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtou8_check(const char *str, uint8_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT8_MAX) return 2; // out of range
    *out = (uint8_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtou16_check(const char *str, uint16_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT16_MAX) return 2; // out of range
    *out = (uint16_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtou32_check(const char *str, uint32_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT32_MAX) return 2; // out of range
    *out = (uint32_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtou64_check(const char *str, uint64_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT64_MAX) return 2; // out of range
    *out = (uint64_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoimax_check(const char *str, intmax_t out[static 1]) {
    if(NULL == str || '\0' == str[0]) return 1; // invalid input

    errno = 0;
    char *endptr;
    intmax_t val = strtoimax(str,&endptr,10);
    if(ERANGE == errno && (INTMAX_MIN == val || INTMAX_MAX == val)) return 2; // out of range
    else if(0 != errno && 0 == val) return 1; // invalid input
    else if(endptr == str) return 3; // garbage at start
    else if('\0' != *endptr) return 4; // garbage at end

    *out = val;

    return 0; // alles gut
}

__attribute__ ((unused))
static inline int strtoint_check(const char *str, int out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LONG_MIN || tmp > (intmax_t)INT_MAX) return 2; // out of range
    *out = (int)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtol_check(const char *str, long out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LONG_MIN || tmp > (intmax_t)LONG_MAX) return 2; // out of range
    *out = (long)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoll_check(const char *str, long long out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LLONG_MIN || tmp > (intmax_t)LLONG_MAX) return 2; // out of range
    *out = (long long)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtossize_check(const char *str, ssize_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)0 || tmp > (intmax_t)SSIZE_MAX) return 2; // out of range
    *out = (ssize_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoi8_check(const char *str, int8_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT8_MIN || tmp > (intmax_t)INT8_MAX) return 2; // out of range
    *out = (int8_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoi16_check(const char *str, int16_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT16_MIN || tmp > (intmax_t)INT16_MAX) return 2; // out of range
    *out = (int16_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoi32_check(const char *str, int32_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT32_MIN || tmp > (intmax_t)INT32_MAX) return 2; // out of range
    *out = (int32_t)tmp;
    return 0;
}
__attribute__ ((unused))
static inline int strtoi64_check(const char *str, int64_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT64_MIN || tmp > (intmax_t)INT64_MAX) return 2; // out of range
    *out = (int64_t)tmp;
    return 0;
}





void print_usage(void) {
    puts("flatpath: project voxels following streamlines");
        
    {
    printf("Available parameters: (* = required) (+ = multiple) [default]\n");

    const int maxlen = 12;
    (void)maxlen;

    
            printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'V',maxlen,"as-voxels",
               "     ",
               "  ",
               "interpret input points as voxel indices",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'B',maxlen,"background",
               "<arg>",
               "  ",
               "background value",
               " [""NAN""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'D',maxlen,"depth",
               "     ",
               "  ",
               "compute depth along full streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'i',maxlen,"dim-x",
               "<arg>",
               "* ",
               "matrix x dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'j',maxlen,"dim-y",
               "<arg>",
               "* ",
               "matrix y dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'k',maxlen,"dim-z",
               "<arg>",
               "* ",
               "matrix z dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'E',maxlen,"epsabs",
               "<arg>",
               "  ",
               "absolute epsilon of ODE solver",
               " [""1E-3""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'1',maxlen,"epsilon-one",
               "<arg>",
               "  ",
               "near-one epsilon",
               " [""1E-2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'0',maxlen,"epsilon-zero",
               "<arg>",
               "  ",
               "near-zero epsilon",
               " [""1E-2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'e',maxlen,"epsrel",
               "<arg>",
               "  ",
               "relative epsilon of ODE solver",
               " [""1E-3""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'f',maxlen,"file",
               "<arg>",
               "  ",
               "input file with streamline initial points",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'l',maxlen,"file-rdepth",
               "<arg>",
               "  ",
               "matrix with relative depth information",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'x',maxlen,"file-x",
               "<arg>",
               "* ",
               "matrix with normals x component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'y',maxlen,"file-y",
               "<arg>",
               "* ",
               "matrix with normals y component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'z',maxlen,"file-z",
               "<arg>",
               "* ",
               "matrix with normals z component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'F',maxlen,"full-only",
               "     ",
               "  ",
               "use full streamlines only",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'H',maxlen,"height",
               "     ",
               "  ",
               "compute thickness along full streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'m',maxlen,"maxiter",
               "<arg>",
               "  ",
               "maximum iterations per streamline",
               " [""4000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'n',maxlen,"nstep",
               "<arg>",
               "  ",
               "number of time steps per integration step",
               " [""2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'N',maxlen,"nstep-path",
               "<arg>",
               "  ",
               "number of steps for height/depth integration",
               " [""1000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'a',maxlen,"offset-x",
               "<arg>",
               "* ",
               "x coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'b',maxlen,"offset-y",
               "<arg>",
               "* ",
               "y coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'c',maxlen,"offset-z",
               "<arg>",
               "* ",
               "z coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'o',maxlen,"output",
               "<arg>",
               "  ",
               "output file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'O',maxlen,"output-dir",
               "<arg>",
               "  ",
               "output directory",
               " [""streamlines""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'p',maxlen,"print",
               "     ",
               "  ",
               "print streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'L',maxlen,"proj-rdepth",
               "<arg>",
               "  ",
               "relative depth of projection surface",
               " [""0.5""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'s',maxlen,"proj-side",
               "<arg>",
               "  ",
               "side of projection surface (bottom / top)",
               " [""TOP""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'r',maxlen,"reverse",
               "     ",
               "  ",
               "reverse normals",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'h',maxlen,"tstep",
               "<arg>",
               "  ",
               "integration time step",
               " [""0.5""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'U',maxlen,"uncompressed",
               "     ",
               "  ",
               "uncompressed input files",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'u',maxlen,"voxel-x",
               "<arg>",
               "* ",
               "voxel x dimension",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'v',maxlen,"voxel-y",
               "<arg>",
               "* ",
               "voxel y dimension",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'w',maxlen,"voxel-z",
               "<arg>",
               "* ",
               "voxel z dimension",
               " [""0.0""]");
   
    
                printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"bin"
            ,"     "
            ,"  ","output binary streamlines","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"first"
            ,"<arg>"
            ,"  ","first voxel to process","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"help"
            ,"     "
            ,"  ","print usage information","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"last"
            ,"<arg>"
            ,"  ","last voxel to process","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"randomize"
            ,"     "
            ,"  ","randomize voxel indices","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seed"
            ,"<arg>"
            ,"  ","set random seed","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"usage"
            ,"     "
            ,"  ","alias for --help","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"verbose"
            ,"     "
            ,"  ","print diagnostic messages","");
   }
}



int main(int argc, char *argv[]) {
    gsl_set_error_handler_off(); // let us handle GSL errors

    size_t first = 0;
    size_t last = 0;
    bool save_binary = false;
    bool do_randomize = false;
    unsigned long long seed = 0xBB5; // initial random seed, only used for shuffling

    /* BEGIN option parsing */
            
        struct {
            double eps_zero; 
           double eps_one; 
           double off_x; 
           double off_y; 
           double off_z; 
           double background; 
           bool do_depth; 
           double epsrel; 
           double epsabs; 
           char* file; 
           bool full_only; 
           double tstep; 
           bool do_height; 
           size_t dim_x; 
           size_t dim_y; 
           size_t dim_z; 
           char* file_rdepth; 
           double proj_rdepth; 
           size_t maxiter; 
           size_t nstep; 
           size_t nstep_path; 
           bool do_print; 
           char* outfile; 
           char* outdir; 
           bool reverse; 
           enum proj_side proj_side; 
           bool uncompr; 
           bool asvox; 
           double vox_x; 
           double vox_y; 
           double vox_z; 
           char* file_x; 
           char* file_y; 
           char* file_z; 
               bool eps_zero_isset; 
           bool eps_one_isset; 
           bool off_x_isset; 
           bool off_y_isset; 
           bool off_z_isset; 
           bool background_isset; 
           bool do_depth_isset; 
           bool epsrel_isset; 
           bool epsabs_isset; 
           bool file_isset; 
           bool full_only_isset; 
           bool tstep_isset; 
           bool do_height_isset; 
           bool dim_x_isset; 
           bool dim_y_isset; 
           bool dim_z_isset; 
           bool file_rdepth_isset; 
           bool proj_rdepth_isset; 
           bool maxiter_isset; 
           bool nstep_isset; 
           bool nstep_path_isset; 
           bool do_print_isset; 
           bool outfile_isset; 
           bool outdir_isset; 
           bool reverse_isset; 
           bool proj_side_isset; 
           bool uncompr_isset; 
           bool asvox_isset; 
           bool vox_x_isset; 
           bool vox_y_isset; 
           bool vox_z_isset; 
           bool file_x_isset; 
           bool file_y_isset; 
           bool file_z_isset; 
       } opts = { 0 };
    (void)opts;
   

    { // set option default values
            opts.eps_zero = 1E-2;
           opts.eps_one = 1E-2;
           opts.off_x = 0.0;
           opts.off_y = 0.0;
           opts.off_z = 0.0;
           opts.background = NAN;
           opts.do_depth = false;
           opts.epsrel = 1E-3;
           opts.epsabs = 1E-3;
               opts.file = NULL;
    
           opts.full_only = false;
           opts.tstep = 0.5;
           opts.do_height = false;
           opts.dim_x = 0;
           opts.dim_y = 0;
           opts.dim_z = 0;
               opts.file_rdepth = NULL;
    
           opts.proj_rdepth = 0.5;
           opts.maxiter = 4000;
           opts.nstep = 2;
           opts.nstep_path = 1000;
           opts.do_print = false;
               opts.outfile = NULL;
    
               opts.outdir = NULL;
        if(       NULL == ((opts.outdir) = strdup("streamlines")))        { error0("out of memory"); exit(EXIT_FAILURE); }

           opts.reverse = false;
           opts.proj_side = TOP;
           opts.uncompr = false;
           opts.asvox = false;
           opts.vox_x = 0.0;
           opts.vox_y = 0.0;
           opts.vox_z = 0.0;
               opts.file_x = NULL;
    
               opts.file_y = NULL;
    
               opts.file_z = NULL;
    
       }
        opterr = 1;
    
    
    
                                                                                                                                                                                                                                                                                                                                                                                                                                                     
    if(argc == 1) { // no arguments at all
                
    {
    printf("Available parameters: (* = required) (+ = multiple) [default]\n");

    const int maxlen = 12;
    (void)maxlen;

    
            printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'V',maxlen,"as-voxels",
               "     ",
               "  ",
               "interpret input points as voxel indices",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'B',maxlen,"background",
               "<arg>",
               "  ",
               "background value",
               " [""NAN""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'D',maxlen,"depth",
               "     ",
               "  ",
               "compute depth along full streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'i',maxlen,"dim-x",
               "<arg>",
               "* ",
               "matrix x dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'j',maxlen,"dim-y",
               "<arg>",
               "* ",
               "matrix y dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'k',maxlen,"dim-z",
               "<arg>",
               "* ",
               "matrix z dimension",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'E',maxlen,"epsabs",
               "<arg>",
               "  ",
               "absolute epsilon of ODE solver",
               " [""1E-3""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'1',maxlen,"epsilon-one",
               "<arg>",
               "  ",
               "near-one epsilon",
               " [""1E-2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'0',maxlen,"epsilon-zero",
               "<arg>",
               "  ",
               "near-zero epsilon",
               " [""1E-2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'e',maxlen,"epsrel",
               "<arg>",
               "  ",
               "relative epsilon of ODE solver",
               " [""1E-3""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'f',maxlen,"file",
               "<arg>",
               "  ",
               "input file with streamline initial points",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'l',maxlen,"file-rdepth",
               "<arg>",
               "  ",
               "matrix with relative depth information",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'x',maxlen,"file-x",
               "<arg>",
               "* ",
               "matrix with normals x component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'y',maxlen,"file-y",
               "<arg>",
               "* ",
               "matrix with normals y component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'z',maxlen,"file-z",
               "<arg>",
               "* ",
               "matrix with normals z component",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'F',maxlen,"full-only",
               "     ",
               "  ",
               "use full streamlines only",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'H',maxlen,"height",
               "     ",
               "  ",
               "compute thickness along full streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'m',maxlen,"maxiter",
               "<arg>",
               "  ",
               "maximum iterations per streamline",
               " [""4000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'n',maxlen,"nstep",
               "<arg>",
               "  ",
               "number of time steps per integration step",
               " [""2""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'N',maxlen,"nstep-path",
               "<arg>",
               "  ",
               "number of steps for height/depth integration",
               " [""1000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'a',maxlen,"offset-x",
               "<arg>",
               "* ",
               "x coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'b',maxlen,"offset-y",
               "<arg>",
               "* ",
               "y coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'c',maxlen,"offset-z",
               "<arg>",
               "* ",
               "z coordinate offset",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'o',maxlen,"output",
               "<arg>",
               "  ",
               "output file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'O',maxlen,"output-dir",
               "<arg>",
               "  ",
               "output directory",
               " [""streamlines""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'p',maxlen,"print",
               "     ",
               "  ",
               "print streamlines",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'L',maxlen,"proj-rdepth",
               "<arg>",
               "  ",
               "relative depth of projection surface",
               " [""0.5""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'s',maxlen,"proj-side",
               "<arg>",
               "  ",
               "side of projection surface (bottom / top)",
               " [""TOP""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'r',maxlen,"reverse",
               "     ",
               "  ",
               "reverse normals",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'h',maxlen,"tstep",
               "<arg>",
               "  ",
               "integration time step",
               " [""0.5""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'U',maxlen,"uncompressed",
               "     ",
               "  ",
               "uncompressed input files",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'u',maxlen,"voxel-x",
               "<arg>",
               "* ",
               "voxel x dimension",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'v',maxlen,"voxel-y",
               "<arg>",
               "* ",
               "voxel y dimension",
               " [""0.0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'w',maxlen,"voxel-z",
               "<arg>",
               "* ",
               "voxel z dimension",
               " [""0.0""]");
   
    
                printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"bin"
            ,"     "
            ,"  ","output binary streamlines","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"first"
            ,"<arg>"
            ,"  ","first voxel to process","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"help"
            ,"     "
            ,"  ","print usage information","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"last"
            ,"<arg>"
            ,"  ","last voxel to process","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"randomize"
            ,"     "
            ,"  ","randomize voxel indices","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seed"
            ,"<arg>"
            ,"  ","set random seed","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"usage"
            ,"     "
            ,"  ","alias for --help","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"verbose"
            ,"     "
            ,"  ","print diagnostic messages","");
   }
    exit(EXIT_SUCCESS);
    }
    else { // process option tables
        char *old_argv0 = argv[0];
        

        const char *optlabel = NULL; 
        (void)optlabel;

        struct option optlist[] = {
                    { "help", no_argument, NULL, 0 }, // print usage information
                   { "usage", no_argument, NULL, 0 }, // alias for --help
                   { "verbose", no_argument, NULL, 0 }, // print diagnostic messages
                           /* first voxel to process */
            { "first", required_argument,                NULL,                0 },
                   /* last voxel to process */
            { "last", required_argument,                NULL,                0 },
                   /* output binary streamlines */
            { "bin", no_argument,                NULL,                0 },
                   /* randomize voxel indices */
            { "randomize", no_argument,                NULL,                0 },
                   /* set random seed */
            { "seed", required_argument,                NULL,                0 },
                           /* near-zero epsilon */
            { "epsilon-zero", required_argument, NULL, '0' },
                   /* near-one epsilon */
            { "epsilon-one", required_argument, NULL, '1' },
                   /* x coordinate offset */
            { "offset-x", required_argument, NULL, 'a' },
                   /* y coordinate offset */
            { "offset-y", required_argument, NULL, 'b' },
                   /* z coordinate offset */
            { "offset-z", required_argument, NULL, 'c' },
                   /* background value */
            { "background", required_argument, NULL, 'B' },
                   /* compute depth along full streamlines */
            { "depth", no_argument, NULL, 'D' },
                   /* relative epsilon of ODE solver */
            { "epsrel", required_argument, NULL, 'e' },
                   /* absolute epsilon of ODE solver */
            { "epsabs", required_argument, NULL, 'E' },
                   /* input file with streamline initial points */
            { "file", required_argument, NULL, 'f' },
                   /* use full streamlines only */
            { "full-only", no_argument, NULL, 'F' },
                   /* integration time step */
            { "tstep", required_argument, NULL, 'h' },
                   /* compute thickness along full streamlines */
            { "height", no_argument, NULL, 'H' },
                   /* matrix x dimension */
            { "dim-x", required_argument, NULL, 'i' },
                   /* matrix y dimension */
            { "dim-y", required_argument, NULL, 'j' },
                   /* matrix z dimension */
            { "dim-z", required_argument, NULL, 'k' },
                   /* matrix with relative depth information */
            { "file-rdepth", required_argument, NULL, 'l' },
                   /* relative depth of projection surface */
            { "proj-rdepth", required_argument, NULL, 'L' },
                   /* maximum iterations per streamline */
            { "maxiter", required_argument, NULL, 'm' },
                   /* number of time steps per integration step */
            { "nstep", required_argument, NULL, 'n' },
                   /* number of steps for height/depth integration */
            { "nstep-path", required_argument, NULL, 'N' },
                   /* print streamlines */
            { "print", no_argument, NULL, 'p' },
                   /* output file */
            { "output", required_argument, NULL, 'o' },
                   /* output directory */
            { "output-dir", required_argument, NULL, 'O' },
                   /* reverse normals */
            { "reverse", no_argument, NULL, 'r' },
                   /* side of projection surface (bottom / top) */
            { "proj-side", required_argument, NULL, 's' },
                   /* uncompressed input files */
            { "uncompressed", no_argument, NULL, 'U' },
                   /* interpret input points as voxel indices */
            { "as-voxels", no_argument, NULL, 'V' },
                   /* voxel x dimension */
            { "voxel-x", required_argument, NULL, 'u' },
                   /* voxel y dimension */
            { "voxel-y", required_argument, NULL, 'v' },
                   /* voxel z dimension */
            { "voxel-z", required_argument, NULL, 'w' },
                   /* matrix with normals x component */
            { "file-x", required_argument, NULL, 'x' },
                   /* matrix with normals y component */
            { "file-y", required_argument, NULL, 'y' },
                   /* matrix with normals z component */
            { "file-z", required_argument, NULL, 'z' },
                   { NULL, 0, NULL, 0 }
        };

        /* build optstring */
        const char optstring[] = "0:1:a:b:c:B:De:E:f:Fh:Hi:j:k:l:L:m:n:N:po:O:rs:UVu:v:w:x:y:z:";

        int opt, longidx;
        bool getopt_verbose = false; // --verbose passed
        bool longopt_isset[8] = { 0 };
        char *longopt_arg[8] = { 0 };
        (void)longopt_isset;
(void)longopt_arg;

        while((opt = getopt_long(argc,argv,optstring,optlist,&longidx)) != -1) {
            if(opt == 0) { /* long options with no short option */
                if(longidx < 8)
                    longopt_isset[longidx] = true;
                if(longidx == 0 || longidx == 1) { // --help or --usage
                        print_usage();
                } else if(longidx == 2) { // --verbose
                    getopt_verbose = true;
                } else if(longidx >= 3) {
                    if(longidx < 8) {
                            if(NULL != (optarg) &&       NULL == ((longopt_arg[longidx]) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    }

                    longidx -= 3; // subtract default

                    /* set optlabel for each long option */
                    switch(longidx) {
                                            case 0:
                            optlabel = "--first";
                            break;
                                           case 1:
                            optlabel = "--last";
                            break;
                                           case 2:
                            optlabel = "--bin";
                            break;
                                           case 3:
                            optlabel = "--randomize";
                            break;
                                           case 4:
                            optlabel = "--seed";
                            break;
                                       }

                    /* user-defined actions */
                        switch(longidx) {
        case 0: // --first
                if(optarg && strtosize_check(optarg,&first)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
            break;
        case 1: // --last
                if(optarg && strtosize_check(optarg,&last)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
            break;
        case 2: // --bin
            save_binary = true;
            break;
        case 3: // --randomize
            do_randomize = true;
            break;
        case 4: // --seed
                if(optarg && strtoull_check(optarg,&seed)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
            break;
    }
                }
            }
            else if('?' == opt || ':' == opt) {
                if(optopt == 0) {
                    size_t oplen = 0; // length until '='
                    const char *optxt = argv[optind - 1] + 2; // start after '--'
                    for(const char *c = optxt; '\0' != *c && *c != '='; ++c, ++oplen);
                    char opbuf[oplen + 1]; (void)opbuf;
                    for(size_t i = 0; i < oplen; ++i) opbuf[i] = optxt[i];
                    opbuf[oplen] = '\0';
                    
                }
                exit(EXIT_FAILURE);
            } else { /* long options with short option */
                
                switch(opt) {
                                    case '0':
                        optlabel = "-0/--epsilon-zero";

                            if(opts.eps_zero_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.eps_zero_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.eps_zero)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case '1':
                        optlabel = "-1/--epsilon-one";

                            if(opts.eps_one_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.eps_one_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.eps_one)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'a':
                        optlabel = "-a/--offset-x";

                            if(opts.off_x_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.off_x_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.off_x)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'b':
                        optlabel = "-b/--offset-y";

                            if(opts.off_y_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.off_y_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.off_y)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'c':
                        optlabel = "-c/--offset-z";

                            if(opts.off_z_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.off_z_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.off_z)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'B':
                        optlabel = "-B/--background";

                            if(opts.background_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.background_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.background)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'D':
                        optlabel = "-D/--depth";

                            if(opts.do_depth_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.do_depth_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.do_depth = !false;
                    break;
                                   case 'e':
                        optlabel = "-e/--epsrel";

                            if(opts.epsrel_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.epsrel_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.epsrel)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'E':
                        optlabel = "-E/--epsabs";

                            if(opts.epsabs_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.epsabs_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.epsabs)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'f':
                        optlabel = "-f/--file";

                            if(opts.file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . file); (opts . file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'F':
                        optlabel = "-F/--full-only";

                            if(opts.full_only_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.full_only_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.full_only = !false;
                    break;
                                   case 'h':
                        optlabel = "-h/--tstep";

                            if(opts.tstep_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.tstep_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.tstep)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'H':
                        optlabel = "-H/--height";

                            if(opts.do_height_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.do_height_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.do_height = !false;
                    break;
                                   case 'i':
                        optlabel = "-i/--dim-x";

                            if(opts.dim_x_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.dim_x_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.dim_x)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'j':
                        optlabel = "-j/--dim-y";

                            if(opts.dim_y_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.dim_y_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.dim_y)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'k':
                        optlabel = "-k/--dim-z";

                            if(opts.dim_z_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.dim_z_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.dim_z)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'l':
                        optlabel = "-l/--file-rdepth";

                            if(opts.file_rdepth_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.file_rdepth_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . file_rdepth); (opts . file_rdepth) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.file_rdepth) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'L':
                        optlabel = "-L/--proj-rdepth";

                            if(opts.proj_rdepth_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.proj_rdepth_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.proj_rdepth)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'm':
                        optlabel = "-m/--maxiter";

                            if(opts.maxiter_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.maxiter_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.maxiter)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'n':
                        optlabel = "-n/--nstep";

                            if(opts.nstep_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nstep_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.nstep)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'N':
                        optlabel = "-N/--nstep-path";

                            if(opts.nstep_path_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nstep_path_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.nstep_path)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'p':
                        optlabel = "-p/--print";

                            if(opts.do_print_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.do_print_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.do_print = !false;
                    break;
                                   case 'o':
                        optlabel = "-o/--output";

                            if(opts.outfile_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.outfile_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . outfile); (opts . outfile) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.outfile) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'O':
                        optlabel = "-O/--output-dir";

                            if(opts.outdir_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.outdir_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . outdir); (opts . outdir) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.outdir) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'r':
                        optlabel = "-r/--reverse";

                            if(opts.reverse_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.reverse_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.reverse = !false;
                    break;
                                   case 's':
                        optlabel = "-s/--proj-side";

                            if(opts.proj_side_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.proj_side_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                                            break;
                                   case 'U':
                        optlabel = "-U/--uncompressed";

                            if(opts.uncompr_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.uncompr_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.uncompr = !false;
                    break;
                                   case 'V':
                        optlabel = "-V/--as-voxels";

                            if(opts.asvox_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.asvox_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.asvox = !false;
                    break;
                                   case 'u':
                        optlabel = "-u/--voxel-x";

                            if(opts.vox_x_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.vox_x_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.vox_x)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'v':
                        optlabel = "-v/--voxel-y";

                            if(opts.vox_y_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.vox_y_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.vox_y)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'w':
                        optlabel = "-w/--voxel-z";

                            if(opts.vox_z_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.vox_z_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.vox_z)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'x':
                        optlabel = "-x/--file-x";

                            if(opts.file_x_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.file_x_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . file_x); (opts . file_x) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.file_x) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'y':
                        optlabel = "-y/--file-y";

                            if(opts.file_y_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.file_y_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . file_y); (opts . file_y) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.file_y) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'z':
                        optlabel = "-z/--file-z";

                            if(opts.file_z_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.file_z_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . file_z); (opts . file_z) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.file_z) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                               }

                /* user-defined actions */
                    switch(opt) {
        case 'D': opts.full_only = true; break;
        case 'e':     if(! (opts.epsrel > 0.0) ) {
        error0("Condition not fulfilled: opts.epsrel > 0.0");
        exit(EXIT_FAILURE);
    }
 break;
        case 'E':     if(! (opts.epsabs > 0.0) ) {
        error0("Condition not fulfilled: opts.epsabs > 0.0");
        exit(EXIT_FAILURE);
    }
 break;
        case 'h':     if(! (opts.tstep > 0.0) ) {
        error0("Condition not fulfilled: opts.tstep > 0.0");
        exit(EXIT_FAILURE);
    }
 break;
        case 'H': opts.full_only = true; break;
        case 's':
            if(strncasecmp(optarg,"bottom",3) == 0) opts.proj_side = BOTTOM;
            else if(strncasecmp(optarg,"top",3) == 0) opts.proj_side = TOP;
            else {
                error("Please specify %s: \"top\" or \"bottom\"",optlabel);
                return 1;
            }
            break;
    }
            }
        }

        /* check if required options are present */
                                                              if(!opts.off_x_isset) {
        error("Please specify %s: %s","-a/--offset-x","x coordinate offset");
        exit(EXIT_FAILURE);
    }
                       if(!opts.off_y_isset) {
        error("Please specify %s: %s","-b/--offset-y","y coordinate offset");
        exit(EXIT_FAILURE);
    }
                       if(!opts.off_z_isset) {
        error("Please specify %s: %s","-c/--offset-z","z coordinate offset");
        exit(EXIT_FAILURE);
    }
                                                                                                                                                                               if(!opts.dim_x_isset) {
        error("Please specify %s: %s","-i/--dim-x","matrix x dimension");
        exit(EXIT_FAILURE);
    }
                       if(!opts.dim_y_isset) {
        error("Please specify %s: %s","-j/--dim-y","matrix y dimension");
        exit(EXIT_FAILURE);
    }
                       if(!opts.dim_z_isset) {
        error("Please specify %s: %s","-k/--dim-z","matrix z dimension");
        exit(EXIT_FAILURE);
    }
                                                                                                                                                                                                                                                           if(!opts.vox_x_isset) {
        error("Please specify %s: %s","-u/--voxel-x","voxel x dimension");
        exit(EXIT_FAILURE);
    }
                       if(!opts.vox_y_isset) {
        error("Please specify %s: %s","-v/--voxel-y","voxel y dimension");
        exit(EXIT_FAILURE);
    }
                       if(!opts.vox_z_isset) {
        error("Please specify %s: %s","-w/--voxel-z","voxel z dimension");
        exit(EXIT_FAILURE);
    }
                       if(!opts.file_x_isset) {
        error("Please specify %s: %s","-x/--file-x","matrix with normals x component");
        exit(EXIT_FAILURE);
    }
                       if(!opts.file_y_isset) {
        error("Please specify %s: %s","-y/--file-y","matrix with normals y component");
        exit(EXIT_FAILURE);
    }
                       if(!opts.file_z_isset) {
        error("Please specify %s: %s","-z/--file-z","matrix with normals z component");
        exit(EXIT_FAILURE);
    }
       
        /* if verbose, print all option values */
        if(getopt_verbose) {
                    
    {
    verbose0("Parameter summary: --flag <bool> = value .");

    const int maxlen = 12;
    (void)maxlen;

    
        {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.asvox ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'V',maxlen,"as-voxels"
                ,opts.asvox_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.background,opts.background);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'B',maxlen,"background"
                ,opts.background_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.do_depth ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'D',maxlen,"depth"
                ,opts.do_depth_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.dim_x);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'i',maxlen,"dim-x"
                ,opts.dim_x_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.dim_y);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'j',maxlen,"dim-y"
                ,opts.dim_y_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.dim_z);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'k',maxlen,"dim-z"
                ,opts.dim_z_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.epsabs,opts.epsabs);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'E',maxlen,"epsabs"
                ,opts.epsabs_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.eps_one,opts.eps_one);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'1',maxlen,"epsilon-one"
                ,opts.eps_one_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.eps_zero,opts.eps_zero);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'0',maxlen,"epsilon-zero"
                ,opts.eps_zero_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.epsrel,opts.epsrel);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'e',maxlen,"epsrel"
                ,opts.epsrel_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'f',maxlen,"file"
                ,opts.file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.file_rdepth) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.file_rdepth) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'l',maxlen,"file-rdepth"
                ,opts.file_rdepth_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.file_x) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.file_x) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'x',maxlen,"file-x"
                ,opts.file_x_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.file_y) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.file_y) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'y',maxlen,"file-y"
                ,opts.file_y_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.file_z) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.file_z) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'z',maxlen,"file-z"
                ,opts.file_z_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.full_only ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'F',maxlen,"full-only"
                ,opts.full_only_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.do_height ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'H',maxlen,"height"
                ,opts.do_height_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.maxiter);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'m',maxlen,"maxiter"
                ,opts.maxiter_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.nstep);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'n',maxlen,"nstep"
                ,opts.nstep_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.nstep_path);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'N',maxlen,"nstep-path"
                ,opts.nstep_path_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.off_x,opts.off_x);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'a',maxlen,"offset-x"
                ,opts.off_x_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.off_y,opts.off_y);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'b',maxlen,"offset-y"
                ,opts.off_y_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.off_z,opts.off_z);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'c',maxlen,"offset-z"
                ,opts.off_z_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.outfile) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.outfile) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'o',maxlen,"output"
                ,opts.outfile_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.outdir) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.outdir) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'O',maxlen,"output-dir"
                ,opts.outdir_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.do_print ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'p',maxlen,"print"
                ,opts.do_print_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.proj_rdepth,opts.proj_rdepth);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'L',maxlen,"proj-rdepth"
                ,opts.proj_rdepth_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
                verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'s',maxlen,"proj-side"
                ,opts.proj_side_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.reverse ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'r',maxlen,"reverse"
                ,opts.reverse_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.tstep,opts.tstep);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'h',maxlen,"tstep"
                ,opts.tstep_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.uncompr ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'U',maxlen,"uncompressed"
                ,opts.uncompr_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.vox_x,opts.vox_x);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'u',maxlen,"voxel-x"
                ,opts.vox_x_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.vox_y,opts.vox_y);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'v',maxlen,"voxel-y"
                ,opts.vox_y_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.vox_z,opts.vox_z);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'w',maxlen,"voxel-z"
                ,opts.vox_z_isset ? "  <set>" : "<unset>","",buf," .");
    }
   
    
                verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"bin"
            ,longopt_isset[5] ? "  <set>" : "<unset>","","","");
               if(longopt_isset[3]) {
        verbose("%s    --%-*s %s%s " "=" " %s%s"
                ,"      ",maxlen,"first","  <set>","",longopt_arg[3],"");
    } else {
        verbose("%s    --%-*s %s%s " "" " %s%s"
                ,"      ",maxlen,"first","<unset>","","","");
    }
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"help"
            ,longopt_isset[0] ? "  <set>" : "<unset>","","","");
               if(longopt_isset[4]) {
        verbose("%s    --%-*s %s%s " "=" " %s%s"
                ,"      ",maxlen,"last","  <set>","",longopt_arg[4],"");
    } else {
        verbose("%s    --%-*s %s%s " "" " %s%s"
                ,"      ",maxlen,"last","<unset>","","","");
    }
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"randomize"
            ,longopt_isset[6] ? "  <set>" : "<unset>","","","");
               if(longopt_isset[7]) {
        verbose("%s    --%-*s %s%s " "=" " %s%s"
                ,"      ",maxlen,"seed","  <set>","",longopt_arg[7],"");
    } else {
        verbose("%s    --%-*s %s%s " "" " %s%s"
                ,"      ",maxlen,"seed","<unset>","","","");
    }
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"usage"
            ,longopt_isset[1] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"verbose"
            ,longopt_isset[2] ? "  <set>" : "<unset>","","","");
   }
        }

        const size_t len_optab_long = 8;
        {
const size_t _len = len_optab_long;
for(size_t _idx = 0; _idx < _len; ++_idx) {
const size_t _0 = _idx;
const size_t _idx = 0; (void)_idx;
{ free(( ( ( longopt_arg ) ) [ _0 ] )); (( ( ( longopt_arg ) ) [ _0 ] )) = NULL; }}
}

        argv[0] = old_argv0;
    }

    if(opts.do_height_isset && opts.do_depth_isset) {
        error("Please specify only one of %s and %s",
              "-H/--height",
              "-D/--depth");
        return 1;
    }
    /* END option parsing */

    size_t dim[3] = { opts.dim_x, opts.dim_y, opts.dim_z };
    size_t dim01 = (size_t)dim[0] * dim[1];
    size_t dim012 = (size_t)dim[0] * dim[1] * dim[2];

            float *L = malloc(dim012 * sizeof(float)); assert(L != NULL);
           float *X = malloc(dim012 * sizeof(float)); assert(X != NULL);
           float *Y = malloc(dim012 * sizeof(float)); assert(Y != NULL);
           float *Z = malloc(dim012 * sizeof(float)); assert(Z != NULL);
   
#define NFILES 4
    { // load files in parallel
        const char *src[NFILES] = { opts.file_rdepth, opts.file_x, opts.file_y, opts.file_z };
        float *dst[NFILES] = { L, X, Y, Z };
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for(size_t i = 0; i < NFILES; ++i) {
            if(opts.uncompr) {
                FILE *fp = fopen(src[i],"rb"); assert(fp != NULL);
                size_t nread = fread(dst[i],sizeof(float),dim012,fp); assert(nread == dim012);
                fclose(fp);
            } else {
                gzFile zfp = gzopen(src[i],"rb"); assert(zfp != NULL);
                size_t nread = gzfread(dst[i],sizeof(float),dim012,zfp); assert(nread == dim012);
                gzclose(zfp);
            }
        }
    }
    info0("Successfully loaded matrices");

    /* make normal vectors unit norm */
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for(size_t ix = 0; ix < dim012; ix++) {
        const double norm = gsl_hypot3(X[ix],Y[ix],Z[ix]);
        const double norm_checked = norm > NORM_EPSILON ? norm : 1.0;
        X[ix] /= norm_checked;Y[ix] /= norm_checked;Z[ix] /= norm_checked;    }

    /* optionally reverse normals */
    if(opts.reverse)
        for(size_t ix = 0; ix < dim012; ix++) { X[ix] *= -1.0;Y[ix] *= -1.0;Z[ix] *= -1.0; }

    /* setup streamline initial positions */
    size_t npoint = 0;
    double *points = NULL; // Cartesian coordinates (um)
    size_t *ixvox = NULL; // voxel indices

    if(opts.file_isset) { // get points from file
        bool is_stdin = (strcmp(opts.file,"-") == 0);
        FILE *fp = is_stdin ? stdin : fopen(opts.file,"r");
        assert(fp != NULL);
        size_t n = 0;
        char *line = NULL;
        while(getline(&line,&n,fp) != -1) {
            if(opts.asvox) {
                unsigned long j,i,k;
                if(sscanf(line,"%lu %lu %lu",&j,&i,&k) == 3) {
                    ++npoint;
                    double yinit[3];
                                        yinit[0] = opts.off_x + opts.vox_x * (j + 0.5);
                                       yinit[1] = opts.off_y + opts.vox_y * (i + 0.5);
                                       yinit[2] = opts.off_z + opts.vox_z * (k + 0.5);
                                       points = realloc(points,(npoint * 3) * sizeof(*points));
                    ixvox = realloc(ixvox,(npoint * 3) * sizeof(*ixvox));
                    memcpy(&points[3 * (npoint - 1)],yinit,3 * sizeof(*points));
                    ixvox[npoint - 1] = (size_t)j + i * dim[0] + k * dim01;
                } else { error("expected three voxel indices in line: %s",line); return 1; }
            } else {
                double x,y,z;
                if(sscanf(line,"%lf %lf %lf",&x,&y,&z) == 3) {
                    const double yinit[3] = { x, y, z };
                    points = realloc(points,(++npoint * 3) * sizeof(double));
                    memcpy(&points[3 * (npoint - 1)],yinit,3 * sizeof(double));
                } else { error("expected three coordinates in line: %s",line); return 1; }
            }
        }
        if(fp != stdin) fclose(fp);
        info("Using %zu initial positions from file \"%s\"",npoint,
                is_stdin ? "<stdin>" : opts.file);
    } else { // use voxel centers
        /* count first, in parallel */
#ifdef _OPENMP
#pragma omp parallel for reduction(+:npoint)
#endif
        for(size_t ix = 0; ix < dim012; ++ix) {
            if((isnan(opts.background) && isnan(L[ix])) ||
                    (!isnan(opts.background) && L[ix] == opts.background)) continue;
            npoint++; // layer different from background means we take it
        }
        /* allocate at once */
        points = malloc((size_t)3 * npoint * sizeof(*points)); assert(points != NULL);
        ixvox = malloc(npoint * sizeof(*ixvox)); assert(ixvox != NULL);
        /* collect */
        for(size_t ix = 0, ipt = 0; ix < dim012; ++ix) {
            if((isnan(opts.background) && isnan(L[ix])) ||
                    (!isnan(opts.background) && L[ix] == opts.background)) continue;
            const size_t vx = ix;
            const size_t j = ((vx % dim01) % dim[0]);const size_t i = ((vx % dim01) / dim[0]);const size_t k = (vx / dim01); // unravel

            double *yinit = &points[3 * ipt]; // point location
                        yinit[0] = opts.off_x + opts.vox_x * (j + 0.5);
                       yinit[1] = opts.off_y + opts.vox_y * (i + 0.5);
                       yinit[2] = opts.off_z + opts.vox_z * (k + 0.5);
                       ixvox[ipt] = ix; // save point index
            ++ipt; // increase index
        }
        info("Using %zu voxels centers from mask as initial positions",npoint);
    }

    /* determine which streamlines to process, and in which order */
    size_t *ord = malloc(npoint * sizeof(size_t)); // global voxel index order
    if(NULL == ord) return ENOMEM;

    for(size_t i=0;i<npoint;i++) ord[i] = i; // monotonic increasing order

    if(do_randomize) { /* randomize order of voxel indices */
        const gsl_rng_type *T = gsl_rng_ranlxd2; /* RANLUX double-precision w/luxury level 2 */
        gsl_rng *r = gsl_rng_alloc(T);
        gsl_rng_set(r,seed);
        gsl_ran_shuffle(r,ord,npoint,sizeof(size_t));
        gsl_rng_free(r);
        info("Done shuffling voxel indices with seed %""#llX",seed);
    }

    if(first >= npoint) {
        warn0("First index larger than total points, nothing to do");
        return 1;
    }
    if(last == 0 || last > npoint) last = npoint;
    if(last <= first) {
        error("Please specify --%s < --%s",
              "--first",
              "--last");
        return 1;
    }
    info("Processing points with index %zu to %zu",first,last - 1);

    /* open output file */
    FILE *fpout = stdout;
    if(opts.outfile_isset && !(opts.outfile[0] == '-' && opts.outfile[1] == '\0'))
        fpout = fopen(opts.outfile, "w");
    assert(NULL != fpout);

    /* common parameters */
    struct interp_params p = {
        .x=X, .vx=opts.vox_x, .ox=opts.off_x,.y=Y, .vy=opts.vox_y, .oy=opts.off_y,.z=Z, .vz=opts.vox_z, .oz=opts.off_z,        .rdepth=L, .nx=dim[0], .ny=dim[1], .nz=dim[2],
        .proj_rdepth=opts.proj_rdepth,.proj_side=opts.proj_side,
        .eps_zero=opts.eps_zero,
        .eps_one=opts.eps_one
    };

    /* storage for streamlines */
    size_t *sn = calloc(npoint,sizeof(size_t)); // array lengths
    if(NULL == sn) return ENOMEM;
    double **sx = calloc(npoint,sizeof(double*)); // positions
    if(NULL == sx) return ENOMEM;

    { // compute streamlines
        /* setup ODE system with interpolated direction field */
        gsl_odeiv2_system sys = { dirfield, NULL, 3, &p };

        /* parallelize across initial positions */
#ifdef _OPENMP
#pragma omp parallel
        {
#endif
            gsl_odeiv2_driver *d =
                gsl_odeiv2_driver_alloc_y_new(&sys,gsl_odeiv2_step_rk2,opts.tstep,opts.epsabs,opts.epsrel);

#ifdef _OPENMP
#pragma omp for
#endif
            for(size_t ii=first;ii<last;ii++) {
                const size_t ix = ord[ii];
                const double *const yinit = &points[3 * ix];
#if DEBUG
                if(ixvox != NULL) {
                    const size_t vx = ixvox[ix];
                    const size_t ix_j = ((vx % dim01) % dim[0]);const size_t ix_i = ((vx % dim01) / dim[0]);const size_t ix_k = (vx / dim01);                    info("Initial X %g Y %g Z %g [%zu %zu %zu] -> %g",
                            yinit[0],yinit[1],yinit[2], ix_j,ix_i,ix_k,                            get_rdepth(yinit[0],yinit[1],yinit[2],&p));
                }
#endif

                int ret = compute_streamline(d,yinit,ix,opts.tstep,opts.maxiter,opts.nstep,&p,&sn[ix],&sx[ix]);
                if(ret) warn("Streamline %zu returned %d",ix,ret);
            }

            gsl_odeiv2_driver_free(d);
#ifdef _OPENMP
        }
#endif
    }

    if(opts.do_print) { // output streamlines
        char buf[PATH_MAX];
        snprintf(buf,sizeof(buf),"%s/index.txt",opts.outdir);
        FILE *fp = fopen(buf,"a"); assert(fp != NULL);
        if(save_binary) {
            for(size_t ii=first;ii<last;ii++) {
                const size_t ix = ord[ii];
                char buf[PATH_MAX];
                snprintf(buf,sizeof(buf),"%s/streamline_%08zu.bin",opts.outdir,ix);
                float *bufsave = malloc(3 * sn[ix] * sizeof(float));
                for(size_t i=0;i<3 * sn[ix];i++) bufsave[i] = sx[ix][i]; /* double to float */
                FILE *fpb = fopen(buf,"wb"); assert(fpb != NULL);
                fwrite(bufsave,sizeof(float),3 * sn[ix],fpb);
                free(bufsave);
                fclose(fpb);
                fprintf(fp,"%08zu %zu\n",ix,sn[ix]); // print to index
            }
        } else {
            for(size_t ii=first;ii<last;ii++) {
                const size_t ix = ord[ii];
                char buf[PATH_MAX];
                snprintf(buf,sizeof(buf),"%s/streamline_%08zu.xyz",opts.outdir,ix);
                FILE *fpt = fopen(buf,"w"); assert(fpt != NULL);
                for(size_t i=0;i<sn[ix];i++)
                    fprintf(fpt,"%.8f %.8f %.8f\n",sx[ix][3 * i + 0],sx[ix][3 * i + 1],sx[ix][3 * i + 2]);
                fclose(fpt);
                fprintf(fp,"%08zu %zu\n",ix,sn[ix]); // print to index
            }
        }
        fclose(fp);
    }

    if(opts.do_height || opts.do_depth) { // compute height/depth along streamlines
        info("Computing %s along full streamlines",opts.do_height ? "height" : "depth");

        // NOTE: parallelization would require independent params per thread
        for(size_t ii=first;ii<last;ii++) {
            const size_t ix = ord[ii];

            const double *const yinit = &points[3 * ix];
            size_t ix_j = 0;size_t ix_i = 0;size_t ix_k = 0;            if(ixvox != NULL) {
                const size_t vx = ixvox[ix];
                ix_j = ((vx % dim01) % dim[0]);ix_i = ((vx % dim01) / dim[0]);ix_k = (vx / dim01);            }

            double value = NAN;
            if(sn[ix] >= 2)
                streamline_measure(ix,sn[ix],sx[ix],yinit,&p,opts.do_height,opts.nstep_path,&value);

            if(ixvox != NULL) // use voxel indices
                fprintf(fpout,"%zu %zu %zu %.16g\n",ix_j,ix_i,ix_k,value);
            else // use point index
                fprintf(fpout,"%zu %.16g\n",ix,value);
        }
    }

    else if(opts.proj_rdepth > 0) { // compute intersection with projection surface
        info("Taking projection surface at relative depth %g",opts.proj_rdepth);

        if(opts.full_only) { info0("Using full streamlines only"); }
        else info0("Using all streamlines");

        gsl_root_fsolver *rs = gsl_root_fsolver_alloc(gsl_root_fsolver_brent);

        // NOTE: parallelization would require independent root solvers per thread
        for(size_t ii=first;ii<last;ii++) {
            const size_t ix = ord[ii];

            const double *const yinit = &points[3 * ix];
            size_t ix_j = 0;size_t ix_i = 0;size_t ix_k = 0;            if(ixvox != NULL) {
                const size_t vx = ixvox[ix];
                ix_j = ((vx % dim01) % dim[0]);ix_i = ((vx % dim01) / dim[0]);ix_k = (vx / dim01);            }

            double isec[3] = { NAN, NAN, NAN };
            if(sn[ix] >= 2) streamline_intersection(ix,sn[ix],sx[ix],yinit,&p,opts.full_only,rs,isec);

            if(ixvox != NULL) // use voxel indices
                fprintf(fpout,"%zu %zu %zu %.16g %.16g %.16g\n",ix_j,ix_i,ix_k,
                       isec[0],isec[1],isec[2]);
            else // use point index
                fprintf(fpout,"%zu %.16g %.16g %.16g\n",ix,isec[0],isec[1],isec[2]);
        }

        gsl_root_fsolver_free(rs);
    }

    /* close output file */
    fclose(fpout);

    /* cleanup streamlines */
    for(size_t i=0;i<npoint;i++) free(sx[i]);
    free(sn);
    free(sx);

    /* cleanup points */
    free(points);
    free(ixvox);
    free(ord);

    /* cleanup volumes */
    free(L);free(X);free(Y);free(Z);
    /* cleanup options */
                                                                                                                       { free(opts . file); (opts . file) = NULL; }
                                                                                 { free(opts . file_rdepth); (opts . file_rdepth) = NULL; }
                                                                      { free(opts . outfile); (opts . outfile) = NULL; }
               { free(opts . outdir); (opts . outdir) = NULL; }
                                                                                            { free(opts . file_x); (opts . file_x) = NULL; }
               { free(opts . file_y); (opts . file_y) = NULL; }
               { free(opts . file_z); (opts . file_z) = NULL; }
   }
