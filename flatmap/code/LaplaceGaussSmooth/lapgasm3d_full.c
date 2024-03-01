#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

enum load_mode {
    LOAD_MMAP,
    LOAD_PARALLEL,
    LOAD_SEQUENTIAL
};

enum save_mode {
    SAVE_PARALLEL,
    SAVE_SEQUENTIAL
};

#define UNCOORD_DIM(x,jik,nx,dim) \
    jik[0] = ((x) % (dim)) % (nx); \
    jik[1] = ((x) % (dim)) / (nx); \
    jik[2] =  (x) / (dim);

static volatile sig_atomic_t keepRunning = 1;
static volatile sig_atomic_t nint = 0;

#define TERMINATE_MSG "[lapgasm3d_full]  WARN: interrupt caught, will stop after current iteration (send again for immediate termination)\n"
static void terminate_handler(int sig) {
    if(nint++) {
        (void)signal(sig,SIG_DFL); // reset handler
        (void)raise(sig);          // raise again
    }
    keepRunning = 0;
    write(STDERR_FILENO,TERMINATE_MSG,strlen(TERMINATE_MSG));
}

static void alarm_handler(int sig) {
    (void)sig;
}

#define CREAT_MASK (S_IRUSR | S_IWUSR | S_IRGRP)
#define MAX_RETRY_SAVE 100

#define ERRFATAL(FUN,FMT,...) { error("%s failed: %s (%d): " FMT,FUN,strerror(errno),errno,__VA_ARGS__); exit(EXIT_FAILURE); }
#define PERROR(FUN) ERRFATAL(FUN,"at line %d",__LINE__)
#define FATAL(M,...) { error(M,__VA_ARGS__); exit(EXIT_FAILURE); }
#define FATAL0(M) { error0(M); exit(EXIT_FAILURE); }

#define U16FACTOR (UINT16_MAX - 2)
#define OUT_U16 (U16FACTOR + 1)
#define WM_U16 (U16FACTOR + 2)
#define OUT_FLT (2.0)
#define WM_FLT (-1.0)

#ifndef U16DATA
#define U16DATA 0
#endif

#ifndef FLTDATA
#define FLTDATA 1
#endif

#define INBND(msk) ((msk) > 0)
#define INMASK_LABEL(x) ((x) >= 1 && (x) < NREGION)

#if U16DATA
typedef uint16_t data_t;
#define PRIdata PRIu16
#define TYPESTR "uint16"
#define INMASK(x) ((x) <= U16FACTOR)
#define TOFLOAT(x) ((double)(x) / U16FACTOR)
#define FROMFLOAT(x) ((x) * U16FACTOR)
#define PRINTBND(M) \
    info0(M); \
    for(size_t i = 1; i < 1 + NREGION; ++i) info("    %"PRIdata" (%g)",bnd_vals[i],TOFLOAT(bnd_vals[i]));
#elif FLTDATA
typedef float data_t;
#define PRIdata "g"
#define TYPESTR "float32"
#define INMASK(x) ((x) > WM_FLT && (x) < OUT_FLT)
#define TOFLOAT(x) (x)
#define FROMFLOAT(x) (x)
#define PRINTBND(M) \
    info0(M); \
    for(size_t i = 1; i < 1 + NREGION; ++i) info("    %"PRIdata,bnd_vals[i]);
#endif


#include <stdio.h>

#ifndef info0
#define info0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[39m","lapgasm3d_full","INFO",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","INFO",__func__ )))
#endif
#ifndef info
#define info(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[39m","lapgasm3d_full","INFO",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","INFO",__func__ ,__VA_ARGS__)))
#endif
#ifndef warn0
#define warn0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;11m","lapgasm3d_full","WARN",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","WARN",__func__ )))
#endif
#ifndef warn
#define warn(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;11m","lapgasm3d_full","WARN",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","WARN",__func__ ,__VA_ARGS__)))
#endif
#ifndef error0
#define error0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;9m","lapgasm3d_full","ERROR",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","ERROR",__func__ )))
#endif
#ifndef error
#define error(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;9m","lapgasm3d_full","ERROR",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","ERROR",__func__ ,__VA_ARGS__)))
#endif
#ifndef verbose0
#define verbose0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;12m","lapgasm3d_full","VERB",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","VERB",__func__ )))
#endif
#ifndef verbose
#define verbose(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;12m","lapgasm3d_full","VERB",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","VERB",__func__ ,__VA_ARGS__)))
#endif
#ifndef debug0
#define debug0(M) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;13m","lapgasm3d_full","DEBUG",__func__ )\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","DEBUG",__func__ )))
#endif
#ifndef debug
#define debug(M, ...) ((false) ? 0\
        : ((false)\
            ? fprintf(stderr,"%s[%s] %5s: %s: " M "\x1B[39m" "\n", "\x1B[38;5;13m","lapgasm3d_full","DEBUG",__func__ ,__VA_ARGS__)\
            : fprintf(stderr,"[%s] %5s: %s: " M "\n","lapgasm3d_full","DEBUG",__func__ ,__VA_ARGS__)))
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





static void print_usage(void) {
    puts("lapgasm3d_full: heat equation approximation through iterative gaussian smoothing in 3D (full version)");
        
    {
    printf("Available parameters: (* = required) (+ = multiple) [default]\n");

    const int maxlen = 12;
    (void)maxlen;

    
            printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'B',maxlen,"bound-vals",
               "<arg>",
               "  ",
               "boundary values comma-separated list",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'b',maxlen,"boundaries",
               "<arg>",
               "  ",
               "boundary values file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'F',maxlen,"convert-f32",
               "     ",
               "  ",
               "convert uint16[0,65533] to float[0,1]",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'C',maxlen,"convert-u16",
               "     ",
               "  ",
               "convert float[0,1] to uint16[0,65533]",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'d',maxlen,"delta",
               "<arg>",
               "  ",
               "absolute delta tolerance for convergence",
               " [""1E-9""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'D',maxlen,"delta-bnd",
               "<arg>",
               "  ",
               "relative delta tolerance for boundary update",
               " [""1E-6""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'E',maxlen,"extract-mask",
               "     ",
               "  ",
               "extract mask from labels file (int8)",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'f',maxlen,"force-bnd",
               "     ",
               "  ",
               "force initial boundary values",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'G',maxlen,"get-mask",
               "     ",
               "  ",
               "generate mask from labels file (uint8)",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'i',maxlen,"initial",
               "<arg>",
               "  ",
               "file with initial solution (" TYPESTR ")",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'n',maxlen,"itermax",
               "<arg>",
               "  ",
               "maximum number of iterations",
               " [""1000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'l',maxlen,"labels",
               "<arg>",
               "  ",
               "file with labels ((u)int8)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'m',maxlen,"mask",
               "<arg>",
               "  ",
               "file with mask (uint8)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'M',maxlen,"mask-sizes",
               "<arg>",
               "  ",
               "file with per-slice mask sizes (uint64)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'o',maxlen,"output",
               "<arg>",
               "  ",
               "output file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'r',maxlen,"regions",
               "<arg>",
               "* ",
               "number of regions",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'x',maxlen,"xdim",
               "<arg>",
               "  ",
               "size of x dimension",
               " [""6145""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'y',maxlen,"ydim",
               "<arg>",
               "  ",
               "size of y dimension",
               " [""4310""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'z',maxlen,"zdim",
               "<arg>",
               "  ",
               "size of z dimension",
               " [""7406""]");
   
    
                printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"docs"
            ,"     "
            ,"  ","print documentation","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"help"
            ,"     "
            ,"  ","print usage information","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"mmap"
            ,"     "
            ,"  ","map input file directly from memory (default)","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"parload"
            ,"     "
            ,"  ","load input file from disk in parallel","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"parsave"
            ,"     "
            ,"  ","save output in parallel (default)","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seqload"
            ,"     "
            ,"  ","load input file from disk sequentially","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seqsave"
            ,"     "
            ,"  ","save output sequentially","");
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


static void print_docs(void) {
    puts("lapgasm3d_full: heat equation approximation through iterative gaussian smoothing in 3D (full version)");
    puts("Features:\n"
"   + Low memory footprint: mmap-ed inputs, sparse spatial locations\n"
"   + Fast: RLE of masked values, parallel save\n"
"\n"
"Input formats:\n"
"   + Labels is a uint8 file with 0 for outside, and increasing numbers for layers, up to (and including) WM\n"
"   + Initial solution is a float32 file with 2.0 for outside, -1.0 for WM, and values [0,1] for layers\n"
);
}


