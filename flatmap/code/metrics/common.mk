calc = $(shell calc -pd <<< '$1')
wrap = $(addsuffix $3,$(addprefix $1,$2))

.ONESHELL:

SHELL := /bin/bash
_SET_PIPEFAIL := set -euo pipefail

# Command paths
PYTHON ?= python
RSCRIPT ?= Rscript
PARALLEL ?= parallel --lb --halt now,fail=1

# SLURM setup
SLURM_ACCOUNT ?= proj83
SLURM_PARTITION ?= prod
SLURM_TIMELIMIT ?= 24:00:00
SLURM_JOBNAME ?= flatmap-metrics
SRUN = srun -A$(SLURM_ACCOUNT) -p$(SLURM_PARTITION) -t$(SLURM_TIMELIMIT) -J$(SLURM_JOBNAME) --exclusive $(SLURM_EXTRA)

# Global path
FLATMAP_METRICS_ROOT := $(HOME)/flatmap/metrics

# Virtualenv load
LOAD_ENV := source $(FLATMAP_METRICS_ROOT)/setup.sh

# Executables performing analyses
N_NOT_MAPPED := $(PYTHON) $(FLATMAP_METRICS_ROOT)/n_not_mapped.py
LARGEST_CC_TO_TOTAL := $(PYTHON) $(FLATMAP_METRICS_ROOT)/largest_cc_to_total.py
NHOOD_CC := $(PYTHON) $(FLATMAP_METRICS_ROOT)/nhood_cc.py
NHOOD_LCCRATIO := $(PYTHON) $(FLATMAP_METRICS_ROOT)/nhood_lccratio.py
PIXEL_WIDTH := $(PYTHON) $(FLATMAP_METRICS_ROOT)/pixel_width.py
DISTANCE_METRIC := $(PYTHON) $(FLATMAP_METRICS_ROOT)/distance_metric.py
PERC_NCOMP1 := $(PYTHON) $(FLATMAP_METRICS_ROOT)/perc_ncomp1.py
PERC_LCCRATIO_NEAR1 := $(PYTHON) $(FLATMAP_METRICS_ROOT)/perc_lccratio_near1.py
PERC_GOODFIT :=  $(PYTHON) $(FLATMAP_METRICS_ROOT)/perc_goodfit.py
PIXEL_METRIC := $(PYTHON) $(FLATMAP_METRICS_ROOT)/pixel_metric.py
FLAT_USAGE := $(PYTHON) $(FLATMAP_METRICS_ROOT)/flat_usage.py
NVOX_NORM := $(PYTHON) $(FLATMAP_METRICS_ROOT)/nvox_norm.py
GINI_COEFF := $(PYTHON) $(FLATMAP_METRICS_ROOT)/gini_coeff.py
ORTHOGONALITY := $(PYTHON) $(FLATMAP_METRICS_ROOT)/orthogonality.py
DUMP_VALUES := $(PYTHON) $(FLATMAP_METRICS_ROOT)/dumpvalues.py
LIFT_VALUES := $(PYTHON) $(FLATMAP_METRICS_ROOT)/lift.py
STATS := $(RSCRIPT) $(FLATMAP_METRICS_ROOT)/stats.R
PERVOX := $(RSCRIPT) $(FLATMAP_METRICS_ROOT)/pervox.R

# Input files
MASK := $(RESULTS_ROOT)/mask.nrrd
FLATMAP_FLOAT := $(RESULTS_ROOT)/flatmap_float.nrrd
DIMENSIONS := $(RESULTS_ROOT)/dimensions.txt
NORMAL_X := $(RESULTS_ROOT)/normal_x.nrrd
NORMAL_Y := $(RESULTS_ROOT)/normal_y.nrrd
NORMAL_Z := $(RESULTS_ROOT)/normal_z.nrrd

# Analyses
GINI_CASES := nvox ncomp rmean
GINI_TARGETS := $(addprefix gini-,$(GINI_CASES))

ANALYSIS_TARGETS := ncomp1 lccratio-near1 flat-usage goodfit pixel-metric
VOLUMETRIC_TARGETS := orthogonality divergence err-vs-grad laplacian

.PHONY: analyze volumetric
analyze: n-not-mapped nvox-per-vertex $(ANALYSIS_TARGETS) $(GINI_TARGETS)
volumetric: $(VOLUMETRIC_TARGETS)

# Discretization sizes analyzed
SIZES ?= $(shell cat sizes)# overridable

## Global analyses

