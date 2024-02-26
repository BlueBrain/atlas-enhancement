# Include local config
-include config.mk

# Command to load voxcell, numpy, scipy and matplotlib
LOAD_VOXCELL ?= module load unstable brainbuilder

# ATLAS must be non-empty
ifeq (,$(ATLAS))
$(error 'Please specify ATLAS')
endif

# PARENT_REGION must be non-empty
ifeq (,$(PARENT_REGION))
$(error 'Please specify PARENT_REGION')
endif

.ONESHELL:

## External programs
# Basic commands
MV ?= mv
LN ?= ln
RM ?= rm -f
# Basic utilities
AWK ?= gawk
SED ?= sed
ECHO ?= echo
SORT ?= sort
MKDIR ?= mkdir
PASTE ?= paste
JOIN ?= join
GZIP ?= gzip
TR ?= tr
CUT ?= cut
# Required software
PYTHON3 ?= python3

# FLATPATH_ROOT is the installation directory
## Internal programs
FLATPATH ?= $(FLATPATH_ROOT)/flatpath
FLATPATH_BBOX := $(FLATPATH_ROOT)/bbox.py
FLATPATH_BOUNDARY := $(FLATPATH_ROOT)/boundary.py
FLATPATH_MAKE_FLATMAP := $(FLATPATH_ROOT)/make_flatmap.py
FLATPATH_DOWNSAMPLE := $(FLATPATH_ROOT)/downsample.py
FLATPATH_MAPFLATMAP := $(FLATPATH_ROOT)/mapflatmap.py
FLATPATH_PATHLEN := $(FLATPATH_ROOT)/pathlen.py
## Ellipses stuff
FLATPATH_GETPOINTS := $(FLATPATH_ROOT)/getpoints.py
FLATPATH_APPRELLIPSE := $(FLATPATH_ROOT)/ApprEllipse.Release
# CGAL stuff
FLATTEN_SURFACE ?= $(FLATPATH_ROOT)/surf_cgal/build/Flatten_Authalic
#FLATTEN_SURFACE ?= $(FLATPATH_ROOT)/surf_cgal/Flatten_Authalic_Iterative
RECONSTRUCT_SURFACE ?= $(FLATPATH_ROOT)/surf_cgal/build/Reconstruct_PCAlpha
#RECONSTRUCT_SURFACE ?= $(FLATPATH_ROOT)/surf_cgal/Reconstruct_PCAdv
NEAREST_SURFACE := $(FLATPATH_ROOT)/surf_cgal/build/Nearest_KNN

## Atlas files (may be overriden by config)
ATLAS_HIERARCHY ?= $(ATLAS)/hierarchy.json
ATLAS_REGIONS ?= $(ATLAS)/brain_regions.nrrd
ATLAS_ORIENTATION ?= $(ATLAS)/orientation.nrrd

all: $(ATLAS_MASK) flatmap.nrrd

.PHONY: matrix-targets matrix-targets-gz atlas-derived-targets

MATRIX_TARGETS := normal_x.bin normal_y.bin normal_z.bin layers.bin
matrix-targets: $(MATRIX_TARGETS)

MATRIX_TARGETS_GZ ?= $(addsuffix .gz,$(MATRIX_TARGETS))
matrix-targets-gz: $(MATRIX_TARGETS_GZ)

ATLAS_DERIVED_TARGETS ?= $(MATRIX_TARGETS_GZ) dimensions.txt
atlas-derived-targets: $(ATLAS_DERIVED_TARGETS)
$(ATLAS_DERIVED_TARGETS): $(ATLAS_HIERARCHY) $(ATLAS_REGIONS) $(ATLAS_ORIENTATION)
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_BBOX) $^ $(PARENT_REGION) $(ATLAS_MASK)
	$(GZIP) -f $(MATRIX_TARGETS)

%/:
	$(MKDIR) -p $@

# print streamlines to files (for inspection)
.PHONY: sample-streamlines
sample-streamlines: N ?= 200# number of streamlines to output
sample-streamlines: SEED ?= 2997# random seed
sample-streamlines: $(ATLAS_DERIVED_TARGETS) | streamlines/
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	$(FLATPATH) \
		-x $(word 1,$^) -y $(word 2,$^) -z $(word 3,$^) -l $(word 4,$^) \
		-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
		-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
		-a $${off[0]} -b $${off[1]} -c $${off[2]} \
		--first 0 --last $N --randomize --seed $(SEED) -p --full-only \
		$(FLATPATH_EXTRA) > streamlines/dots.txt
	cd streamlines && \
		wc -l streamline*.xyz \
			| $(AWK) 'BEGIN { while(getline < ARGV[ARGC - 1] > 0) tot[$$2] = $$1; ARGC-- } \
			       FNR == 1 { printf "\n\n" } \
				   { print $$0,FNR / tot[FILENAME] };' streamline*.xyz - \
			> all_streamlines.txt