__attribute__ ((unused))
  int strtof_check(const char *str, float out[static 1]) {
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
  int strtod_check(const char *str, double out[static 1]) {
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
  int strtold_check(const char *str, long double out[static 1]) {
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
  int strtoumax_check(const char *str, uintmax_t out[static 1]) {
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
  int strtouint_check(const char *str, unsigned out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT_MAX) return 2; // out of range
    *out = (unsigned)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoul_check(const char *str, unsigned long out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)ULONG_MAX) return 2; // out of range
    *out = (unsigned long)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoull_check(const char *str, unsigned long long out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)ULLONG_MAX) return 2; // out of range
    *out = (unsigned long long)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtosize_check(const char *str, size_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)SIZE_MAX) return 2; // out of range
    *out = (size_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtou8_check(const char *str, uint8_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT8_MAX) return 2; // out of range
    *out = (uint8_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtou16_check(const char *str, uint16_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT16_MAX) return 2; // out of range
    *out = (uint16_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtou32_check(const char *str, uint32_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT32_MAX) return 2; // out of range
    *out = (uint32_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtou64_check(const char *str, uint64_t out[static 1]) {
    int ret;
    uintmax_t tmp;
    if((ret = strtoumax_check(str,&tmp))) return ret;
    if(tmp > (uintmax_t)UINT64_MAX) return 2; // out of range
    *out = (uint64_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoimax_check(const char *str, intmax_t out[static 1]) {
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
  int strtoint_check(const char *str, int out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LONG_MIN || tmp > (intmax_t)INT_MAX) return 2; // out of range
    *out = (int)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtol_check(const char *str, long out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LONG_MIN || tmp > (intmax_t)LONG_MAX) return 2; // out of range
    *out = (long)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoll_check(const char *str, long long out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)LLONG_MIN || tmp > (intmax_t)LLONG_MAX) return 2; // out of range
    *out = (long long)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtossize_check(const char *str, ssize_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)0 || tmp > (intmax_t)SSIZE_MAX) return 2; // out of range
    *out = (ssize_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoi8_check(const char *str, int8_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT8_MIN || tmp > (intmax_t)INT8_MAX) return 2; // out of range
    *out = (int8_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoi16_check(const char *str, int16_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT16_MIN || tmp > (intmax_t)INT16_MAX) return 2; // out of range
    *out = (int16_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoi32_check(const char *str, int32_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT32_MIN || tmp > (intmax_t)INT32_MAX) return 2; // out of range
    *out = (int32_t)tmp;
    return 0;
}
__attribute__ ((unused))
  int strtoi64_check(const char *str, int64_t out[static 1]) {
    int ret;
    intmax_t tmp;
    if((ret = strtoimax_check(str,&tmp))) return ret;
    if(tmp < (intmax_t)INT64_MIN || tmp > (intmax_t)INT64_MAX) return 2; // out of range
    *out = (int64_t)tmp;
    return 0;
}

#define NX (dims.nx)
#define NY (dims.ny)
#define NZ (dims.nz)
#define NXY (NX * NY)
#define NVOX (NXY * NZ)
struct _dims {
    size_t nx;
    size_t ny;
    size_t nz;
};

#define NHOOD_NEUMANN_N 7
#define KERNEL_VSIZE NHOOD_NEUMANN_N

/* 3D gaussian kernel (at approx 1.889 std devs) */

/* 3D von Neumann neighborhood */
inline static void get_nhood3d_neumann(size_t ix, const data_t *restrict array, struct _dims dims, data_t buf[restrict static KERNEL_VSIZE]) {
    buf[0] = array[ix + (int64_t)(-NXY)];
    buf[1] = array[ix + (int64_t)(-NX)];
    buf[2] = array[ix + (int64_t)(-1)];
    buf[3] = array[ix + (int64_t)(1)];
    buf[4] = array[ix + (int64_t)(NX)];
    buf[5] = array[ix + (int64_t)(NXY)];
    buf[6] = array[ix + (int64_t)(0)];
}

inline static void get_nhood3d_neumann_u8(size_t ix, const uint8_t *restrict array, struct _dims dims, uint8_t buf[restrict static KERNEL_VSIZE]) {
    buf[0] = array[ix + (int64_t)(-NXY)];
    buf[1] = array[ix + (int64_t)(-NX)];
    buf[2] = array[ix + (int64_t)(-1)];
    buf[3] = array[ix + (int64_t)(1)];
    buf[4] = array[ix + (int64_t)(NX)];
    buf[5] = array[ix + (int64_t)(NXY)];
    buf[6] = array[ix + (int64_t)(0)];
}

static const double kernel[KERNEL_VSIZE] = {
0x1p-3,0x1p-3,0x1p-3,0x1p-3,0x1p-3,0x1p-3,0x1p-2};

inline static double dotproduct(const double a[restrict static KERNEL_VSIZE], const double b[restrict static KERNEL_VSIZE]) {
    double sum = 0.0;
    for(int i = 0; i < KERNEL_VSIZE; ++i)
        sum += a[i] * b[i];
    return sum;
}

inline static double convolve(struct _dims dims, const data_t *restrict res, size_t gix) {
    data_t nhood[KERNEL_VSIZE];
    get_nhood3d_neumann(gix,res,dims,nhood);

    double buf[KERNEL_VSIZE] = { 0 };
    double norm = 0.0;
    /* optimize center voxel */
    buf[KERNEL_VSIZE - 1] = TOFLOAT(nhood[KERNEL_VSIZE - 1]);
    norm = kernel[KERNEL_VSIZE - 1];
    for(int i = 0; i < KERNEL_VSIZE - 1; ++i) { // skip out-of-mask voxels
        if(!INMASK(nhood[i])) continue;
        buf[i] = TOFLOAT(nhood[i]);
        norm += kernel[i];
    }

    double sum = dotproduct(buf,kernel) / norm; // compute convolution (normalized)
    if(sum > 1.0) sum = 1.0; // clamp hi
    else if(sum < 0.0) sum = 0.0; // clamp lo

    return sum;
}






int main(int argc, char *argv[]) {
    enum load_mode loadm = LOAD_MMAP;
    enum save_mode savem = SAVE_PARALLEL;
    int do_print_docs = false;

            
        struct {
            char* bnd_file; 
           char* bnd_vals; 
           bool conv_u16; 
           double delta_tol; 
           double delta_bnd; 
           bool ext_mask; 
           bool force_bnd; 
           bool conv_f32; 
           bool get_mask; 
           char* init_file; 
           char* labels_file; 
           char* mask_file; 
           char* nmask_file; 
           size_t iter_max; 
           char* out_file; 
           uint8_t nregion; 
           size_t nx; 
           size_t ny; 
           size_t nz; 
               bool bnd_file_isset; 
           bool bnd_vals_isset; 
           bool conv_u16_isset; 
           bool delta_tol_isset; 
           bool delta_bnd_isset; 
           bool ext_mask_isset; 
           bool force_bnd_isset; 
           bool conv_f32_isset; 
           bool get_mask_isset; 
           bool init_file_isset; 
           bool labels_file_isset; 
           bool mask_file_isset; 
           bool nmask_file_isset; 
           bool iter_max_isset; 
           bool out_file_isset; 
           bool nregion_isset; 
           bool nx_isset; 
           bool ny_isset; 
           bool nz_isset; 
       } opts = { 0 };
    (void)opts;
   

    { // set option default values
                opts.bnd_file = NULL;
    
               opts.bnd_vals = NULL;
    
           opts.conv_u16 = false;
           opts.delta_tol = 1E-9;
           opts.delta_bnd = 1E-6;
           opts.ext_mask = false;
           opts.force_bnd = false;
           opts.conv_f32 = false;
           opts.get_mask = false;
               opts.init_file = NULL;
    
               opts.labels_file = NULL;
    
               opts.mask_file = NULL;
    
               opts.nmask_file = NULL;
    
           opts.iter_max = 1000;
               opts.out_file = NULL;
    
           opts.nregion = 0;
           opts.nx = 6145;
           opts.ny = 4310;
           opts.nz = 7406;
       }
        opterr = 1;
    
    
    
                                                                                                                                                                                                                                                                                           
    if(argc == 1) { // no arguments at all
                
    {
    printf("Available parameters: (* = required) (+ = multiple) [default]\n");

    const int maxlen = 12;
    (void)maxlen;

    
            printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'B',maxlen,"bound-vals",
               "<arg>",
               "  ",
               "boundary values comma-separated list",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'b',maxlen,"boundaries",
               "<arg>",
               "  ",
               "boundary values file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'F',maxlen,"convert-f32",
               "     ",
               "  ",
               "convert uint16[0,65533] to float[0,1]",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'C',maxlen,"convert-u16",
               "     ",
               "  ",
               "convert float[0,1] to uint16[0,65533]",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'d',maxlen,"delta",
               "<arg>",
               "  ",
               "absolute delta tolerance for convergence",
               " [""1E-9""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'D',maxlen,"delta-bnd",
               "<arg>",
               "  ",
               "relative delta tolerance for boundary update",
               " [""1E-6""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'E',maxlen,"extract-mask",
               "     ",
               "  ",
               "extract mask from labels file (int8)",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'f',maxlen,"force-bnd",
               "     ",
               "  ",
               "force initial boundary values",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'G',maxlen,"get-mask",
               "     ",
               "  ",
               "generate mask from labels file (uint8)",
               " [""false""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'i',maxlen,"initial",
               "<arg>",
               "  ",
               "file with initial solution (" TYPESTR ")",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'n',maxlen,"itermax",
               "<arg>",
               "  ",
               "maximum number of iterations",
               " [""1000""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'l',maxlen,"labels",
               "<arg>",
               "  ",
               "file with labels ((u)int8)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'m',maxlen,"mask",
               "<arg>",
               "  ",
               "file with mask (uint8)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'M',maxlen,"mask-sizes",
               "<arg>",
               "  ",
               "file with per-slice mask sizes (uint64)",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'o',maxlen,"output",
               "<arg>",
               "  ",
               "output file",
               "");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'r',maxlen,"regions",
               "<arg>",
               "* ",
               "number of regions",
               " [""0""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'x',maxlen,"xdim",
               "<arg>",
               "  ",
               "size of x dimension",
               " [""6145""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'y',maxlen,"ydim",
               "<arg>",
               "  ",
               "size of y dimension",
               " [""4310""]");
           printf("%s-%c, --%-*s %s%s " ":" " %s%s" "\n",
               "      ",'z',maxlen,"zdim",
               "<arg>",
               "  ",
               "size of z dimension",
               " [""7406""]");
   
    
                printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"docs"
            ,"     "
            ,"  ","print documentation","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"help"
            ,"     "
            ,"  ","print usage information","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"mmap"
            ,"     "
            ,"  ","map input file directly from memory (default)","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"parload"
            ,"     "
            ,"  ","load input file from disk in parallel","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"parsave"
            ,"     "
            ,"  ","save output in parallel (default)","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seqload"
            ,"     "
            ,"  ","load input file from disk sequentially","");
               printf("%s    --%-*s %s%s " ":" " %s%s" "\n"
            ,"      "
            
            ,maxlen,"seqsave"
            ,"     "
            ,"  ","save output sequentially","");
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
                           /* map input file directly from memory (default) */
            { "mmap", no_argument,                NULL,                0 },
                   /* load input file from disk in parallel */
            { "parload", no_argument,                NULL,                0 },
                   /* load input file from disk sequentially */
            { "seqload", no_argument,                NULL,                0 },
                   /* save output in parallel (default) */
            { "parsave", no_argument,                NULL,                0 },
                   /* save output sequentially */
            { "seqsave", no_argument,                NULL,                0 },
                   /* print documentation */
            { "docs", no_argument,                &do_print_docs,                true },
                           /* boundary values file */
            { "boundaries", required_argument, NULL, 'b' },
                   /* boundary values comma-separated list */
            { "bound-vals", required_argument, NULL, 'B' },
                   /* convert float[0,1] to uint16[0,65533] */
            { "convert-u16", no_argument, NULL, 'C' },
                   /* absolute delta tolerance for convergence */
            { "delta", required_argument, NULL, 'd' },
                   /* relative delta tolerance for boundary update */
            { "delta-bnd", required_argument, NULL, 'D' },
                   /* extract mask from labels file (int8) */
            { "extract-mask", no_argument, NULL, 'E' },
                   /* force initial boundary values */
            { "force-bnd", no_argument, NULL, 'f' },
                   /* convert uint16[0,65533] to float[0,1] */
            { "convert-f32", no_argument, NULL, 'F' },
                   /* generate mask from labels file (uint8) */
            { "get-mask", no_argument, NULL, 'G' },
                   /* file with initial solution (" TYPESTR ") */
            { "initial", required_argument, NULL, 'i' },
                   /* file with labels ((u)int8) */
            { "labels", required_argument, NULL, 'l' },
                   /* file with mask (uint8) */
            { "mask", required_argument, NULL, 'm' },
                   /* file with per-slice mask sizes (uint64) */
            { "mask-sizes", required_argument, NULL, 'M' },
                   /* maximum number of iterations */
            { "itermax", required_argument, NULL, 'n' },
                   /* output file */
            { "output", required_argument, NULL, 'o' },
                   /* number of regions */
            { "regions", required_argument, NULL, 'r' },
                   /* size of x dimension */
            { "xdim", required_argument, NULL, 'x' },
                   /* size of y dimension */
            { "ydim", required_argument, NULL, 'y' },
                   /* size of z dimension */
            { "zdim", required_argument, NULL, 'z' },
                   { NULL, 0, NULL, 0 }
        };

        /* build optstring */
        const char optstring[] = "b:B:Cd:D:EfFGi:l:m:M:n:o:r:x:y:z:";

        int opt, longidx;
        bool getopt_verbose = false; // --verbose passed
        bool longopt_isset[9] = { 0 };
        char *longopt_arg[9] = { 0 };
        (void)longopt_isset;
(void)longopt_arg;

        while((opt = getopt_long(argc,argv,optstring,optlist,&longidx)) != -1) {
            if(opt == 0) { /* long options with no short option */
                if(longidx < 9)
                    longopt_isset[longidx] = true;
                if(longidx == 0 || longidx == 1) { // --help or --usage
                        print_usage();
    exit(EXIT_SUCCESS);
                } else if(longidx == 2) { // --verbose
                    getopt_verbose = true;
                } else if(longidx >= 3) {
                    if(longidx < 9) {
                            if(NULL != (optarg) &&       NULL == ((longopt_arg[longidx]) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    }

                    longidx -= 3; // subtract default

                    /* set optlabel for each long option */
                    switch(longidx) {
                                            case 0:
                            optlabel = "--mmap";
                            break;
                                           case 1:
                            optlabel = "--parload";
                            break;
                                           case 2:
                            optlabel = "--seqload";
                            break;
                                           case 3:
                            optlabel = "--parsave";
                            break;
                                           case 4:
                            optlabel = "--seqsave";
                            break;
                                           case 5:
                            optlabel = "--docs";
                            break;
                                       }

                    /* user-defined actions */
                        switch(longidx) {
        case 0: // --mmap
            loadm = LOAD_MMAP;
            break;
        case 1: // --parload
            loadm = LOAD_PARALLEL;
            break;
        case 2: // --seqload
            loadm = LOAD_SEQUENTIAL;
            break;
        case 3: // --parsave
            savem = SAVE_PARALLEL;
            break;
        case 4: // --seqsave
            savem = SAVE_SEQUENTIAL;
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
                                    case 'b':
                        optlabel = "-b/--boundaries";

                            if(opts.bnd_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.bnd_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . bnd_file); (opts . bnd_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.bnd_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'B':
                        optlabel = "-B/--bound-vals";

                            if(opts.bnd_vals_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.bnd_vals_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . bnd_vals); (opts . bnd_vals) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.bnd_vals) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'C':
                        optlabel = "-C/--convert-u16";

                            if(opts.conv_u16_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.conv_u16_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.conv_u16 = !false;
                    break;
                                   case 'd':
                        optlabel = "-d/--delta";

                            if(opts.delta_tol_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.delta_tol_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.delta_tol)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'D':
                        optlabel = "-D/--delta-bnd";

                            if(opts.delta_bnd_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.delta_bnd_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtod_check(optarg,&opts.delta_bnd)) {
        error("Not a floating-point or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'E':
                        optlabel = "-E/--extract-mask";

                            if(opts.ext_mask_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.ext_mask_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.ext_mask = !false;
                    break;
                                   case 'f':
                        optlabel = "-f/--force-bnd";

                            if(opts.force_bnd_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.force_bnd_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.force_bnd = !false;
                    break;
                                   case 'F':
                        optlabel = "-F/--convert-f32";

                            if(opts.conv_f32_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.conv_f32_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.conv_f32 = !false;
                    break;
                                   case 'G':
                        optlabel = "-G/--get-mask";

                            if(opts.get_mask_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.get_mask_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            opts.get_mask = !false;
                    break;
                                   case 'i':
                        optlabel = "-i/--initial";

                            if(opts.init_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.init_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . init_file); (opts . init_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.init_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'l':
                        optlabel = "-l/--labels";

                            if(opts.labels_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.labels_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . labels_file); (opts . labels_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.labels_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'm':
                        optlabel = "-m/--mask";

                            if(opts.mask_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.mask_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . mask_file); (opts . mask_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.mask_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'M':
                        optlabel = "-M/--mask-sizes";

                            if(opts.nmask_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nmask_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . nmask_file); (opts . nmask_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.nmask_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'n':
                        optlabel = "-n/--itermax";

                            if(opts.iter_max_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.iter_max_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.iter_max)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'o':
                        optlabel = "-o/--output";

                            if(opts.out_file_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.out_file_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            { free(opts . out_file); (opts . out_file) = NULL; }
        if(NULL != (optarg) &&       NULL == ((opts.out_file) = strdup(optarg)))        { error0("out of memory"); exit(EXIT_FAILURE); }
                    break;
                                   case 'r':
                        optlabel = "-r/--regions";

                            if(opts.nregion_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nregion_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtou8_check(optarg,&opts.nregion)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'x':
                        optlabel = "-x/--xdim";

                            if(opts.nx_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nx_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.nx)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'y':
                        optlabel = "-y/--ydim";

                            if(opts.ny_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.ny_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.ny)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                                   case 'z':
                        optlabel = "-z/--zdim";

                            if(opts.nz_isset) {
        error("Duplicate option found: %s",optlabel);
        exit(EXIT_FAILURE);
    }
                        opts.nz_isset = true; 

                        /* option default actions, overriden by user-defined actions below */
                            if(optarg && strtosize_check(optarg,&opts.nz)) {
        error("Not an unsigned integer, out of bounds, or empty argument passed to %s: %s",optlabel,optarg);
        exit(EXIT_FAILURE);
    }
                    break;
                               }

                /* user-defined actions */
                    switch(opt) {
        case 'd':     if(! (opts.delta_tol > 0) ) { error0("Condition not fulfilled: opts.delta_tol > 0"); exit(EXIT_FAILURE); }
 break;
        case 'r':     if(! (opts.nregion >= 2) ) { error0("Condition not fulfilled: opts.nregion >= 2"); exit(EXIT_FAILURE); }
 break;
    }
            }
        }

        /* check if required options are present */
                                                                                                                                                                                                                                                                                                                     if(!opts.nregion_isset) {
        error("Please specify %s: %s","-r/--regions","number of regions");
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
            if(NULL == opts.bnd_vals) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.bnd_vals) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'B',maxlen,"bound-vals"
                ,opts.bnd_vals_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.bnd_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.bnd_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'b',maxlen,"boundaries"
                ,opts.bnd_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.conv_f32 ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'F',maxlen,"convert-f32"
                ,opts.conv_f32_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.conv_u16 ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'C',maxlen,"convert-u16"
                ,opts.conv_u16_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.delta_tol,opts.delta_tol);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'d',maxlen,"delta"
                ,opts.delta_tol_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "g" " (%" "a" ")",opts.delta_bnd,opts.delta_bnd);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'D',maxlen,"delta-bnd"
                ,opts.delta_bnd_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.ext_mask ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'E',maxlen,"extract-mask"
                ,opts.ext_mask_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.force_bnd ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'f',maxlen,"force-bnd"
                ,opts.force_bnd_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%s",opts.get_mask ? "<true>" : "<false>");
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'G',maxlen,"get-mask"
                ,opts.get_mask_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.init_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.init_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'i',maxlen,"initial"
                ,opts.init_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.iter_max);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'n',maxlen,"itermax"
                ,opts.iter_max_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.labels_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.labels_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'l',maxlen,"labels"
                ,opts.labels_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.mask_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.mask_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'m',maxlen,"mask"
                ,opts.mask_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.nmask_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.nmask_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'M',maxlen,"mask-sizes"
                ,opts.nmask_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            if(NULL == opts.out_file) { (void)snprintf(buf,sizeof(buf),"<none>"); }
    else if(snprintf(buf,sizeof(buf),"%s",opts.out_file) >= (int)sizeof(buf)) {
        (void)snprintf(buf + sizeof(buf) - 4,4,"...");
    }
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'o',maxlen,"output"
                ,opts.out_file_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" PRIu8,opts.nregion);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'r',maxlen,"regions"
                ,opts.nregion_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.nx);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'x',maxlen,"xdim"
                ,opts.nx_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.ny);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'y',maxlen,"ydim"
                ,opts.ny_isset ? "  <set>" : "<unset>","",buf," .");
    }
       {
        char buf[1024] = "<non-representable>";
            (void)snprintf(buf,sizeof(buf),"%" "zu",opts.nz);
        verbose("%s-%c, --%-*s %s%s " "=" " %s%s"
                ,"      ",'z',maxlen,"zdim"
                ,opts.nz_isset ? "  <set>" : "<unset>","",buf," .");
    }
   
    
                verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"docs"
            ,longopt_isset[8] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"help"
            ,longopt_isset[0] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"mmap"
            ,longopt_isset[3] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"parload"
            ,longopt_isset[4] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"parsave"
            ,longopt_isset[6] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"seqload"
            ,longopt_isset[5] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"seqsave"
            ,longopt_isset[7] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"usage"
            ,longopt_isset[1] ? "  <set>" : "<unset>","","","");
               verbose("%s    --%-*s %s%s " "" " %s%s"
            ,"      ",maxlen,"verbose"
            ,longopt_isset[2] ? "  <set>" : "<unset>","","","");
   }
        }

        const size_t len_optab_long = 9;
        {
const size_t _len = len_optab_long;
for(size_t _idx = 0; _idx < _len; ++_idx) {
const size_t _0 = _idx;
const size_t _idx = 0; (void)_idx;
{ free(( ( ( longopt_arg ) ) [ _0 ] )); (( ( ( longopt_arg ) ) [ _0 ] )) = NULL; }}
}

        argv[0] = old_argv0;
    }

    size_t NREGION = opts.nregion; //XXX: making this const results in internal compiler error with GCC 13.1.1
    struct _dims dims = { opts.nx, opts.ny, opts.nz };

    if(do_print_docs) {
        print_docs();
        exit(EXIT_SUCCESS);
    }

    if(opts.get_mask) {
        if(!opts.labels_file_isset) FATAL0("missing required input labels file");
        if(!opts.mask_file_isset) FATAL0("missing required output mask file");
        if(!opts.nmask_file_isset) FATAL0("missing required output mask sizes file");

        uint8_t *labels = NULL;

        errno = 0;
        int fd = open(opts.labels_file,O_RDONLY);
        if(fd == -1) ERRFATAL("open[r]","%s",opts.labels_file);

        void *labels_map = NULL;
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            labels_map = mmap(NULL,NVOX * sizeof(*labels),PROT_READ,MAP_SHARED,fd,0);
            if(MAP_FAILED == labels_map) PERROR("mmap[r]");

            errno = 0;
            if(madvise(labels_map,NVOX * sizeof(*labels),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            labels = labels_map;
        } else { // read
            errno = 0;
            labels = malloc(NVOX * sizeof(*labels));
            if(NULL == labels) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*labels));
            if(loadm == LOAD_PARALLEL) {
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(fd,labels) reduction(+:nerr) shared(dims)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*labels);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pread(fd,&labels[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pread failed: %zu times with %zu items",nerr,NVOX);
                close(fd);
            } else {
                FILE *fp = fdopen(fd,"rb");
                if(fread(labels,sizeof(*labels),NVOX,fp) != NVOX) FATAL("fread failed: %zu items",NVOX);
                fclose(fp);
                fd = -1; // invalid from here on
            }
        }

        uint64_t nmask[NZ];
        memset(nmask,0x0,NZ * sizeof(*nmask));
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels) reduction(+:nmask[:NZ]) shared(dims,NREGION)
#endif
        for(size_t i = 0; i < NVOX; ++i)
            nmask[i / NXY] += INMASK_LABEL(labels[i]);

        size_t ntot = 0;
        size_t mask_off[NZ];
        for(size_t i = 0; i < NZ; ++i) {
            mask_off[i] = ntot;
            ntot += nmask[i];
        }
        if(mask_off[1] != 0 || mask_off[NZ - 1] != ntot)
            FATAL("malformed mask with size %zu",ntot);

        errno = 0;
        uint8_t *mask = calloc(ntot,sizeof(*mask)); // init to zero (interior)
        if(NULL == mask) ERRFATAL("calloc","%zu bytes",ntot * sizeof(*mask));

        { // get mask
            size_t nbad = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels,mask,mask_off) shared(dims,NREGION) reduction(+:nbad)
#endif
            for(size_t k = 1; k < NZ - 1; ++k) {
                const uint8_t *slice = &labels[k * NXY];
                uint8_t *inmask = &mask[mask_off[k]];
                for(size_t ix = 0, im = 0; ix < NXY; ++ix) {
                    const size_t gix = (size_t)ix + k * NXY;
                    const uint8_t layer = slice[ix];
                    if(INMASK_LABEL(layer)) { // in mask
                        uint8_t buf[NHOOD_NEUMANN_N];
                        get_nhood3d_neumann_u8(gix,labels,dims,buf);
                        if(layer < NREGION - 1) {
                            for(size_t i = 0; i < NHOOD_NEUMANN_N; ++i) {
                                if(layer - 1 == buf[i]) { inmask[im] = layer; break; } // L* top
                            }
                        } else if(layer == NREGION - 1) {
                            for(size_t i = 0; i < NHOOD_NEUMANN_N; ++i) {
                                if(layer - 1 == buf[i]) { inmask[im] = layer; break; } // L6 top
                                if(NREGION == buf[i]) { inmask[im] = NREGION; break; } // L6 bottom = WM
                            }
                        }
                        ++im;
                    } else if(layer > NREGION) ++nbad;
                }
            }
            if(nbad > 0) FATAL("have %zu values > NREGION in layers file",nbad);
        }

        { // logging
            size_t nbnd[1 + NREGION];
            memset(nbnd,0x0,(1 + NREGION) * sizeof(*nbnd)); // init
            for(size_t i = 0; i < ntot; ++i) nbnd[mask[i]]++;
            info("interior size: %zu",nbnd[0]);
            info0("boundary sizes:");
            for(size_t i = 1; i < 1 + NREGION; ++i) printf("    %zu\n",nbnd[i]);
        }

        { // save mask sizes
            errno = 0;
            FILE *fp = fopen(opts.nmask_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.nmask_file);
            fwrite(nmask,sizeof(*nmask),NZ,fp);
            fclose(fp);
        }

        { // save mask
            errno = 0;
            FILE *fp = fopen(opts.mask_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.mask_file);
            fwrite(mask,sizeof(*mask),ntot,fp);
            fclose(fp);
        }

        free(mask);

        if(loadm == LOAD_MMAP) { // munmap
            errno = 0;
            if(munmap(labels_map,NVOX * sizeof(*labels)) == -1) PERROR("munmap");
            close(fd);
        } else free(labels);

        return 0;
    }

    if(opts.ext_mask) {
#define INMASK_INT8(x) ((x) >= 0 && (x) <= (int8_t)NREGION)
        if(!opts.labels_file_isset) FATAL0("missing required input labels file");
        if(!opts.mask_file_isset) FATAL0("missing required output mask file");
        if(!opts.nmask_file_isset) FATAL0("missing required output mask sizes file");

                int8_t *labels = NULL;

        errno = 0;
        int fd = open(opts.labels_file,O_RDONLY);
        if(fd == -1) ERRFATAL("open[r]","%s",opts.labels_file);

        void *labels_map = NULL;
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            labels_map = mmap(NULL,NVOX * sizeof(*labels),PROT_READ,MAP_SHARED,fd,0);
            if(MAP_FAILED == labels_map) PERROR("mmap[r]");

            errno = 0;
            if(madvise(labels_map,NVOX * sizeof(*labels),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            labels = labels_map;
        } else { // read
            errno = 0;
            labels = malloc(NVOX * sizeof(*labels));
            if(NULL == labels) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*labels));
            if(loadm == LOAD_PARALLEL) {
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(fd,labels) reduction(+:nerr) shared(dims)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*labels);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pread(fd,&labels[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pread failed: %zu times with %zu items",nerr,NVOX);
                close(fd);
            } else {
                FILE *fp = fdopen(fd,"rb");
                if(fread(labels,sizeof(*labels),NVOX,fp) != NVOX) FATAL("fread failed: %zu items",NVOX);
                fclose(fp);
                fd = -1; // invalid from here on
            }
        }
                uint64_t nmask[NZ];
        memset(nmask,0x0,NZ * sizeof(*nmask));
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels) reduction(+:nmask[:NZ]) shared(dims,NREGION)
#endif
        for(size_t i = 0; i < NVOX; ++i)
            nmask[i / NXY] += INMASK_INT8(labels[i]);

        size_t ntot = 0;
        size_t mask_off[NZ];
        for(size_t i = 0; i < NZ; ++i) {
            mask_off[i] = ntot;
            ntot += nmask[i];
        }
        if(mask_off[1] != 0 || mask_off[NZ - 1] != ntot)
            FATAL("malformed mask with size %zu",ntot);

        errno = 0;
        uint8_t *mask = calloc(ntot,sizeof(*mask)); // init to zero (interior)
        if(NULL == mask) ERRFATAL("calloc","%zu bytes",ntot * sizeof(*mask));

        { // extract mask
            size_t nbad = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels,mask,mask_off) shared(dims,NREGION) reduction(+:nbad)
#endif
            for(size_t k = 1; k < NZ - 1; ++k) {
                const int8_t *slice = &labels[k * NXY];
                uint8_t *inmask = &mask[mask_off[k]];
                for(size_t ix = 0, im = 0; ix < NXY; ++ix) {
                    const int8_t layer = slice[ix];
                    if(INMASK_INT8(layer)) inmask[im++] = layer; // in mask, set mask
                    else if(layer > (int8_t)NREGION) ++nbad;
                }
            }
            if(nbad > 0) FATAL("have %zu values > NREGION in layers file",nbad);
        }

                { // logging
            size_t nbnd[1 + NREGION];
            memset(nbnd,0x0,(1 + NREGION) * sizeof(*nbnd)); // init
            for(size_t i = 0; i < ntot; ++i) nbnd[mask[i]]++;
            info("interior size: %zu",nbnd[0]);
            info0("boundary sizes:");
            for(size_t i = 1; i < 1 + NREGION; ++i) printf("    %zu\n",nbnd[i]);
        }
                { // save mask sizes
            errno = 0;
            FILE *fp = fopen(opts.nmask_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.nmask_file);
            fwrite(nmask,sizeof(*nmask),NZ,fp);
            fclose(fp);
        }

        { // save mask
            errno = 0;
            FILE *fp = fopen(opts.mask_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.mask_file);
            fwrite(mask,sizeof(*mask),ntot,fp);
            fclose(fp);
        }

        free(mask);
                if(loadm == LOAD_MMAP) { // munmap
            errno = 0;
            if(munmap(labels_map,NVOX * sizeof(*labels)) == -1) PERROR("munmap");
            close(fd);
        } else free(labels);

        return 0;
#undef INMASK_INT8
    }

    if(opts.conv_u16) {
        if(!opts.labels_file_isset) FATAL0("missing required input labels file");
        if(!opts.init_file_isset) FATAL0("missing required input initial solution file");
        if(!opts.out_file_isset) FATAL0("missing required output file");

        uint8_t *labels = NULL;
        float *smooth_in = NULL;
        uint16_t *smooth_out = NULL;

        void *labels_map = NULL;
        void *smooth_in_map = NULL;
        void *smooth_out_map = NULL;

        int fd[3] = { -1, -1, -1 };
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            fd[0] = open(opts.labels_file,O_RDONLY);
            if(fd[0] == -1) ERRFATAL("open[r]","%s",opts.labels_file);

            errno = 0;
            fd[1] = open(opts.init_file,O_RDONLY);
            if(fd[1] == -1) ERRFATAL("open[r]","%s",opts.init_file);

            errno = 0;
            fd[2] = open(opts.out_file,O_RDWR | O_CREAT,CREAT_MASK);
            if(fd[2] == -1) ERRFATAL("open[rw]","%s",opts.out_file);

            errno = 0;
            if(ftruncate(fd[2],NVOX * sizeof(*smooth_out)) == -1)
                ERRFATAL("ftruncate","%zu bytes",NVOX * sizeof(*smooth_out));

            errno = 0;
            labels_map = mmap(NULL,NVOX * sizeof(*labels),PROT_READ,MAP_SHARED,fd[0],0);
            if(MAP_FAILED == labels_map) PERROR("mmap[r]");

            errno = 0;
            smooth_in_map = mmap(NULL,NVOX * sizeof(*smooth_in),PROT_READ,MAP_SHARED,fd[1],0);
            if(MAP_FAILED == smooth_in_map) PERROR("mmap[r]");

            errno = 0;
            smooth_out_map = mmap(NULL,NVOX * sizeof(*smooth_out),PROT_WRITE,MAP_SHARED,fd[2],0);
            if(MAP_FAILED == smooth_out_map) PERROR("mmap[w]");

            errno = 0;
            if(madvise(labels_map,NVOX * sizeof(*labels),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            errno = 0;
            if(madvise(smooth_in_map,NVOX * sizeof(*smooth_in),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            errno = 0;
            if(madvise(smooth_out_map,NVOX * sizeof(*smooth_out),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            labels = labels_map;
            smooth_in = smooth_in_map;
            smooth_out = smooth_out_map;
        } else {
            errno = 0;
            labels = malloc(NVOX * sizeof(*labels));
            if(NULL == labels) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*labels));

            errno = 0;
            smooth_in = malloc(NVOX * sizeof(*smooth_in));
            if(NULL == smooth_in) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*smooth_in));

            errno = 0;
            smooth_out = malloc(NVOX * sizeof(*smooth_out));
            if(NULL == smooth_out) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*smooth_out));

            if(loadm == LOAD_SEQUENTIAL) {
                { // load labels
                    errno = 0;
                    FILE *fp = fopen(opts.labels_file,"rb");
                    if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.labels_file);
                    if(fread(labels,sizeof(*labels),NVOX,fp) != NVOX)
                        FATAL("fread failed: %zu items",NVOX);
                    fclose(fp);
                }
                { // load smooth_in
                    errno = 0;
                    FILE *fp = fopen(opts.init_file,"rb");
                    if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.init_file);
                    if(fread(smooth_in,sizeof(*smooth_in),NVOX,fp) != NVOX)
                        FATAL("fread failed: %zu items",NVOX);
                    fclose(fp);
                }
            } else FATAL0("parload unimplemented");
        }

        // we quantize the [0,1] interval into [0,65533]
        // we set the outside to 65534 and the WM to 65535
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels,smooth_in) shared(smooth_out,dims,NREGION)
#endif
        for(size_t i = 0; i < NVOX; ++i) {
            if(0 == labels[i]) {
                smooth_out[i] = OUT_U16; // 65534 outside
            } else if(NREGION == labels[i]) {
                smooth_out[i] = WM_U16; // 65535 at WM
            } else {
                smooth_out[i] = smooth_in[i] * U16FACTOR; // 0 .. 65533
            }
        }

        if(loadm == LOAD_MMAP) { // munmap
            errno = 0;
            if(munmap(labels_map,NVOX * sizeof(*labels)) == -1) PERROR("munmap");
            errno = 0;
            if(munmap(smooth_in_map,NVOX * sizeof(*smooth_in)) == -1) PERROR("munmap");
            errno = 0;
            if(munmap(smooth_out_map,NVOX * sizeof(*smooth_out)) == -1) PERROR("munmap");

            close(fd[0]);
            close(fd[1]);
            close(fd[2]);
        } else { // write
            errno = 0;
            FILE *fp = fopen(opts.out_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.out_file);

            if(savem == SAVE_PARALLEL) {
                int fd = fileno(fp);
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:nerr)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*smooth_out);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pwrite(fd,&smooth_out[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pwrite failed: %zu times with %zu items",nerr,NVOX);
            } else {
                if(fwrite(smooth_out,sizeof(*smooth_out),NVOX,fp) != NVOX)
                    FATAL("fwrite failed: %zu items",NVOX);
            }

            fclose(fp);

            free(labels);
            free(smooth_in);
            free(smooth_out);
        }

        return 0;
    }

    if(opts.conv_f32) {
        if(!opts.labels_file_isset) FATAL0("missing required input labels file");
        if(!opts.init_file_isset) FATAL0("missing required input initial solution file");
        if(!opts.out_file_isset) FATAL0("missing required output file");

        uint8_t *labels = NULL;
        uint16_t *smooth_in = NULL;
        float *smooth_out = NULL;

        void *labels_map = NULL;
        void *smooth_in_map = NULL;
        void *smooth_out_map = NULL;

        int fd[3] = { -1, -1, -1 };
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            fd[0] = open(opts.labels_file,O_RDONLY);
            if(fd[0] == -1) ERRFATAL("open[r]","%s",opts.labels_file);

            errno = 0;
            fd[1] = open(opts.init_file,O_RDONLY);
            if(fd[1] == -1) ERRFATAL("open[r]","%s",opts.init_file);

            errno = 0;
            fd[2] = open(opts.out_file,O_RDWR | O_CREAT,CREAT_MASK);
            if(fd[2] == -1) ERRFATAL("open[rw]","%s",opts.out_file);

            errno = 0;
            if(ftruncate(fd[2],NVOX * sizeof(*smooth_out)) == -1)
                ERRFATAL("ftruncate","%zu bytes",NVOX * sizeof(*smooth_out));

            errno = 0;
            labels_map = mmap(NULL,NVOX * sizeof(*labels),PROT_READ,MAP_SHARED,fd[0],0);
            if(MAP_FAILED == labels_map) PERROR("mmap[r]");

            errno = 0;
            smooth_in_map = mmap(NULL,NVOX * sizeof(*smooth_in),PROT_READ,MAP_SHARED,fd[1],0);
            if(MAP_FAILED == smooth_in_map) PERROR("mmap[r]");

            errno = 0;
            smooth_out_map = mmap(NULL,NVOX * sizeof(*smooth_out),PROT_WRITE,MAP_SHARED,fd[2],0);
            if(MAP_FAILED == smooth_out_map) PERROR("mmap[w]");

            errno = 0;
            if(madvise(labels_map,NVOX * sizeof(*labels),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            errno = 0;
            if(madvise(smooth_in_map,NVOX * sizeof(*smooth_in),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            errno = 0;
            if(madvise(smooth_out_map,NVOX * sizeof(*smooth_out),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            labels = labels_map;
            smooth_in = smooth_in_map;
            smooth_out = smooth_out_map;
        } else {
            errno = 0;
            labels = malloc(NVOX * sizeof(*labels));
            if(NULL == labels) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*labels));

            errno = 0;
            smooth_in = malloc(NVOX * sizeof(*smooth_in));
            if(NULL == smooth_in) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*smooth_in));

            errno = 0;
            smooth_out = malloc(NVOX * sizeof(*smooth_out));
            if(NULL == smooth_out) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*smooth_out));

            if(loadm == LOAD_SEQUENTIAL) {
                { // load labels
                    errno = 0;
                    FILE *fp = fopen(opts.labels_file,"rb");
                    if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.labels_file);
                    if(fread(labels,sizeof(*labels),NVOX,fp) != NVOX)
                        FATAL("fread failed: %zu items",NVOX);
                    fclose(fp);
                }
                { // load smooth_in
                    errno = 0;
                    FILE *fp = fopen(opts.init_file,"rb");
                    if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.init_file);
                    if(fread(smooth_in,sizeof(*smooth_in),NVOX,fp) != NVOX)
                        FATAL("fread failed: %zu items",NVOX);
                    fclose(fp);
                }
            } else FATAL0("parload unimplemented");
        }

        // we expand the [0,65533] interval into [0,1]
        // we set the outside to 2.0 and the WM to -1.0
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(labels,smooth_in) shared(smooth_out,dims,NREGION)
#endif
        for(size_t i = 0; i < NVOX; ++i) {
            if(0 == labels[i]) {
                smooth_out[i] = OUT_FLT; // 2.0 outside
            } else if(NREGION == labels[i]) {
                smooth_out[i] = WM_FLT; // -1.0 at WM
            } else {
                smooth_out[i] = (double)smooth_in[i] / U16FACTOR; // [0,1]
            }
        }

        if(loadm == LOAD_MMAP) { // munmap
            errno = 0;
            if(munmap(labels_map,NVOX * sizeof(*labels)) == -1) PERROR("munmap");
            errno = 0;
            if(munmap(smooth_in_map,NVOX * sizeof(*smooth_in)) == -1) PERROR("munmap");
            errno = 0;
            if(munmap(smooth_out_map,NVOX * sizeof(*smooth_out)) == -1) PERROR("munmap");

            close(fd[0]);
            close(fd[1]);
            close(fd[2]);
        } else { // write
            errno = 0;
            FILE *fp = fopen(opts.out_file,"wb");
            if(NULL == fp) ERRFATAL("fopen[w]","%s",opts.out_file);

            if(savem == SAVE_PARALLEL) {
                int fd = fileno(fp);
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:nerr)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*smooth_out);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pwrite(fd,&smooth_out[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pwrite failed: %zu times with %zu items",nerr,NVOX);
            } else {
                if(fwrite(smooth_out,sizeof(*smooth_out),NVOX,fp) != NVOX)
                    FATAL("fwrite failed: %zu items",NVOX);
            }

            fclose(fp);

            free(labels);
            free(smooth_in);
            free(smooth_out);
        }

        return 0;
    }

    /* main functionality */

    /* setup signal handlers */
    { // SIGINT
        struct sigaction act = { 0 };
        act.sa_handler = terminate_handler;
        sigaction(SIGINT,&act,NULL);
    }
    { // SIGALRM
        struct sigaction act = { 0 };
        act.sa_handler = alarm_handler;
        sigaction(SIGALRM,&act,NULL);
    }

    if(!opts.mask_file_isset) FATAL0("missing required input mask file");
    if(!opts.nmask_file_isset) FATAL0("missing required input mask sizes file");
    if(!opts.out_file_isset) FATAL0("missing required output file");

    /* load sizes per slice and mask */
    uint64_t nmask[NZ];
    {
        errno = 0;
        FILE *fp = fopen(opts.nmask_file,"rb");
        if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.nmask_file);
        if(fread(nmask,sizeof(*nmask),NZ,fp) != NZ) FATAL("fread failed: %zu items",NZ);
        fclose(fp);
    }

    size_t ntot = 0;
    uint8_t *mask = NULL;
    uint8_t *maskptr[NZ]; // per-slice pointers
    data_t *maskvals = NULL;
    data_t *maskvalsptr[NZ]; // per-slice pointers
    { // load mask
        size_t mask_off[NZ];
        for(size_t k = 0; k < NZ; ++k) {
            mask_off[k] = ntot;
            ntot += nmask[k];
        }
        if(mask_off[0] != 0 || mask_off[NZ - 1] != ntot)
            FATAL("malformed mask with size %zu",ntot);

        errno = 0;
        mask = malloc(ntot * sizeof(*mask));
        if(NULL == mask) ERRFATAL("malloc","%zu bytes",ntot * sizeof(*mask));

        errno = 0;
        maskvals = calloc(ntot,sizeof(*maskvals));
        if(NULL == maskvals) ERRFATAL("calloc","%zu bytes",ntot * sizeof(*maskvals));

        maskptr[0] = maskptr[NZ - 1] = NULL;
        maskvalsptr[0] = maskvalsptr[NZ - 1] = NULL;
        for(size_t k = 1; k < NZ - 1; ++k) {
            maskptr[k] = &mask[mask_off[k]];
            maskvalsptr[k] = &maskvals[mask_off[k]];
        }

        if(loadm == LOAD_PARALLEL) {
            errno = 0;
            int fd = open(opts.mask_file,O_RDONLY);
            if(-1 == fd) ERRFATAL("open[r]","%s",opts.mask_file);
            size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(fd,nmask) reduction(+:nerr) shared(dims,maskptr,mask_off)
#endif
            for(size_t k = 1; k < NZ - 1; ++k) {
                size_t count = nmask[k] * sizeof(*mask);
                off_t offset = mask_off[k] * sizeof(*mask);
                ssize_t ret = pread(fd,maskptr[k],count,offset);
                if(ret == -1 || (size_t)ret != count) ++nerr;
            }
            if(nerr > 0) FATAL("pread failed: %zu times with %zu items",nerr,ntot);
            close(fd);
        } else {
            errno = 0;
            FILE *fp = fopen(opts.mask_file,"rb");
            if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.mask_file);
            if(fread(mask,sizeof(*mask),ntot,fp) != ntot) FATAL("fread failed: %zu items",ntot);
            fclose(fp);
        }
    }

    size_t nbnd[1 + NREGION];
    memset(nbnd,0x0,(1 + NREGION) * sizeof(*nbnd)); // init
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(ntot,mask) reduction(+:nbnd[:1 + NREGION])
#endif
    for(size_t i = 0; i < ntot; ++i)
        nbnd[mask[i]]++;

    info("loaded mask with %zu voxels",ntot);
    info("interior size: %zu",nbnd[0]);
    info0("boundary sizes:");
    for(size_t i = 1; i < 1 + NREGION; ++i) info("    %zu",nbnd[i]);

    int fdmap = -1;
    data_t *res = NULL;
    void *res_map = NULL;

    /* setup initial solution */
    if(opts.init_file_isset) { // load from file
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            fdmap = open(opts.init_file,O_RDWR | O_DSYNC);
            if(-1 == fdmap) ERRFATAL("open[rw]","%s",opts.init_file);

            errno = 0;
            res_map = mmap(NULL,NVOX * sizeof(*res),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_NORESERVE,fdmap,0);
            if(MAP_FAILED == res_map) ERRFATAL("mmap[rw]","%s",opts.init_file);

            if(madvise(res_map,NVOX * sizeof(*res),MADV_RANDOM) == -1) PERROR("madvise");

            res = res_map;
        } else {
            errno = 0;
            res = malloc(NVOX * sizeof(*res));
            if(NULL == res) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*res));
            if(loadm == LOAD_PARALLEL) {
                errno = 0;
                int fd = open(opts.init_file,O_RDONLY);
                if(-1 == fd) ERRFATAL("open[r]","%s",opts.init_file);
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:nerr) shared(dims)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*res);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pread(fd,&res[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pread failed: %zu times with %zu items",nerr,NVOX);
                close(fd);
            } else {
                errno = 0;
                FILE *fp = fopen(opts.init_file,"rb");
                if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.init_file);
                if(fread(res,sizeof(*res),NVOX,fp) != NVOX) FATAL("fread failed: %zu items",NVOX);
                fclose(fp);
            }
        }

        info("loaded initial solution with size %zux%zux%zu (%zu voxels)",NX,NY,NZ,NVOX);
        warn0("beware of in-file data ordering, NZ must be number of slices");
    } else if(opts.labels_file_isset) { // init from labels
        info0("no initial solution provided, will derive it from labels");

        res = malloc(NVOX * sizeof(*res));
        if(NULL == res) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*res));

                uint8_t *labels = NULL;

        errno = 0;
        int fd = open(opts.labels_file,O_RDONLY);
        if(fd == -1) ERRFATAL("open[r]","%s",opts.labels_file);

        void *labels_map = NULL;
        if(loadm == LOAD_MMAP) { // mmap
            errno = 0;
            labels_map = mmap(NULL,NVOX * sizeof(*labels),PROT_READ,MAP_SHARED,fd,0);
            if(MAP_FAILED == labels_map) PERROR("mmap[r]");

            errno = 0;
            if(madvise(labels_map,NVOX * sizeof(*labels),MADV_SEQUENTIAL) == -1)
                PERROR("madvise");

            labels = labels_map;
        } else { // read
            errno = 0;
            labels = malloc(NVOX * sizeof(*labels));
            if(NULL == labels) ERRFATAL("malloc","%zu bytes",NVOX * sizeof(*labels));
            if(loadm == LOAD_PARALLEL) {
                size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(fd,labels) reduction(+:nerr) shared(dims)
#endif
                for(size_t i = 0; i < NZ; ++i) {
                    size_t count = NXY * sizeof(*labels);
                    off_t offset = (off_t)i * count;
                    ssize_t ret = pread(fd,&labels[i * NXY],count,offset);
                    if(ret == -1 || (size_t)ret != count) ++nerr;
                }
                if(nerr > 0) FATAL("pread failed: %zu times with %zu items",nerr,NVOX);
                close(fd);
            } else {
                FILE *fp = fdopen(fd,"rb");
                if(fread(labels,sizeof(*labels),NVOX,fp) != NVOX) FATAL("fread failed: %zu items",NVOX);
                fclose(fp);
                fd = -1; // invalid from here on
            }
        }

