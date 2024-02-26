include config.mk

# functions
wrap = $(addprefix $1,$(addsuffix $3,$2))
calc = $(shell calc -pd <<< '$1')

NULL :=
SPACE := $(NULL) # space
COMMA := ,

# venv with datashader
LOAD_VENV := source $(HOME)/flatmap/applications/conndataviz/venv/bin/activate

# commands
PYTHON := python3
FLATPLOT := $(PYTHON) $(HOME)/flatmap/applications/conndataviz/flatplot.py
GNUPLOT := gnuplot
MAGICK := magick

# variables
UID := $(shell id -u)
TMPDIR := /var/run/user/$(UID)
SCRATCH := $(TMPDIR)
LAYER_NAMES := 1 23 4 5 6
FLATPIX = $(shell grep HIST_PIXELS $(CONFIG) | cut -d' ' -f3)
FLATPIX2 = $(call calc,2 * $(FLATPIX))

# cached datasets
CACHED_FLATMAP := $(TMPDIR)/flatmap_conndataviz.nrrd
CACHED_LAYERS := $(TMPDIR)/layers_conndataviz.nrrd

ifdef EXPERIMENT_ID
SRC_REGION := $(shell awk '$$1 == $(EXPERIMENT_ID) { print $$2 }' experiment_ids)
ifeq (,$(SRC_REGION))
$(error Please first make experiment_ids)
endif

all: plot plot-split

.PHONY: get-data
DATA_TARGETS := $(DATADIR)/projection_density_$(EXPERIMENT_ID).nrrd \
				$(DATADIR)/injection_density_$(EXPERIMENT_ID).nrrd \
				$(DATADIR)/data_mask_$(EXPERIMENT_ID).nrrd
get-data: $(DATA_TARGETS)
$(DATADIR)/projection_density_$(EXPERIMENT_ID).nrrd: IMAGE := projection_density
$(DATADIR)/injection_density_$(EXPERIMENT_ID).nrrd: IMAGE := injection_density
$(DATADIR)/data_mask_$(EXPERIMENT_ID).nrrd: IMAGE := data_mask
$(DATA_TARGETS):
	curl -o '$@' 'http://api.brain-map.org/grid_data/download_file/$(EXPERIMENT_ID)?image=$(IMAGE)&resolution=10'

# all layers stuff
.PHONY: plot-data
PLOT_DATA_TARGETS := $(SCRATCH)/projection_density_$(EXPERIMENT_ID).png $(SCRATCH)/injection_density_$(EXPERIMENT_ID).png
plot-data: $(PLOT_DATA_TARGETS)
$(SCRATCH)/projection_density_$(EXPERIMENT_ID).png: CMAP := gray_rev
$(SCRATCH)/injection_density_$(EXPERIMENT_ID).png: CMAP := gray
$(PLOT_DATA_TARGETS): $(SCRATCH)/%.png: $(DATADIR)/%.nrrd $(DATADIR)/data_mask_$(EXPERIMENT_ID).nrrd | $(CACHED_FLATMAP)
	$(LOAD_VENV); \
	$(FLATPLOT) --colormap $(CMAP) --mask $(word 2,$^) -p $(FLATPIX) $(CACHED_FLATMAP) $< $(subst .png,,$@)

.PHONY: plot
BACKGROUND := background_Layer5.png
plot: $(EXPERIMENT_ID)_$(SRC_REGION).png
$(EXPERIMENT_ID)_$(SRC_REGION).png: $(SCRATCH)/$(EXPERIMENT_ID).png
	cp $< $@
$(SCRATCH)/$(EXPERIMENT_ID).png: $(SCRATCH)/projection_density_$(EXPERIMENT_ID).png $(SCRATCH)/injection_density_$(EXPERIMENT_ID).png $(BACKGROUND)
	$(MAGICK) composite -compose Multiply $< $(word 3,$^) - \
		| $(MAGICK) composite -compose Screen - $(word 2,$^) - \
		| $(MAGICK) composite -compose CopyOpacity $(word 3,$^) - $@