# Extract streamline intersections with boundary
# Split in blocks to reduce memory usage per run
BLOCKSIZE ?= 40000# Set according to available memory, higher -> faster

SRUN := srun
SLURM_ACCOUNT := proj83
SLURM_PARTITION := prod
SLURM_TIMELIMIT := 08:00:00

ifdef STEPS
SLURM_STEPS := -r{%} -N1
endif

dots: $(DOTS_FILE)
DOTS_FILE := dots.txt
$(DOTS_FILE): NVOX = $(shell head -1 $(word 5,$^))
$(DOTS_FILE): NBLOCK = $(shell expr $(NVOX) / $(BLOCKSIZE) + \( $(NVOX) % $(BLOCKSIZE) \> 0 \))
$(DOTS_FILE): FIRST := $$first
$(DOTS_FILE): LAST := $$last
$(DOTS_FILE): $(ATLAS_DERIVED_TARGETS)
	if [ -z "$(LAYER_BOUNDARY)" ]; then $(ECHO) "Please set LAYER_BOUNDARY"; exit; fi
	$(RM) $@ $@.err
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	for i in `seq 1 $(NBLOCK)`; do
		first=`expr \( $$i - 1 \) \* $(BLOCKSIZE)`
		last=`expr $$i \* $(BLOCKSIZE)`
		$(FLATPATH) \
			-x $(word 1,$^) -y $(word 2,$^) -z $(word 3,$^) -l $(word 4,$^) \
			-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
			-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
			-a $${off[0]} -b $${off[1]} -c $${off[2]} \
			--first $(FIRST) --last $(LAST) --full \
			-L $(word 1,$(LAYER_BOUNDARY)) -s $(word 2,$(LAYER_BOUNDARY)) \
			$(FLATPATH_EXTRA) >> $@ 2>> $@.err
	done