#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res) shared(labels,dims,NREGION)
#endif
        for(size_t i = 0; i < NVOX; ++i) {
#if U16DATA
            if(0 == labels[i]) res[i] = OUT_U16;
            else if(NREGION == labels[i]) res[i] = WM_U16;
            else if (2 == NREGION) res[i] = 0;
            else res[i] = ((NREGION - 1 - labels[i]) / (float)(NREGION - 2)) * U16FACTOR;
#elif FLTDATA
            if(0 == labels[i]) res[i] = OUT_FLT;
            else if(NREGION == labels[i]) res[i] = WM_FLT;
            else if (2 == NREGION) res[i] = 0.0;
            else res[i] = ((NREGION - 1 - labels[i]) / (float)(NREGION - 2));
#endif
        }

                if(loadm == LOAD_MMAP) { // munmap
            errno = 0;
            if(munmap(labels_map,NVOX * sizeof(*labels)) == -1) PERROR("munmap");
            close(fd);
        } else free(labels);
    } else FATAL0("no initial solution provided, and no labels file to derive it from");

    size_t slice_rle_n[NZ]; memset(slice_rle_n,0x0,NZ * sizeof(*slice_rle_n));
    uint32_t *slice_rle_start[NZ];
    uint32_t *slice_rle_end[NZ];

    { // check mask sizes and compute slice RLE
        uint64_t ninmask[NZ]; memset(ninmask,0x0,NZ * sizeof(*ninmask));
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res) shared(ninmask,slice_rle_n,dims) schedule(dynamic)
#endif
        for(size_t k = 1; k < NZ - 1; ++k) {
            data_t *slice = &res[k * NXY];
            for(size_t ix = 0, rlen = 0; ix < NXY; ++ix) {
                if(INMASK(slice[ix])) {
                    ninmask[k]++;
                    if(rlen == 0) { ++rlen; slice_rle_n[k]++; } // range start
                } else if(rlen > 0) rlen = 0;
            }
        }
        for(size_t k = 1; k < NZ - 1; ++k) {
            slice_rle_start[k] = malloc(slice_rle_n[k] * sizeof(**slice_rle_start));
            slice_rle_end[k] = malloc(slice_rle_n[k] * sizeof(**slice_rle_end));
        }
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res) shared(slice_rle_start,slice_rle_end,dims) schedule(dynamic)
#endif
        for(size_t k = 1; k < NZ - 1; ++k) {
            data_t *slice = &res[k * NXY];
            for(size_t ix = 0, rlen = 0, ir = 0; ix < NXY; ++ix) {
                if(INMASK(slice[ix])) {
                    if(rlen == 0) { ++rlen; slice_rle_start[k][ir++] = ix; } // first index in range
                } else if(rlen > 0) {
                    slice_rle_end[k][ir - 1] = ix - 1; // last index in range (inclusive)
                    rlen = 0;
                }
            }
        }
