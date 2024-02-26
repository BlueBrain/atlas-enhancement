BIN_PREFIX = $(PREFIX)/bin
LIB_PREFIX = $(PREFIX)/lib
INC_PREFIX = $(PREFIX)/include
MOD_PREFIX = $(PREFIX)/include

CC := gcc -std=gnu99
CMOD := cmod
RM := rm -f
INSTALL := install
YACC := bison
YFLAGS := -by --defines=parser.h
LEX := flex
LFLAGS := --header-file=scanner.h
WFLAGS := -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-misleading-indentation -Wno-unknown-warning-option
OPTIM_FLAGS ?= -O2
OMP_FLAGS ?= -fopenmp
_CFLAGS := $(CFLAGS)
CFLAGS = $(_CFLAGS) $(WFLAGS) $(OPTIM_FLAGS) 
_CPPFLAGS := $(CPPFLAGS)
CPPFLAGS = $(_CPPFLAGS) $(INCLUDES) $(OMP_FLAGS)
LDLIBS := -lgomp
_LDFLAGS := $(LDFLAGS)
LDFLAGS = $(_LDFLAGS) -Wl,--as-needed