ifdef NJOB
dots-split: NVOX = $(shell head -1 $<)
dots-split: PREFIX := dots
dots-split: UPREFIX := DOTS
dots-split: dimensions.txt | dots_parts/
	seq 0 $(BLOCKSIZE) $(NVOX) \
		| $(AWK) '{ print $$1,$$1 + $(BLOCKSIZE) }' \
		| $(PARALLEL) --lb -j$(NJOB) -C' ' \
			$(SRUN) --mpi=none -A$(SLURM_ACCOUNT) -p$(SLURM_PARTITION) -t$(SLURM_TIMELIMIT) $(SLURM_STEPS) --exclusive \
			$(MAKE) dots NBLOCK=1 FIRST={1} LAST={2} $(UPREFIX)_FILE=dots_parts/$(PREFIX)_{\#}.txt
	cat dots_parts/$(PREFIX)_*.txt > $(PREFIX).txt
endif # NJOB

# Compute height (distance from L6) or depth (distance from L1) along streamlines
heightdots: $(HEIGHT_FILE)
depthdots: $(DEPTH_FILE)
HEIGHT_FILE := height.txt
DEPTH_FILE := depth.txt
$(DEPTH_FILE): NSTEP_PATH := 1000
$(DEPTH_FILE): COMMAND := depth
$(HEIGHT_FILE): NSTEP_PATH := 2000
$(HEIGHT_FILE): COMMAND := height
$(HEIGHT_FILE) $(DEPTH_FILE): NVOX = $(shell head -1 $(word 5,$^))
$(HEIGHT_FILE) $(DEPTH_FILE): NBLOCK = $(shell expr $(NVOX) / $(BLOCKSIZE) + \( $(NVOX) % $(BLOCKSIZE) \> 0 \))
$(HEIGHT_FILE) $(DEPTH_FILE): FIRST := $$first
$(HEIGHT_FILE) $(DEPTH_FILE): LAST := $$last
$(HEIGHT_FILE) $(DEPTH_FILE): %.txt: $(ATLAS_DERIVED_TARGETS)
	$(RM) $@ $@.err
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	for i in `seq 1 $(NBLOCK)`; do
		first=`expr \( $$i - 1 \) \* $(BLOCKSIZE)`
		last=`expr $$i \* $(BLOCKSIZE)`
		$(FLATPATH) \
			-x $(word 1,$^) -y $(word 2,$^) -z $(word 3,$^) -l $(word 4,$^) \
			-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
			-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
			-a $${off[0]} -b $${off[1]} -c $${off[2]} \
			--first $(FIRST) --last $(LAST) --$(COMMAND) -N $(NSTEP_PATH) $(FLATPATH_EXTRA) \
			>> $@ 2>> $@.err
	done

ifdef NJOB
height-split depth-split: NVOX = $(shell head -1 $<)
depth-split: PREFIX := depth
depth-split: UPREFIX := DEPTH
depth-split: | depth_parts/
height-split: PREFIX := height
height-split: UPREFIX := HEIGHT
height-split: | height_parts/
height-split depth-split: dimensions.txt
	seq 0 $(BLOCKSIZE) $(NVOX) \
		| $(AWK) '{ print $$1,$$1 + $(BLOCKSIZE) }' \
		| $(PARALLEL) --lb -j$(NJOB) -C' ' \
			$(SRUN) --mpi=none -A$(SLURM_ACCOUNT) -p$(SLURM_PARTITION) -t$(SLURM_TIMELIMIT) $(SLURM_STEPS) --exclusive \
			$(MAKE) $(PREFIX)dots NBLOCK=1 FIRST={1} LAST={2} $(UPREFIX)_FILE=$(PREFIX)_parts/$(PREFIX)_{\#}.txt
	cat $(PREFIX)_parts/$(PREFIX)_*.txt > $(PREFIX).txt
endif # NJOB

# Filter not-nan voxels and split indices and intersections
dots_notnan.xyz vox_notnan.xyz: dots.txt
	$(AWK) '$$4 != "nan";' $< \
		| $(AWK) '{ print $$1,$$2,$$3 > "vox_notnan.xyz"; \
		            print $$4,$$5,$$6 > "dots_notnan.xyz" }'

# Filter nan voxels and split indices and intersections
dots_nan.xyz vox_nan.xyz: dots.txt
	$(AWK) '$$4 == "nan";' $< \
		| $(AWK) '{ print $$1,$$2,$$3 > "vox_nan.xyz"; \
		            print $$4,$$5,$$6 > "dots_nan.xyz" }'

# Filter not-nan voxels
height_notnan.txt depth_notnan.txt: %_notnan.txt: %.txt
	grep -v nan $< > $@

# Generate height/depth volume
.PHONY: height depth
height depth: %: %.nrrd
height.nrrd depth.nrrd: %.nrrd: %_notnan.txt
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_PATHLEN) $(ATLAS_REGIONS) $< $@

# Extract boundary from atlas
boundary.xyz boundary.nrrd: $(ATLAS_HIERARCHY) $(ATLAS_REGIONS)
	if [ -z "$(LAYER_BOUNDARY)" ]; then $(ECHO) "Please set LAYER_BOUNDARY"; exit; fi
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_BOUNDARY) $^ $(LAYER_BOUNDARY) $@ $(PARENT_REGION) $(ATLAS_MASK)

# Reconstruct surface
surf.off: boundary.xyz
	$(RECONSTRUCT_SURFACE) $< $@ $(RECO_PARAMS)

# Refine mesh using GMSH
GMSH := gmsh
OFF2PLY := $(FLATPATH_ROOT)/off2ply.awk
PLY2OFF := $(FLATPATH_ROOT)/ply2off.awk

ifdef IN
ifdef OUT
off2ply:
	$(AWK) -f $(OFF2PLY) $(IN) > $(OUT)

ply2off:
	$(AWK) -f $(PLY2OFF) $(IN) > $(OUT)
else
off2ply ply2off:
	@echo '$@: Please set IN and OUT'; exit 0
endif # ifdef OUT
else
off2ply ply2off:
	@echo '$@: Please set IN and OUT'; exit 0
endif # ifdef IN

REFINE_SURFACE ?= 3# thrice, to reduce nearest projection error
ifeq (0,$(REFINE_SURFACE)) # REFINE_SURFACE == 0

FLAT_PREREQ := surf.off

else # REFINE_SURFACE > 0

FLAT_PREREQ := surf_refined.off

surf_refined.off: surf.off
	$(MAKE) off2ply IN=$< OUT=$$$$.ply2; \
	for i in `seq 1 $(REFINE_SURFACE)`; do \
		$(GMSH) $$$$.ply2 -2 -refine -format ply2 -o $$$$.ply2; \
	done; \
	$(MAKE) ply2off IN=$$$$.ply2 OUT=$@; \
	$(RM) $$$$.ply2