#if DEBUG
        for(size_t k = 1; k < NZ - 1; ++k) {
            printf("%zu\n",k);
            for(size_t i = 0; i < slice_rle_n[k]; ++i) {
                printf("%"PRIu32"..%"PRIu32", len=%"PRIu16"\n",
                    slice_rle_start[k][i],slice_rle_end[k][i],
                    (uint16_t)(slice_rle_end[k][i] - slice_rle_start[k][i]) + 1);
            }
        }
#endif
#if DEBUG
        for(size_t k = 0; k < NZ; ++k) {
            fprintf(stderr,"%"PRIu64" == %"PRIu64"; %zu ranges\n",
                    ninmask[k],nmask[k],slice_rle_n[k]);
        }
#endif
        if(memcmp(ninmask,nmask,NZ * sizeof(*nmask)) != 0)
            FATAL0("input does not match mask");
    }

    data_t bnd_vals[1 + NREGION];
    bnd_vals[0] = 0;

    /* setup initial boundary values */
    if(opts.bnd_file_isset) { // read from file
        errno = 0;
        FILE *fp = fopen(opts.bnd_file,"rb");
        if(NULL == fp) ERRFATAL("fopen[r]","%s",opts.bnd_file);
        if(fread(&bnd_vals[1],sizeof(*bnd_vals),NREGION,fp) != NREGION)
            FATAL("fread failed: %zu items",(size_t)NREGION);
        fclose(fp);
    } else if(opts.bnd_vals_isset) { // parse from CLI
        char *rtxt = opts.bnd_vals;
        size_t nval = 0;
        data_t val;
        do {
            char *comma = strchr(rtxt,',');
            if(NULL != comma) *comma++ = '\0';
#if U16DATA
            if(strtou16_check(rtxt,&val))
                FATAL("failed integer conversion: %s",rtxt);
#elif FLTDATA
            if(strtof_check(rtxt,&val))
                FATAL("failed floating-point conversion: %s",rtxt);
#endif
            rtxt = comma;
            bnd_vals[++nval] = val;
        } while(NULL != rtxt && nval < NREGION);
        if(nval != NREGION) FATAL("expected %zu initial boundary values, got %zu",NREGION,nval);
    } else { // init uniformly
#if U16DATA
        for(size_t i = 0; i < NREGION; ++i)
            bnd_vals[1 + i] = ((NREGION - 1 - i) / (float)(NREGION - 1)) * U16FACTOR;
#elif FLTDATA
        for(size_t i = 0; i < NREGION; ++i)
            bnd_vals[1 + i] = ((NREGION - 1 - i) / (float)(NREGION - 1));
#endif
    }

    PRINTBND("initial boundary values: ");
    // delineate boundaries
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res,maskptr,bnd_vals) shared(slice_rle_n,slice_rle_start,slice_rle_end,dims) schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 0) {
                slice[ix] = bnd_vals[maskptr[k][im]];
            }
        }
    }
}

    info("start iteration up to %zu",opts.iter_max);
    if(opts.force_bnd) info0("forcing initial boundary values");
    else info("updating boundary values at relative delta %g",opts.delta_bnd);
    info("convergence at absolute delta %g",opts.delta_tol);

    double diff = 0.0;
    double delta = 0.0;
    double delta_rel = 0.0;
    double bnd_diff = 0.0;
    double bnd_delta = 0.0;
    double bnd_delta_rel = 0.0;
    /* smoothing loop */
    for(size_t iter = 1; iter <= opts.iter_max && keepRunning; ++iter) {
        struct timespec ts_begin, ts_end;
        clock_gettime(CLOCK_MONOTONIC,&ts_begin);
        delta = diff;
        diff = 0.0;

#ifdef _OPENMP
#pragma omp parallel default(none) firstprivate(res,maskptr,maskvalsptr) shared(diff,slice_rle_n,slice_rle_start,slice_rle_end,dims)
    {
#endif
        // smoothen and compute delta
#ifdef _OPENMP
#pragma omp for reduction(+:diff) schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk == 0) {
                double sum = convolve(dims,res,(size_t)ix + k * NXY);
maskvalsptr[k][im] = FROMFLOAT(sum);
diff += fabs(sum - TOFLOAT(slice[ix]));

            }
        }
    }
}
        // write back to array buffer
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk == 0) {
                slice[ix] = maskvalsptr[k][im];
            }
        }
    }
}
#ifdef _OPENMP
    }
