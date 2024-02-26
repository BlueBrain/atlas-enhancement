## External programs
# Basic utilities
PARALLEL ?= parallel
SPLIT ?= split
JOIN ?= join
CALC ?= calc
UNIQ ?= uniq
# Required software
HEXGRID ?= hexgrid

## Internal programs
FLATPATH_HEXVOL := $(BBMAKE_ROOT)/structural/hexvol.py
FLATPATH_HEXASSIGN := $(BBMAKE_ROOT)/structural/hexassign_2d.sh

.PHONY: hexgrid flatmap-hexplot
hexgrid: hexgrid.nrrd radii.nrrd
flatmap-hexplot: flatmap_hexcake.png boundary_hex.png

# Generate radii image
radii.nrrd: flathexes.txt
	$(AWK) '{ print $$1,$$2,$$3,$$7 }' $< > $<.tmp
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_HEIGHTS) $(ATLAS_REGIONS) $<.tmp $@
	$(RM) $<.tmp

# Hex cake plot
flatmap_hexcake.png: flathexes.txt
	echo "\
		set term pngcairo size 25in,10in; \
		set out '$@'; \
		unset key; \
		set view $(PLOT_VIEW); \
		set view noequal; \
		set xr $(XRANGE); \
		set yr $(YRANGE); \
		set hidden3d; \
		splot '$<' u 1:2:3:6 w p pt 7 ps 0.5 lc var; \
		" | gnuplot

# Generate hexagonal column image
HEXGRID_WEIGHTS ?= 1 1
HEXGRID_WEIGHTS_INV := $(foreach _,$(HEXGRID_WEIGHTS),$(shell $(CALC) -pd <<< '1.0 / $_'))

hexgrid.nrrd: flathexes.txt
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_HEXVOL) $(ATLAS_REGIONS) $< $@

boundary_hex.png: boundary_hex.txt
	echo "\
		set term pngcairo size 25in,10in; \
		set out '$@'; \
		unset key; \
		set view $(PLOT_VIEW); \
		set view noequal; \
		set xr $(XRANGE); \
		set yr $(YRANGE); \
		set hidden3d; \
		splot '$<' u 1:2:3:6 w p pt 7 ps 0.8 lc var; \
		" | gnuplot

.DELETE_ON_ERROR: boundary_hex.txt
boundary_hex.txt: boundary_vox.xyz flathexes.txt
	awk 'BEGIN { while(getline <ARGV[2]> 0) line[$$1_$$2_$$3] = $$0; ARGC-- } \
		       { if(line[$$1_$$2_$$3] == "") { print "NOT FOUND "$$1" "$$2" "$$3 } print line[$$1_$$2_$$3] }' $^ \
		| grep -v NOTFOUND \
		> $@

.DELETE_ON_ERROR: flathexes.txt
flathexes.txt: NPART := 72
flathexes.txt: flatmap.txt hexgrid.txt
	$(SPLIT) -n l/$(NPART) $< fltmp
	ls fltmp* | \
		$(PARALLEL) --lb -j$(NPART) $(FLATPATH_HEXASSIGN) {} $(HEXGRID_WEIGHTS_INV) $(filter-out $<,$^) \
		> $@
	$(RM) fltmp*

# Set these parameters to cover the whole unit square
.DELETE_ON_ERROR: hexgrid.txt
hexgrid.txt: HEXGRID_RADIUS ?= 0.07
hexgrid.txt: HEXGRID_NITER ?= 11
hexgrid.txt: HEXGRID_CENTER_X ?= 0.5
hexgrid.txt: HEXGRID_CENTER_Y ?= 0.5
hexgrid.txt:
	$(HEXGRID) -n$(HEXGRID_NITER) -r $(HEXGRID_RADIUS) \
		-x $(HEXGRID_CENTER_X) -y $(HEXGRID_CENTER_Y) \
		-u 1.0 -v 0.0 -w 0.0 \
		-a $(word 1,$(HEXGRID_WEIGHTS)) -b $(word 2,$(HEXGRID_WEIGHTS)) \
	| awk '$$1 >= 0.0 && $$1 <= 1.0 && $$2 >= 0.0 && $$2 <= 1.0' \
	> $@