.PHONY: n-not-mapped
n-not-mapped: n_not_mapped.txt
.DELETE_ON_ERROR: n_not_mapped.txt
n_not_mapped.txt: $(FLATMAP_FLOAT) $(MASK)
	$(_SET_PIPEFAIL)
	$(LOAD_ENV)
	$(N_NOT_MAPPED) $^ | tee $@

nvox-per-vertex: nvox_per_vertex.txt
.DELETE_ON_ERROR: nvox_per_vertex.txt
nvox_per_vertex.txt: $(RESULTS_ROOT)/nearest.txt
	cat $< \
		| cut -d' ' -f1 \
		| sed 's/^v//' \
		| sort -g --parallel=10 -S10G \
		| uniq -c \
		> $@

.PHONY: approx-native-npix
approx-native-npix: approx_native_npix.txt
approx_native_npix.txt: nvox_per_vertex.txt
	cat $< | wc -l \
		| sed -e 's/^/sqrt(/' -e 's/$$/)/' \
		| calc -pd \
		| tee $@

.PHONY: coverage
coverage: coverage.txt
coverage.txt: n_not_mapped.txt
	calc -pd <<< '1 - $(shell cut -d' ' -f2 $<)' | tee $@

.PHONY: usage-fraction-native
usage-fraction-native: usage_fraction_native.txt
usage_fraction_native.txt: nvox_per_vertex.txt $(RESULTS_ROOT)/nearest.txt
	calc -pd <<< '$(shell cat $< | wc -l) / $(shell cat $(word 2,$^) | wc -l)' | tee $@

# NN error
.PHONY: nnerror
nnerror: nearest_error.pdf
nearest_error.pdf: nearest_error.txt
	$(STATS) $< $@ NNerr > stats_nearest_error.txt

nearest_error.txt: $(RESULTS_ROOT)/nearest.txt
	awk '{ print $$5 }' $< > $@

# mesh depths
.PHONY: mesh-depths
mesh-depths: mesh_depths.pdf
mesh_depths.pdf: mesh_depths.txt
	$(STATS) $< $@ depth > stats_mesh_depths.txt

mesh_depths.txt: $(RESULTS_ROOT)/surf.off $(RESULTS_ROOT)/layers.nrrd
	awk 'NR == 2 { nv=$$1 } NR > 2 && NR <= 2 + nv' $< > mesh_vertices.txt
	$(LOAD_ENV); \
	echo -e "\
import voxcell as vc                               \n\
import numpy as np                                 \n\
with open('mesh_vertices.txt') as f:               \n\
    vtx = [line.rstrip().split(' ') for line in f] \n\
    vtx = np.array(vtx,dtype=np.float32)           \n\
depth = vc.VoxelData.load_nrrd('$(word 2,$^)')     \n\
d = depth.lookup(vtx)                              \n\
np.savetxt('$@',d)                                 \n\
" | python

# per-voxel analyses
.PHONY: orthogonality
orthogonality: orthogonality.pdf
orthogonality.nrrd: VOXEL_SIZE = $(shell awk 'NR == 3' $(DIMENSIONS))
orthogonality.nrrd: $(FLATMAP_FLOAT) $(NORMAL_X) $(NORMAL_Y) $(NORMAL_Z) $(DIMENSIONS)
	$(ORTHOGONALITY) $(wordlist 1,4,$^) $(VOXEL_SIZE) $(REVERSE_CROSS)

orthogonality.txt: orthogonality.nrrd $(RESULTS_ROOT)/mask_interior.nrrd
	$(LOAD_ENV); \
	$(DUMP_VALUES) $< $@ $(word 2,$^)

orthogonality.pdf: orthogonality.txt
	$(STATS) $< $@ orthogonality > stats_orthogonality.txt

.PHONY: divergence
divergence: divergence.pdf
divergence.nrrd: $(NORMAL_X) $(NORMAL_Y) $(NORMAL_Z) $(MASK)
	$(LOAD_ENV); \
	echo -e "\
import numpy as np                                  \n\
import voxcell as vc                                \n\
vx = vc.VoxelData.load_nrrd('$(word 1,$^)')         \n\
vy = vc.VoxelData.load_nrrd('$(word 2,$^)')         \n\
vz = vc.VoxelData.load_nrrd('$(word 3,$^)')         \n\
mask = vc.VoxelData.load_nrrd('$(word 4,$^)')       \n\
div = np.gradient(vx.raw, axis=0) +                   \
      np.gradient(vy.raw, axis=1) +                   \
      np.gradient(vz.raw, axis=2)                   \n\
div[mask.raw == 0] = np.nan                         \n\
vx.with_data(div.astype('float32')).save_nrrd('$@') \n\
" | python