#endif

        diff /= ntot;
        delta = fabs(diff - delta);
        delta_rel = delta / diff;

        if(opts.force_bnd) { // force initial boundary values
            // delineate boundaries
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res,maskptr,bnd_vals) shared(slice_rle_n,slice_rle_start,slice_rle_end,dims) schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 0) {
                slice[ix] = bnd_vals[maskptr[k][im]];
            }
        }
    }
}
        } else if(delta_rel < opts.delta_bnd) { // update boundary values for smoothness
            double bnd_mean[1 + NREGION];
            memset(bnd_mean,0x0,(1 + NREGION) * sizeof(*nbnd)); // init
            bnd_delta = bnd_diff;
            bnd_diff = 0.0;

#ifdef _OPENMP
#pragma omp parallel default(none) firstprivate(res,mask,maskvals,maskptr,maskvalsptr) shared(slice_rle_n,slice_rle_start,slice_rle_end,dims,bnd_mean,ntot,NREGION)
    {
#endif
        // smoothen
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 1 && msk < NREGION) {
                maskvalsptr[k][im] = FROMFLOAT(convolve(dims,res,(size_t)ix + k * NXY));
            }
        }
    }
}

#ifdef _OPENMP
#pragma omp for reduction(+:bnd_mean[:1 + NREGION]) schedule(static)
#endif
            for(size_t i = 0; i < ntot; ++i)
                bnd_mean[mask[i]] += TOFLOAT(maskvals[i]);
