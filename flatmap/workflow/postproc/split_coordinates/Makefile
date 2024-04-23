include ../../common_header.mk

ifeq (1,$(DISCRETE))
output/$(FLATMAP_SPLIT_X_DISCRETE_NRRD_FILE) output/$(FLATMAP_SPLIT_Y_DISCRETE_NRRD_FILE): input/$(FLATMAP_DISCRETE_NRRD_FILE)
	$(SPLIT_FLATMAP_BIN) $< $(dir $@)

# input links
input/$(FLATMAP_DISCRETE_NRRD_FILE): SRC := $(STAGE_III_ROOT)/03_discretization/output/$(FLATMAP_DISCRETE_NRRD_FILE)
else
output/$(FLATMAP_SPLIT_X_NRRD_FILE) output/$(FLATMAP_SPLIT_Y_NRRD_FILE): input/$(FLATMAP_NRRD_FILE)
	$(SPLIT_FLATMAP_BIN) $< $(dir $@)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)
endif

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
