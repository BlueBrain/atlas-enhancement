include ../../common_header.mk

METRICS_RESOLUTION_SEQ = $(shell seq 1 $(words $(METRICS_RESOLUTION)))

define heatmap_rule
output/$$(call HEATMAP_$1_FILE,$2): HEATMAP_PLOT_EXTRA := $$($1_PLOT_EXTRA)
output/$$(call HEATMAP_$1_FILE,$2): SCALING := $3
output/$$(call HEATMAP_$1_FILE,$2): input/$$(call METRICS_$1_NRRD_FILE,$2)
endef
$(foreach m,$(METRICS_DISCRETE),$(foreach i,$(METRICS_RESOLUTION_SEQ),$(eval $(call heatmap_rule,$m,$(word $i,$(METRICS_RESOLUTION)),$(word $i,$(HEATMAP_SCALING))))))

$(OUTPUTS):
	$(PLOT_HEATMAP_BIN) -z $(SCALING) $< $(subst .png,,$@) $(HEATMAP_PLOT_EXTRA)

# input links
define create_input_links
input/$$(call METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_size_discrete/output/$$(call METRICS_PREIMAGE_SIZE_DISCRETE_NRRD_FILE,$1)
input/$$(call METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_size_uniformity/output/$$(call METRICS_PREIMAGE_SIZE_UNIFORMITY_NRRD_FILE,$1)
input/$$(call METRICS_PREIMAGE_CONNECTEDNESS_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_connectedness/output/$$(call METRICS_PREIMAGE_CONNECTEDNESS_NRRD_FILE,$1)
input/$$(call METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/pairwise_preimage_continuity/output/$$(call METRICS_PAIRWISE_PREIMAGE_CONTINUITY_NRRD_FILE,$1)
input/$$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_geometry/output/$$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$1)
input/$$(call METRICS_PREIMAGE_CONICITY_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_geometry/output/$$(call METRICS_PREIMAGE_CONICITY_NRRD_FILE,$1)
endef
$(foreach r,$(METRICS_RESOLUTION),$(eval $(call create_input_links,$r)))

# auxiliary commands
view: view-all

view-all:
	$(IMAGE_VIEWER) $(OUTPUTS)

include ../../common_footer.mk
