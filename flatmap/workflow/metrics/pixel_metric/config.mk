# path to executable
GET_MEAN_SD_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/utils/get_mean_sd.py
PIXEL_METRIC_GNUPLOT := $(SOURCE_CODE_ROOT)/metrics/pixel_metric.gnuplot

# DO NOT EDIT BELOW THIS LINE
override INPUTS = $(addprefix input/,$(foreach r,$(METRICS_RESOLUTION),$(call METRICS_PREIMAGE_RADIUS_NRRD_FILE,$r)))

override OUTPUTS := output/$(METRICS_PIXEL_METRIC_PLOT_FILE)

override INTERMEDIATE := output/pixel_metric.txt

override USER_PARAMETERS := METRICS_RESOLUTION

override USER_BINARIES := GET_MEAN_SD_BIN PIXEL_METRIC_GNUPLOT