divergence.txt: divergence.nrrd $(RESULTS_ROOT)/mask_interior.nrrd
	$(LOAD_ENV); \
	$(DUMP_VALUES) $< $@ $(word 2,$^)

divergence.pdf: divergence.txt
	$(STATS) $< $@ divergence > stats_divergence.txt

.PHONY: err-vs-grad
error-vs-grad: error_vs_grad.pdf
error_vs_grad.nrrd: VOXEL_SIZE = $(shell awk 'NR == 3' $(DIMENSIONS))
error_vs_grad.nrrd: $(RESULTS_ROOT)/layers.nrrd $(NORMAL_X) $(NORMAL_Y) $(NORMAL_Z) $(MASK) $(DIMENSIONS)
	$(LOAD_ENV); \
	echo -e "\
import numpy as np                                  \n\
import voxcell as vc                                \n\
depth = vc.VoxelData.load_nrrd('$(word 1,$^)')      \n\
vx = vc.VoxelData.load_nrrd('$(word 2,$^)')         \n\
vy = vc.VoxelData.load_nrrd('$(word 3,$^)')         \n\
vz = vc.VoxelData.load_nrrd('$(word 4,$^)')         \n\
mask = vc.VoxelData.load_nrrd('$(word 5,$^)')       \n\
dx, dy, dz = np.gradient(depth.raw)                 \n\
# normalize gradient                                \n\
norm = np.sqrt(dx ** 2 + dy ** 2 + dz ** 2)         \n\
nonzero = np.where(norm > 0)                        \n\
dx[nonzero] /= norm[nonzero]                        \n\
dy[nonzero] /= norm[nonzero]                        \n\
dz[nonzero] /= norm[nonzero]                        \n\
dx *= np.sign($(word 1,$(VOXEL_SIZE))) # match vx   \n\
dy *= np.sign($(word 2,$(VOXEL_SIZE))) # match vy   \n\
dz *= np.sign($(word 3,$(VOXEL_SIZE))) # match vz   \n\
err = np.sqrt((vx.raw - dx) ** 2 +                    \
              (vy.raw - dy) ** 2 +                    \
              (vz.raw - dz) ** 2)                   \n\
err[mask.raw == 0] = np.nan                         \n\
vx.with_data(dx.astype('float32')).save_nrrd('grad_x.nrrd')\n\
vx.with_data(dy.astype('float32')).save_nrrd('grad_y.nrrd')\n\
vx.with_data(dz.astype('float32')).save_nrrd('grad_z.nrrd')\n\
vx.with_data(err.astype('float32')).save_nrrd('$@') \n\
" | python

error_vs_grad.txt: error_vs_grad.nrrd $(RESULTS_ROOT)/mask_interior.nrrd
	$(LOAD_ENV); \
	$(DUMP_VALUES) $< $@ $(word 2,$^)

error_vs_grad.pdf: error_vs_grad.txt
	$(STATS) $< $@ error_vs_grad > stats_error_vs_grad.txt

.PHONY: laplacian
laplacian: laplacian.pdf
laplacian.nrrd: $(RESULTS_ROOT)/layers.nrrd
	$(LOAD_ENV); \
	echo -e "\
import numpy as np                                      \n\
import voxcell as vc                                    \n\
from scipy.ndimage import laplace                       \n\
dnorm = vc.VoxelData.load_nrrd('$<')                    \n\
lapl = laplace(dnorm.raw,mode='nearest')                \n\
dnorm.with_data(lapl.astype('float32')).save_nrrd('$@') \n\
" | python

laplacian.txt: laplacian.nrrd $(RESULTS_ROOT)/mask_interior.nrrd
	$(LOAD_ENV); \
	$(DUMP_VALUES) $< $@ $(word 2,$^)

laplacian.pdf: laplacian.txt
	$(STATS) $< $@ laplacian > stats_laplacian.txt

.PHONY: pervox
pervox: pervox.png
pervox.png: divergence.txt laplacian.txt orthogonality.txt
	$(PERVOX) $^ $@

## Discretization-level analyses
define analysis_rule
.PHONY: $1
$1: $2.txt
.DELETE_ON_ERROR: $2.txt
$2.txt:
	$(_SET_PIPEFAIL)
	$(LOAD_ENV)
	$(PARALLEL) $$($4) $3_{}.img {} ::: $(SIZES) \
		| sort -k1g \
		| tee $$@
endef

