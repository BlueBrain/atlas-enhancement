# path to executable
METRICS_DIVERGENCE_BIN := $(PYTHON3) $(SOURCE_CODE_ROOT)/metrics/divergence.py

# DO NOT EDIT BELOW THIS LINE
override INPUTS := $(addprefix input/,$(USER_INPUT_ORIENTATION_FILES))\
	input/$(MASK_FILE)

override OUTPUTS := output/$(METRICS_DIVERGENCE_NRRD_FILE)

override USER_PARAMETERS :=

override USER_BINARIES := METRICS_DIVERGENCE_BIN
