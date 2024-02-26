include ../../common_header.mk

define heatmap_rule
output/$$(HEATMAP_$1_FILE): HEATMAP_PLOT_EXTRA := $$($1_PLOT_EXTRA)
output/$$(HEATMAP_$1_FILE): input/$$(METRICS_$1_NRRD_FILE)
endef
$(foreach m,$(METRICS_DISCRETE),$(eval $(call heatmap_rule,$m)))

$(OUTPUTS):
	$(PLOT_HEATMAP_BIN) $< $(subst .png,,$@) $(HEATMAP_PLOT_EXTRA)

# input links
input/$(METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE): SRC := $(METRICS_ROOT)/preimage_size_discrete/output/$(METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE)
input/$(METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE): SRC := $(METRICS_ROOT)/preimage_size_uniformity/output/$(METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE)
input/$(METRICS_PREIMAGE_CONNECTEDNESS_NRRD_FILE): SRC := $(METRICS_ROOT)/preimage_connectedness/output/$(METRICS_PREIMAGE_CONNECTEDNESS_NRRD_FILE)
input/$(METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE): SRC := $(METRICS_ROOT)/pairwise_preimage_continuity/output/$(METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE)
input/$(METRICS_PREIMAGE_RADIUS_NRRD_FILE): SRC := $(METRICS_ROOT)/preimage_geometry/output/$(METRICS_PREIMAGE_RADIUS_NRRD_FILE)
input/$(METRICS_PREIMAGE_CONICITY_NRRD_FILE): SRC := $(METRICS_ROOT)/preimage_geometry/output/$(METRICS_PREIMAGE_CONICITY_NRRD_FILE)

# auxiliary commands
view: view-all

view-all:
	$(IMAGE_VIEWER) $(OUTPUTS)

include ../../common_footer.mk