$(eval $(call analysis_rule,ncomp1,ncomp1,ncomp,PERC_NCOMP1))
$(eval $(call analysis_rule,lccratio-near1,lccratio_near1,lccratio,PERC_LCCRATIO_NEAR1))
$(eval $(call analysis_rule,goodfit,goodfit,fit_rerr,PERC_GOODFIT))
$(eval $(call analysis_rule,flat-usage,flat_usage,nvox,FLAT_USAGE))
$(eval $(call analysis_rule,pixel-metric,pixel_metric,rmean,PIXEL_METRIC))

define gini_rule
.PHONY: gini-$1
gini-$1: gini_coeff_$1.txt
.DELETE_ON_ERROR: gini_coeff_$1.txt
gini_coeff_$1.txt:
	$(_SET_PIPEFAIL)
	$(LOAD_ENV)
	$(PARALLEL) $(GINI_COEFF) $1_{}.img {} ::: $(SIZES) \
		| sort -k1g \
		| tee $$@
endef
$(foreach _,$(GINI_CASES),$(eval $(call gini_rule,$_)))


# Generate metrics at different levels of discretization
SIZE_DEPENDENT_TARGETS := nvox nvox-norm ncomp lccratio fit rmean rratio nhood-lccratio #nhoodcc

sizes-all: $(addprefix all-,$(SIZE_DEPENDENT_TARGETS))
sizes-png: $(addprefix png-,$(SIZE_DEPENDENT_TARGETS))

define all_rule
all-$1: SLURM_JOBNAME := $1-{}
all-$1:
	$$(PARALLEL) $$(SRUN) $$(MAKE) $1 SIZE={} ::: $$(SIZES)

png-$1:
	$$(PARALLEL) $$(MAKE) $1-png SIZE={} ::: $$(SIZES)
endef
$(foreach _,$(SIZE_DEPENDENT_TARGETS),$(eval $(call all_rule,$_)))

refresh:
	$(MAKE) -t sizes-all SRUN=

.PHONY: $(SIZE_DEPENDENT_TARGETS)
ifdef SIZE

all: $(SIZE_DEPENDENT_TARGETS)

nvox: nvox_$(SIZE).img
ncomp: ncomp_$(SIZE).img
lccratio: lccratio_$(SIZE).img
nvox_$(SIZE).img ncomp_$(SIZE).img lccratio_$(SIZE).img: $(FLATMAP_FLOAT)
	$(LOAD_ENV)
	$(LARGEST_CC_TO_TOTAL) $^ $(SIZE) $(NCPU)

NMAPPED = $(shell cat $(DIMENSIONS) n_not_mapped.txt | awk 'NR == 1 {tot=$$1} NR == 5 {not=$$1} END {print tot - not}')
nvox-norm: nvox_norm_$(SIZE).img
nvox_norm_$(SIZE).img: nvox_$(SIZE).img | n_not_mapped.txt
	$(LOAD_ENV)
	$(NVOX_NORM) $< $(SIZE) $(NMAPPED)

# NOTE: Deprecated in favor of nhood-lccratio
#nhoodcc: nhoodcc_$(SIZE).img
#nhoodcc_$(SIZE).img: $(FLATMAP_FLOAT)
#	$(LOAD_ENV)
#	$(NHOOD_CC) $^ $(SIZE) $(NCPU)

nhood-lccratio: nhood_lccratio_$(SIZE).img
nhood_lccratio_$(SIZE).img: $(FLATMAP_FLOAT)
	$(LOAD_ENV)
	$(NHOOD_LCCRATIO) $^ $(SIZE) $(NCPU)

FIT_TARGETS := fit_r0_$(SIZE).img fit_r1_$(SIZE).img fit_K_$(SIZE).img fit_rerr_$(SIZE).img fit_herr_$(SIZE).img
fit: $(FIT_TARGETS)
$(FIT_TARGETS): $(FLATMAP_FLOAT)
	$(LOAD_ENV)
	$(PIXEL_WIDTH) $^ $(SIZE) $(NORMAL_X) $(NORMAL_Y) $(NORMAL_Z) $(NCPU)

rmean: rmean_$(SIZE).img
rratio: rratio_$(SIZE).img
rmean_$(SIZE).img rratio_$(SIZE).img: fit_r0_$(SIZE).img fit_r1_$(SIZE).img fit_rerr_$(SIZE).img
	$(LOAD_ENV)
	$(DISTANCE_METRIC) $^ $(SIZE)

# PNGs
all-png: $(addsuffix -png,$(SIZE_DEPENDENT_TARGETS))

