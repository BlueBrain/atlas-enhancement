include config.mk

# functions
wrap = $(addprefix $1,$(addsuffix $3,$2))
calc = $(shell calc -pd <<< '$1')

# commands
PYTHON := python3
GNUPLOT := gnuplot
MAGICK := magick
PARALLEL := parallel
JQ := jq

# venv with voxcell
LOAD_ENV := module purge; module load archive/2023-04 brainbuilder

# variables
UID := $(shell id -u)
TMPDIR := /var/run/user/$(UID)
SCRATCH := $(TMPDIR)
FLATPIX = $(shell grep HIST_PIXELS $(CONFIG) | cut -d' ' -f3)
FLATPIX2 := $(call calc,2 * $(FLATPIX))

# cached datasets
CACHED_FLATMAP := $(TMPDIR)/flatmap_morphviz.nrrd

ifdef JSONFILE
NEURON_ID := $(subst .json,,$(notdir $(JSONFILE)))
BASE := $(SCRATCH)/$(NEURON_ID)

all: fullplot

$(BASE)/:
	mkdir -p $@

$(BASE)/dots_soma.xyz: | $(BASE)/
	$(JQ) '.neuron.soma | .x, .y, .z' $(JSONFILE) \
		| pr -at3 -s' ' \
		> $@

$(BASE)/dots_axon.xyz: | $(BASE)/
	$(JQ) '.neuron.axon[] | .x, .y, .z' $(JSONFILE) \
		| pr -at3 -s' ' \
		> $@

.PRECIOUS: $(BASE)/flatdots_axon.xy $(BASE)/flatdots_soma.xy
$(BASE)/flat%.xy: $(BASE)/%.xyz | $(CACHED_FLATMAP)
	module purge; module load unstable brainbuilder; \
	echo -e "\
import voxcell as vc                               \n\
import numpy as np                                 \n\
fmap = vc.VoxelData.load_nrrd('$(CACHED_FLATMAP)') \n\
coor = np.loadtxt('$<')                            \n\
flat = fmap.lookup(coor, outer_value=np.array([-2,-2]))\n\
if len(flat.shape) == 1:                           \n\
    flat = flat[None,:]                            \n\
np.savetxt('$@',flat)                              \n\
" | $(PYTHON)

$(BASE)/flatdots_soma.png: PLOTSTYLE := w p pt 5 ps 0.5 lc rgbcolor 'white'
$(BASE)/flatdots_axon.png: PLOTSTYLE := w d lc rgbcolor 'black'

PLOT_W := $(FLATPIX2)
PLOT_H := $(FLATPIX)
$(BASE)/flat%.png: $(BASE)/flat%.xy
	echo -e "\
set view equal xy                                       \n\
set xr [0:2]                                            \n\
set yr [0:1]                                            \n\
unset key; unset tics; unset border                     \n\
set margin 0,0,0,0                                      \n\
set term pngcairo transparent size $(PLOT_W),$(PLOT_H)  \n\
set out '$@'                                            \n\
plot '$<' $(PLOTSTYLE)                                  \n\
" | $(GNUPLOT)

REGSOMA := $(shell jq '.neuron.soma.allenId as $$id | .neuron.allenInformation[] | select(.allenId == $$id) | .acronym' $(JSONFILE) | tr -d '"' | tr '/' '_')

fullplot: $(NEURON_ID)_$(REGSOMA).png
$(NEURON_ID)_$(REGSOMA).png: $(BASE)/flatdots_axon.png $(BASE)/flatdots_soma.png
	$(MAGICK) composite $< $(BACKGROUND) - \
		| $(MAGICK) composite $(word 2,$^) - $@

else # JSONFILE

ISOCORTEX_ID := 315# from Allen CCFv3 hierarchy
MLNB_Isocortex_soma.txt:
	for file in `ls json30/`; do \
		($(JQ) '.neuron.soma.allenId as $$id | .neuron.allenInformation[] | select(.allenId == $$id) | .structureIdPath' "json30/$$file" \
			| grep -q '/$(ISOCORTEX_ID)/' \
			&& echo "$$file" && jq '.neuron.soma.allenId as $$id | .neuron.allenInformation[] | select(.allenId == $$id) | .acronym' "json30/$$file") \
			| tr -d '"' \
			| pr -at2 -s' ' \
			| sed 's/\.json//' \
			>> $@; \
	done

NJOB ?= 16
SLURM_ACCOUNT := proj83
SRUN = srun -A$(SLURM_ACCOUNT) -pprod --exclusive --mpi=none -Jmorphviz-{1} $(RELATIVE)
ifdef NNODE
RELATIVE := -r{%}
NJOB := $(call calc,$(NNODE) - 1)
endif
all-%: MLNB_Isocortex_soma.txt
	$(PARALLEL) --lb -C' ' -j$(NJOB) $(SRUN) $(MAKE) $* JSONFILE=json30/{1}.json :::: $<

endif # JSONFILE

.PHONY: cached-datasets
CACHED_TARGETS := $(CACHED_FLATMAP)
cached-datasets: $(CACHED_TARGETS)
$(CACHED_FLATMAP): SOURCE := $(FLATMAP_BOTH)
$(CACHED_TARGETS):
	unu save -i $(SOURCE) -f nrrd -e raw -o $@