# per-layer stuff
.PHONY: plot-data-split
PLOT_DATA_SPLIT_TARGETS := $(SCRATCH)/projection_density_$(EXPERIMENT_ID).SPLIT.DONE $(SCRATCH)/injection_density_$(EXPERIMENT_ID).SPLIT.DONE
plot-data-split: $(PLOT_DATA_SPLIT_TARGETS)
$(SCRATCH)/projection_density_$(EXPERIMENT_ID).SPLIT.DONE: CMAP := gray_rev
$(SCRATCH)/injection_density_$(EXPERIMENT_ID).SPLIT.DONE: CMAP := gray
$(PLOT_DATA_SPLIT_TARGETS): $(SCRATCH)/%.SPLIT.DONE: $(DATADIR)/%.nrrd $(DATADIR)/data_mask_$(EXPERIMENT_ID).nrrd | $(CACHED_FLATMAP) $(CACHED_LAYERS)
	$(LOAD_VENV); \
	$(FLATPLOT) --colormap $(CMAP) --split --layers $(CACHED_LAYERS) --mask $(word 2,$^) -p $(FLATPIX) $(CACHED_FLATMAP) $< $(subst .SPLIT.DONE,,$@)
	touch $@

.PHONY: plot-split
plot-split: $(EXPERIMENT_ID)_$(SRC_REGION)_perlayer.png
$(EXPERIMENT_ID)_$(SRC_REGION)_perlayer.png: $(SCRATCH)/projection_density_$(EXPERIMENT_ID).SPLIT.DONE $(SCRATCH)/injection_density_$(EXPERIMENT_ID).SPLIT.DONE backgrounds.SPLIT.DONE
	$(foreach L,$(LAYER_NAMES),$(MAKE) $(SCRATCH)/$(EXPERIMENT_ID)_Layer$L.png;)
	-$(MAGICK) montage $(call wrap,$(SCRATCH)/$(EXPERIMENT_ID)_Layer,$(LAYER_NAMES),.png) -tile 1x$(words $(LAYER_NAMES)) -geometry +0+0 -background None $@

$(SCRATCH)/$(EXPERIMENT_ID)_Layer%.png: $(SCRATCH)/projection_density_$(EXPERIMENT_ID)_Layer%.png $(SCRATCH)/injection_density_$(EXPERIMENT_ID)_Layer%.png background_Layer%.png
	$(MAGICK) composite -compose Multiply $< $(word 3,$^) - \
		| $(MAGICK) composite -compose Screen - $(word 2,$^) - \
		| $(MAGICK) composite -compose CopyOpacity $(word 3,$^) - $@