#ifdef _OPENMP
    }
#endif

            for(size_t i = 2; i < NREGION; ++i) { // except L1 and WM boundaries
                bnd_mean[i] /= nbnd[i];
                bnd_diff += fabs(bnd_mean[i] - TOFLOAT(bnd_vals[i]));
                bnd_vals[i] = FROMFLOAT(bnd_mean[i]);
            }
            bnd_diff /= NREGION - 2;
            bnd_delta = fabs(bnd_diff - bnd_delta);
            bnd_delta_rel = bnd_delta / bnd_diff;

            PRINTBND("updated boundary values: ");
            // delineate boundaries
#ifdef _OPENMP
#pragma omp parallel for default(none) firstprivate(res,maskptr,bnd_vals) shared(slice_rle_n,slice_rle_start,slice_rle_end,dims) schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 0) {
                slice[ix] = bnd_vals[maskptr[k][im]];
            }
        }
    }
}
        }

        clock_gettime(CLOCK_MONOTONIC,&ts_end);

        long msec_begin = ts_begin.tv_sec * 1000 + ts_begin.tv_nsec / 1000000L;
        long msec_end = ts_end.tv_sec * 1000 + ts_end.tv_nsec / 1000000L;

        info("iteration %zu done in %gs, diff = %g (%g), bnd_diff = %g (%g)",
             iter,(msec_end - msec_begin) / 1000.0,diff,delta_rel,bnd_diff,bnd_delta_rel);

        if((opts.force_bnd || bnd_delta > 0)
            && delta < opts.delta_tol && bnd_delta < opts.delta_tol) {
            info0("converged!");
            break;
        }
    }

    if(!opts.force_bnd) { // final boundary smoothing
#ifdef _OPENMP
#pragma omp parallel default(none) firstprivate(res,maskptr,maskvalsptr,NREGION) shared(diff,slice_rle_n,slice_rle_start,slice_rle_end,dims)
    {
#endif
        // smoothen
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 1 && msk < NREGION) {
                maskvalsptr[k][im] = FROMFLOAT(convolve(dims,res,(size_t)ix + k * NXY));
            }
        }
    }
}
        // write back to array buffer
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
for(size_t k = 1; k < NZ - 1; ++k) {
    data_t *slice = &res[k * NXY]; (void)slice;
    for(size_t i = 0, im = 0; i < slice_rle_n[k]; ++i) {
        for(size_t ix = slice_rle_start[k][i]; ix <= slice_rle_end[k][i]; ++ix, ++im) {
            uint8_t msk = maskptr[k][im];
            if(msk > 1 && msk < NREGION) {
                slice[ix] = maskvalsptr[k][im];
            }
        }
    }
}
#ifdef _OPENMP
    }