surf_refined.xyz: surf_refined.off
	$(AWK) 'NR == 2 { nv=$$1; } NR > 2 && NR <= 2 + nv;' $< > $@

surf_refined_normals.xyz: $(ATLAS_ORIENTATION) surf_refined.xyz
	$(LOAD_VOXCELL)
	$(PYTHON3) $(BBMAKE_ROOT)/structural/normals.py $^ $@

surf_refined_xyzuvw.txt: surf_refined.xyz surf_refined_normals.xyz
	paste $^ | tr '\t' ' ' > $@

surf_refined_regions.txt: $(ATLAS_REGIONS) surf_refined.xyz
	$(LOAD_VOXCELL)
	$(PYTHON3) $(BBMAKE_ROOT)/structural/regions.py $^ $@

surf_refined_regions_acr.txt: $(ATLAS_HIERARCHY) surf_refined_regions.txt
	$(LOAD_VOXCELL)
	$(PYTHON3) $(BBMAKE_ROOT)/structural/regions_acr.py $^ $@

surf_refined_regions_reg.txt: surf_refined_regions_acr.txt
	$(SED) 's/;.*//' $< > $@

surf_refined_regions_count.txt: surf_refined_regions_reg.txt
	$(AWK) '{ n[$$1]++; k[$$1] = $$1 } END { for(r in k) print k[r],n[r] }' $< \
		| $(SORT) > $@

surf_refined_regions_num.txt: surf_refined_regions_reg.txt surf_refined_regions_count.txt
	$(AWK) 'BEGIN { n = 0; while(getline < ARGV[2] > 0) { num[$$1] = n++ }; ARGC-- }\
		          { print num[$$1] }' $^ > $@

endif # REFINE_SURFACE

flat.off: FLATTEN_EXTRA ?= 0
flat.off: $(FLAT_PREREQ)
	$(FLATTEN_SURFACE) $< $@ $(FLATTEN_EXTRA) 2>&1 | tee flat.log

flat.xyz: flat.off
	$(AWK) 'NR == 2 { nv=$$1; } NR > 2 && NR <= 2 + nv;' $< > $@

surf.xyz: $(FLAT_PREREQ)
	$(AWK) 'NR == 2 { nv=$$1; } NR > 2 && NR <= 2 + nv;' $< > $@

# Find nearest points in surface
.DELETE_ON_ERROR: nearest.txt
nearest.txt: $(FLAT_PREREQ) dots_notnan.xyz
	$(NEAREST_SURFACE) $^ > $@

nearest-error: nearest.txt
	$(AWK) 'BEGIN { min = 1E32; max = -1E32; } \
		          { if($$5 < min) min = $$5; if($$5 > max) max = $$5; } \
	          END { printf "info: Minimum projection error: %s\n",min; \
		            printf "info: Maximum projection error: %s\n",max; }' $< >/dev/stderr

# Produce flatmap
# Here nearest.txt has the same number of lines as vox_notnan.xyz
.DELETE_ON_ERROR: flatmap.txt
flatmap.txt: nearest.txt vox_notnan.xyz flat.xyz
	$(AWK) '{ print $$1 }' $< | $(SED) 's/^v//' \
		| $(PASTE) - $(word 2,$^) \
		| $(AWK) '\
			BEGIN { i = 0; while(getline <ARGV[2] > 0) { fx[i] = $$1; fy[i] = $$2; i++ } ARGC-- } \
			{ print $$2,$$3,$$4,fx[$$1],fy[$$1] } \
			' - $(word 3,$^) > $@
	$(AWK) '$$4 < 0 || $$4 > 1 || $$5 < 0 || $$5 > 1 { print "bad point found: ",$$4,$$5,NR; exit 1 }' $@

# Facility for flat-mapping arbitrary points
ifdef POINTS_IN

ifdef POINTS_OUT
.PHONY: flatpoints
flatpoints: $(POINTS_OUT)
	$(RM) $<.d.tmp $<.v.tmp