.PHONY: clean
clean:
	$(RM) $(EXPERIMENT_ID)_$(SRC_REGION).png $(EXPERIMENT_ID)_$(SRC_REGION)_perlayer.png $(SCRATCH)/*$(EXPERIMENT_ID)*

else

# To be run inside an allocation with NJOB+1 nodes, e.g. $ salloc -Aproj83 -pprod --exclusive -N100
NJOB := 99
SLURM_ACCOUNT := proj83
all-%: experiment_ids
	parallel --lb -C' ' -j$(NJOB) srun -A$(SLURM_ACCOUNT) -pprod --exclusive -r{%} -N1 -J $*-{1} $(MAKE) $* EXPERIMENT_ID={1} :::: $<

endif

source_Isocortex.csv:
	curl -g -o '$@' 'http://connectivity.brain-map.org/projection/csv?criteria=service::mouse_connectivity_injection_structure[injection_structures$$eq315][primary_structure_only$$eqtrue]'

experiment_ids: source_Isocortex.csv
	awk -F, 'NR > 1 { print $$1,$$5 }' $< \
		| sort -g \
		| tee $@ >/dev/null

.PHONY: backgrounds
backgrounds: backgrounds.SPLIT.DONE
backgrounds.SPLIT.DONE: $(call wrap,background_Layer,$(LAYER_NAMES),.png)
	touch $@
background_Layer%.png: flatmap_Layer%.png
	$(MAGICK) composite -blend 50 $< -size $(FLATPIX)x$(FLATPIX) xc:white -alpha Set $@.tmp.png
	$(MAGICK) composite -compose CopyOpacity $< $@.tmp.png $@.tmp.png
	-$(MAGICK) montage $@.tmp.png -flip $@.tmp.png -rotate -90 -tile 2x1 -geometry $(FLATPIX)x$(FLATPIX)+0+0 -background None $@
	$(RM) $@.tmp.png

flatmap.SPLIT.DONE flatmap.png: CMAP = '$(subst $(SPACE),$(COMMA),$(foreach i,$(COLORING),$(word $i,$(PALETTE))))'

.PHONY: flatmap-split
FLATMAP_SPLIT_TARGETS: $(call wrap,flatmap_Layer,$(LAYER_NAMES),.png)
$(FLATMAP_SPLIT_TARGETS): flatmap.SPLIT.DONE
flatmap-split: flatmap.SPLIT.DONE
flatmap.SPLIT.DONE:
	$(LOAD_VENV); \
	$(FLATPLOT) --colormap $(CMAP) --regions --split --layers $(CACHED_LAYERS) -p $(FLATPIX) $(FLATMAP) $(REGIONS) $(subst .SPLIT.DONE,,$@)
	touch $@

.PHONY: flatmap
flatmap: flatmap_labeled.png
flatmap.png:
	$(LOAD_VENV); \
	$(FLATPLOT) --colormap $(CMAP) --regions -p $(FLATPIX) $(FLATMAP) $(REGIONS) $(subst .png,,$@)

flatmap_labeled.png: flatmap.png flatmap_labels.png
	$(MAGICK) convert -rotate 90 -flip $< - \
		| $(MAGICK) composite -blend 50 - -size $(FLATPIX)x$(FLATPIX) xc:white -alpha Set $@
	$(MAGICK) composite $(word 2,$^) $@ $@

ifdef LABELSFONT
LABELSFONT := font $(LABELSFONT)
endif

XCOL := 2
ifdef XREV
XCOL := (1 - \$$2)
endif

LABELEXT := png
LABELTERM := png
LABELTERM_OPTS := transparent

.PHONY: flatmap-labels
flatmap-labels: flatmap_labels.$(LABELEXT)
flatmap_labels.$(LABELEXT): flatmap_labels.txt
	echo -e "\
unset key; unset tics; unset border\n\
set margin 0,0,0,0\n\
set xr [0:1]\n\
set yr [0:1]\n\
set size square\n\
set out '$@'\n\
set term $(LABELTERM) size $(FLATPIX),$(FLATPIX) $(LABELTERM_OPTS)\n\
plot '$<' u $(XCOL):3:1 w labels $(LABELSFONT)\n\
" | $(GNUPLOT)

flatmap_labels.txt: regions.txt
	$(LOAD_VENV); \
	echo -e "\
import voxcell as vc                          \n\
import numpy as np                            \n\
annot = vc.VoxelData.load_nrrd('$(REGIONS)')  \n\
fmap = vc.VoxelData.load_nrrd('$(FLATMAP)')   \n\
reg = np.loadtxt('$<',dtype='U16')            \n\
fmap_valid = (fmap.raw[:,:,:,0] > -1)         \n\
mask = np.where(fmap_valid)                   \n\
flatpos = fmap.raw[mask]                      \n\
flatreg = annot.raw[mask]                     \n\
res = {}                                      \n\
for acro,id in reg:                           \n\
    w = np.where(flatreg == int(id))          \n\
    fpos = flatpos[w]                         \n\
    fpos_valid = (fpos[:,0] > -1)             \n\
    mean = np.mean(fpos[fpos_valid],axis=0)   \n\
    res.update({acro: mean})                  \n\
with open('$@','w') as f:                     \n\
    for k,v in res.items():                   \n\
        print(k,v[0],v[1],file=f)             \n\
" | $(PYTHON)

.PHONY: cached-datasets
CACHED_TARGETS := $(CACHED_FLATMAP) $(CACHED_LAYERS)
cached-datasets: $(CACHED_TARGETS)
$(CACHED_FLATMAP): SOURCE := $(FLATMAP_BOTH)
$(CACHED_LAYERS): SOURCE := $(LAYERS)
$(CACHED_TARGETS):
	unu save -i $(SOURCE) -f nrrd -e raw -o $@
