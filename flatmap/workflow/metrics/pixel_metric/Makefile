include ../../common_header.mk

output/$(METRICS_PIXEL_METRIC_PLOT_FILE): output/pixel_metric.txt
	$(GNUPLOT) -e 'infile = "$<"' -e 'outfile = "$@"' $(PIXEL_METRIC_GNUPLOT)

NJOB ?= $(words $(METRICS_RESOLUTION))
output/pixel_metric.txt: $(INPUTS)
	$(PARALLEL) -k -j$(NJOB) $(GET_MEAN_SD_BIN) {1} {2} ::: $(wordlist 1,$(words $(METRICS_RESOLUTION)),$^) :::+ $(METRICS_RESOLUTION) > $@

# input links
define create_input_links
input/$$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$1): SRC := $(METRICS_ROOT)/preimage_geometry/output/$$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$1)
endef
$(foreach r,$(METRICS_RESOLUTION),$(eval $(call create_input_links,$r)))

# auxiliary commands
view: view-pdf

include ../../common_footer.mk
