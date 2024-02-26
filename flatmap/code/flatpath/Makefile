include common_header.mk

.ONESHELL:

CMOD_STDLIB ?= /usr/include
CMOD_INCLUDES += -I$(CMOD_STDLIB)
LDLIBS += -lz -lm `gsl-config --libs`
LDFLAGS += -L$(HOME)/lib # use local zlib (for gzfread)
OBJS := interp_field.o streamline.o
BIN_TARGETS := flatpath

ifneq (,$(DEBUG))
CFLAGS += -DDEBUG=$(DEBUG)
endif

.INTERMEDIATE: $(OBJS) $(OBJS:.o=.c) main

all: flatpath

main: $(OBJS)

main.c: interp_field.h

ifeq (1,$(DEBUG))
flatpath: CPPFLAGS += -DDEBUG=1
flatpath: CFLAGS += -O0 -ggdb3
endif

flatpath: CFLAGS += -march=native -mtune=native
flatpath: main
	mv $< $@

flatpath-test: test/dots.txt test/height.txt test/depth.txt
.DELETE_ON_ERROR: test/dots.txt test/depth.txt test/height.txt
test/depth.txt: FLATPATH_EXTRA := -D
test/height.txt: FLATPATH_EXTRA := -H
test/dots.txt test/depth.txt test/height.txt: flatpath test/orientation_x.bin.gz test/orientation_y.bin.gz test/orientation_z.bin.gz test/relative_depth.bin.gz test/dimensions.txt
	dim=(`sed '2q;d' $(word 6,$^)`) # atlas dimensions
	vox=(`sed '3q;d' $(word 6,$^)`) # voxel size
	off=(`sed '4q;d' $(word 6,$^)`) # offset
	$(VALGRIND) ./$< \
		-x $(word 2,$^) -y $(word 3,$^) -z $(word 4,$^) -l $(word 5,$^) \
		-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
		-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
		-a $${off[0]} -b $${off[1]} -c $${off[2]} \
		--first 0 --last 1000 --full-only $(FLATPATH_EXTRA) \
		--maxiter 5000 -N 2000 -h 0.5 --epsilon-zero 0.5 --epsilon-one 0.5 \
	> $@
	diff $@ $@.ref

include common_footer.mk
