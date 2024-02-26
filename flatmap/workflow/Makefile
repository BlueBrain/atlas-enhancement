include stage.mk

flatmap: $(FLATMAP_STEPS)
$(foreach t,$(STEP_TARGETS),$(eval $(call some_steps_rule,$t,$(FLATMAP_STEPS),flatmap)))

metrics: $(METRICS_STEPS)
$(foreach t,$(STEP_TARGETS),$(eval $(call some_steps_rule,$t,$(METRICS_STEPS),metrics)))

discrete: $(DISCRETE_STEPS)
$(foreach t,$(STEP_TARGETS),$(eval $(call some_steps_rule,$t,$(DISCRETE_STEPS),discrete)))

applications: $(APPLICATIONS_STEPS)
$(foreach t,$(STEP_TARGETS),$(eval $(call some_steps_rule,$t,$(APPLICATIONS_STEPS),applications)))

.PHONY: rmbkp
rmbkp:
	find -name *~ -delete