$(POINTS_OUT).d.tmp: $(ATLAS_DERIVED_TARGETS)
	if [ -z "$(LAYER_BOUNDARY)" ]; then $(ECHO) "Please set LAYER_BOUNDARY"; exit; fi
	$(RM) $@
	num=`wc -l < $(POINTS_IN)` # number of input points
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	for i in `seq 1 $$(expr $$num / $(BLOCKSIZE) + \( $$num % $(BLOCKSIZE) \> 0 \))`; do
		first=`expr \( $$i - 1 \) \* $(BLOCKSIZE)`
		last=`expr $$i \* $(BLOCKSIZE)`
		$(FLATPATH) \
			-x $(word 1,$^) -y $(word 2,$^) -z $(word 3,$^) -l $(word 4,$^) \
			-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
			-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
			-a $${off[0]} -b $${off[1]} -c $${off[2]} \
			--first $$first --last $$last \
			-L $(word 1,$(LAYER_BOUNDARY)) -s $(word 2,$(LAYER_BOUNDARY)) \
			--full -f $(POINTS_IN) $(FLATPATH_FLAGS) | grep -v nan >> $@
	done

$(POINTS_OUT).v.tmp: $(FLAT_PREREQ) $(POINTS_OUT).d.tmp
	$(CUT) -d' ' -f 2-4 $(word 2,$^) > $@.tmp
	$(NEAREST_SURFACE) $< $@.tmp | $(CUT) -d' ' -f1 | $(TR) -d 'v' | $(PASTE) - $(word 2,$^) > $@
	$(RM) $@.tmp

$(POINTS_OUT): flat.xyz $(POINTS_OUT).v.tmp
	$(AWK) '{ printf "%012d" OFS "%s\n",$$1,$$2 }' $(word 2,$^) | $(SORT) > $@.a.tmp
	$(AWK) '{ printf "%012d" OFS "%s" OFS "%s\n",NR - 1,$$1,$$2 }' $< | $(JOIN) - $@.a.tmp > $@.b.tmp
	$(AWK) '{ printf "%012d" OFS "%s\n",NR - 1,$$0 }' $(POINTS_IN) > $@.c.tmp
	$(AWK) '{ printf "%012d" OFS "%s" OFS "%s\n",$$4,$$2,$$3 }' $@.b.tmp \
		| $(SORT) | $(JOIN) - $@.c.tmp | $(CUT) -d' ' -f1-3 > $@
	$(RM) $@.a.tmp $@.b.tmp $@.c.tmp
endif # POINTS_OUT

ifdef DEPTHS_OUT
.PHONY: flatdepths
flatdepths: $(DEPTHS_OUT)
$(DEPTHS_OUT): $(ATLAS_DERIVED_TARGETS)
	$(RM) $@
	num=`wc -l < $(POINTS_IN)` # number of input points
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	for i in `seq 1 $$(expr $$num / $(BLOCKSIZE) + \( $$num % $(BLOCKSIZE) \> 0 \))`; do
		first=`expr \( $$i - 1 \) \* $(BLOCKSIZE)`
		last=`expr $$i \* $(BLOCKSIZE)`
		$(FLATPATH) \
			-x $(word 1,$^) -y $(word 2,$^) -z $(word 3,$^) -l $(word 4,$^) \
			-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
			-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
			-a $${off[0]} -b $${off[1]} -c $${off[2]} \
			--first $$first --last $$last --depth -f $(POINTS_IN) $(FLATPATH_FLAGS) >> $@
	done
endif # DEPTHS_OUT

endif

# Produce voxelized flatmap (float)
flatmap_float.nrrd: $(ATLAS_REGIONS) flatmap.txt
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_MAKE_FLATMAP) $^ $@ -1.0 $(FLATMAP_EXTRA)

# Produce voxelized flatmap (discrete)
flatmap.nrrd: $(ATLAS_REGIONS) flatmap.txt
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_MAKE_FLATMAP) $^ $@ $(HIST_PIXELS) $(FLATMAP_EXTRA)

.PHONY: clean
clean:
	$(RM) $(ATLAS_DERIVED_TARGETS) dots.txt height.txt depth.txt
	$(RM) dots_notnan.xyz vox_notnan.xyz
	$(RM) dots_nan.xyz vox_nan.xyz
	$(RM) height_notnan.txt depth_notnan.txt height.nrrd depth.nrrd
	$(RM) boundary.xyz boundary.nrrd
	$(RM) $(FLAT_PREREQ) flat.off flat.xyz
	$(RM) nearest.txt flatmap.txt flatmap.nrrd

# Extract triangles for projection mapping
include $(FLATPATH_ROOT)/flatmap_triangles.mk

# Generate reporting plots
include $(FLATPATH_ROOT)/flatmap_plot.mk

# Hexgrid
include $(FLATPATH_ROOT)/flatmap_hexgrid.mk
