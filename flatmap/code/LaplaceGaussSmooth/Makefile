-include /usr/share/cmod/cmod.mk

#CC := clang -std=c99# clang gives better performance here
CC := gcc -std=c99
CFLAGS := -Wall -Wextra -Werror -pedantic #-fopt-info
ifdef DEBUG
CFLAGS += -gdwarf-2 -O3 -fopenmp -march=native
else
CFLAGS += -O3 -fopenmp -march=native #-ffast-math
endif
CPPFLAGS += -D_DEFAULT_SOURCE

all: lapgasm3d_full

lapgasm3d_full: LDLIBS += -lgomp -lm -lz
lapgasm3d_full: | lapgasm3d_full.c
