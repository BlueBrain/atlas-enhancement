include ../../common_header.mk

output/$(VOLUME_DECOMPOSITION_NRRD_FILE): input/$(FLATMAP_NRRD_FILE)
	$(FLAT_HEXGRID_BIN) $< $@ $(HEXGRID_DIAMETER)

# input links
input/$(FLATMAP_NRRD_FILE): SRC := $(STAGE_III_ROOT)/02_flatmap/output/$(FLATMAP_NRRD_FILE)

# auxiliary commands
view: view-nrrd

include ../../common_footer.mk