ncomp-png: ncomp_$(SIZE).png
ncomp_$(SIZE).png: CASEFLAGS := -c grad:grayscale

fit-png: fit_rerr_$(SIZE).png fit_herr_$(SIZE).png
fit_rerr_$(SIZE).png: CASEFLAGS := -c grad:grayscale -r -q 0.95 -Q 1.0# in [0.95,1] black is good
fit_herr_$(SIZE).png: CASEFLAGS := -c grad:grayscale -r -q 0.95 -Q 1.0# in [0.95,1] black is good

# pre-image size
nvox-png: nvox_$(SIZE).png
nvox_$(SIZE).png: CASEFLAGS := -c grad:viridis -q 0.001 -w '\#ff0000'# >= 1, holes in red

nvox-norm-png: nvox_norm_$(SIZE).png
nvox_norm_$(SIZE).png: CASEFLAGS := -c grad:msh -q $(call calc,-1.0 / ($(SIZE) * $(SIZE))) -Q $(call calc,3.0 / ($(SIZE) * $(SIZE)))

# pre-image connectedness
lccratio-png: lccratio_$(SIZE).png
lccratio_$(SIZE).png: CASEFLAGS := -c grad:viridis -q 0.0 -Q 1.0# in [0,1] yellow is good

# pre-image radius
rmean-png: rmean_$(SIZE).png
rmean_$(SIZE).png: CASEFLAGS := -c grad:magma -q 0.0# >= 0

# pre-image conicity
rratio-png: rratio_$(SIZE).png
rratio_$(SIZE).png: CASEFLAGS := -c grad:magma -q 0.0 -Q 1.0# in [0,1], yellow is more conicity

K_RANGE ?= 5E-7
# pre-image conicality
fit_K-png: fit_K_$(SIZE).png
fit_K_$(SIZE).png: CASEFLAGS := -c 'predef:Scientific Colour Maps vik' -q -$(K_RANGE) -Q $(K_RANGE)
fit_K-lift: fit_K_$(SIZE).nrrd
fit_K_$(SIZE).nrrd: BKGVAL := NaN
fit_K_$(SIZE).nrrd: fit_K_$(SIZE).img
	$(LOAD_ENV)
	$(LIFT_VALUES) $(FLATMAP_FLOAT) $(SIZE) $< $(BKGVAL) $@

# pre-image pairwise continuity
nhood-lccratio-png: nhood_lccratio_$(SIZE).png
nhood_lccratio_$(SIZE).png: CASEFLAGS := -c 'predef:Scientific Colour Maps buda' -q 0.0 -Q 1.0# in [0,1] yellow is good

%_$(SIZE).png: %_$(SIZE).img
	slicer $(CASEFLAGS) $(SLICERFLAGS) $< $(SIZE)x$(SIZE)x1 Z0 $@.tmp.ppm \
		&& pnmtopng $@.tmp.ppm > $@
	$(RM) $@.tmp.ppm

else  # SIZE

$(SIZE_DEPENDENT_TARGETS):
	@echo 'Please set SIZE or make all-$@'

endif # SIZE

PLOT_FILES := nvox_{1}.png \
			  nvox_norm_{1}.png \
			  lccratio_{1}.png \
			  rratio_{1}.png \
			  rmean_{1}.png \
			  fit_rerr_{1}.png \
			  fit_K_{1}.png \
			  nhood_lccratio_{1}.png
plots:
	$(PARALLEL) -q $(MAKE) $(PLOT_FILES) SIZE={1} SLICERFLAGS="-z{2}" ::: $(SIZES_SHOW) :::+ $(SIZES_SHOW_SCALING)

figure: metrics.png
metrics.png:
	-montage -background white -geometry +3+3 -tile $(words $(SIZES_SHOW))x$(words $(PLOT_FILES)) \
		$(call wrap,nvox_,$(SIZES_SHOW),.png) \
		$(call wrap,lccratio_,$(SIZES_SHOW),.png) \
		$(call wrap,nhood_lccratio_,$(SIZES_SHOW),.png) \
		$(call wrap,rmean_,$(SIZES_SHOW),.png) \
		$(call wrap,rratio_,$(SIZES_SHOW),.png) \
		$(call wrap,fit_rerr_,$(SIZES_SHOW),.png) \
		$@

pixel_metric.pdf: pixel_metric.gnuplot | pixel_metric.txt
	gnuplot -e 'set term pdf size 8,3 fontscale 0.8; set out "$@"' $^

clean-png:
	$(RM) *.png
