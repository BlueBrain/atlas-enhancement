# path to executable
PLOT_HEATMAP_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/utils/plot_heatmap.py

# DO NOT EDIT BELOW THIS LINE
nrrd2png = $(subst .nrrd,.png,$1)
$(foreach m,$(METRICS_DISCRETE),$(eval HEATMAP_$m_FILE = $(subst %,$$1,$(call nrrd2png,$(call METRICS_$m_NRRD_FILE,%)))))

override INPUTS = $(addprefix input/,$(foreach m,$(METRICS_DISCRETE),$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_$m_NRRD_FILE,$r))))

override OUTPUTS = $(addprefix output/,$(foreach m,$(METRICS_DISCRETE),$(foreach r,$(METRICS_RESOLUTION),$(call HEATMAP_$m_FILE,$r))))

override USER_PARAMETERS := METRICS_RESOLUTION HEATMAP_SCALING $(foreach m,$(METRICS_DISCRETE),$m_PLOT_EXTRA)

override USER_BINARIES := PLOT_HEATMAP_BIN
