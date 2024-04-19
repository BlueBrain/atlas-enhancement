.DEFAULT_GOAL := all

include ../../config.mk
include ../config.mk
include config.mk

include .userconfig.mk
# user configuration
.SILENT: .userconfig.mk
.userconfig.mk: $(USER_DATA_ROOT)/config.mk
	$(ECHO) -n > $@.tmp
	$(foreach u,$(USER_PARAMETERS),$(GREP) '^$u' $< >> $@.tmp;)
	$(foreach u,$(USER_BINARIES),$(GREP) '^$u' $< >> $@.tmp;)
	$(CMP) -s $@ $@.tmp || $(MV) -f $@.tmp $@
	$(RM) $@.tmp

.ONESHELL:

.PHONY: all view setup
.DELETE_ON_ERROR: $(OUTPUTS) $(TEMPORARY)

define required
ifeq (notfound,$(shell type $1 || echo notfound))
$$(error Required binary $1 not found)
endif
endef
$(eval $(call required,$(PYTHON3)))

setup: $(INPUTS)
all: $(OUTPUTS)
