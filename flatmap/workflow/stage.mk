.DEFAULT_GOAL := all

include config.mk

all: $(DEFAULT_STEPS)

define step_rule
.PHONY: $1
$1:
	cd $1 && $(MAKE)
endef

define step_rule_cmd
.PHONY: $1
$2-$1:
	cd $1 && $(MAKE) $2
endef

define all_steps_rule
$1: $$(addprefix $1-,$$(STEPS))
$$(foreach s,$$(STEPS),$$(eval $$(call step_rule_cmd,$$s,$1)))
endef

define some_steps_rule
$3-$1: $$(addprefix $1-,$2)
endef

# individual step targets
$(foreach s,$(STEPS),$(eval $(call step_rule,$s)))

STEP_TARGETS := setup view clean clean-all export

# all-step targets
$(foreach t,$(STEP_TARGETS),$(eval $(call all_steps_rule,$t)))
