# path to executable
PLOT_HEATMAP_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/utils/plot_heatmap.py

# DO NOT EDIT BELOW THIS LINE
nrrd2png = $(subst .nrrd,.png,$1)
$(foreach m,$(METRICS_DISCRETE),$(eval HEATMAP_$m_FILE := $(call nrrd2png,$(METRICS_$m_NRRD_FILE))))

override INPUTS := $(addprefix input/,$(foreach m,$(METRICS_DISCRETE),$(METRICS_$m_NRRD_FILE)))

override OUTPUTS := $(addprefix output/,$(foreach m,$(METRICS_DISCRETE),$(HEATMAP_$m_FILE)))

override USER_PARAMETERS := $(foreach m,$(METRICS_DISCRETE),$m_PLOT_EXTRA)

override USER_BINARIES := PLOT_HEATMAP_BIN