#endif
    }

    PRINTBND("final boundary values: ");
    { // write final boundary values
        char path[PATH_MAX]; snprintf(path,sizeof(path),"%s.bnd",opts.out_file);
        info("writing final boundary values to %s",path);
        errno = 0;
        FILE *fp = fopen(path,"wb");
        if(fp == NULL) ERRFATAL("fopen[w]","%s",path);
        if(fwrite(&bnd_vals[1],sizeof(*bnd_vals),NREGION,fp) != NREGION)
            error("fwrite failed: %zu items",(size_t)NREGION);
        fclose(fp);
    }

    bool couldsave = false;
    // write final solution
    for(size_t try = 1; !couldsave && try <= MAX_RETRY_SAVE; ++try) {
        info("writing final solution to %s (try %zu)",opts.out_file,try);
        if(savem == SAVE_PARALLEL) {
            errno = 0;
            int fd = creat(opts.out_file,CREAT_MASK);
            if(-1 == fd) {
                error("creat failed: %s, retrying in 5 minutes",opts.out_file);
                goto loop;
            }
            size_t nerr = 0;
#ifdef _OPENMP
#pragma omp parallel for reduction(+:nerr)
#endif
            for(size_t i = 0; i < NZ; ++i) {
                size_t count = NXY * sizeof(*res);
                off_t offset = (off_t)i * count;
                ssize_t ret = pwrite(fd,&res[i * NXY],count,offset);
                if(ret == -1 || (size_t)ret != count) ++nerr;
            }
            if(nerr > 0)
                error("pwrite failed: %zu times with %zu items, retrying in 5 minutes",nerr,NVOX);
            else couldsave = true;
            close(fd);
        } else {
            errno = 0;
            FILE *fp = fopen(opts.out_file,"wb");
            if(NULL == fp) {
                error("fopen[w] failed: %s, retrying in 5 minutes",opts.out_file);
                goto loop;
            }
            if(fwrite(res,sizeof(*res),NVOX,fp) != NVOX)
                error("fwrite failed: %zu items, retrying in 5 minutes",NVOX);
            else couldsave = true;
            fclose(fp);
        }
loop:
        if(!couldsave) sleep(300);
        if(try > 10) savem = SAVE_SEQUENTIAL; // fallback to sequential
    }

    /* cleanup */
    if(loadm == LOAD_MMAP) { // munmap
        errno = 0;
        if(munmap(res_map,NVOX * sizeof(*res)) == -1) PERROR("munmap");
        close(fdmap);
    } else { free(res); (res) = NULL; }
    { free(maskvals); (maskvals) = NULL; }
    { free(mask); (mask) = NULL; }

    for(size_t k = 1; k < NZ - 1; ++k) {
        free(slice_rle_start[k]);
        free(slice_rle_end[k]);
    }

    return !couldsave;
}
