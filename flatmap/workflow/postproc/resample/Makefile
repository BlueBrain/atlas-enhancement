include ../../common_header.mk

ifndef FLATMAP_RESAMPLE_MASK
$(error Please set FLATMAP_RESAMPLE_MASK to location of resampling mask NRRD file)
endif

ifeq (1,$(DISCRETE))
output/$(FLATMAP_RESAMPLED_DISCRETE_NRRD_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE) input/$(FLATMAP_RESAMPLE_MASK)
	$(RESAMPLE_FLATMAP_BIN) $< $(word 2,$^) $@

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)
else
output/$(FLATMAP_RESAMPLED_NRRD_FILE): input/$(FLATMAP_NRRD_FILE) input/$(FLATMAP_RESAMPLE_MASK)
	$(RESAMLE_FLATMAP_BIN) $< $(word 2,$^) $@

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
endif

# input links
input/$(FLATMAP_RESAMPLE_MASK): SRC := $(FLATMAP_RESAMPLE_MASK)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
