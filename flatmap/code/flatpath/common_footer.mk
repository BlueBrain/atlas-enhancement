remake: clean all

# cmod pattern rules
%.c : %.cm
	$(CMOD) $(CMODFLAGS) $(CMOD_INCLUDES) -o $@ $<

%.h : %.hm
	$(CMOD) $(CMODFLAGS) $(CMOD_INCLUDES) -o $@ $<

%.l : %.lm
	$(CMOD) -g $(CMODFLAGS) -o $@ $<

%.y : %.ym
	$(CMOD) -g $(CMODFLAGS) -o $@ $<

# tests
.PHONY: test test-case
.SILENT: test-case
test-case:
	mkdir test_$$$$; \
	$(MAKE) test_$$$$/$(TEST_CASE) TEST_ROOT=test_$$$$ && cmp test_$$$$/$(TEST_CASE) tests/$(TEST_CASE); \
	ret=$$?; rm -rf test_$$$$; exit $$ret

test: all
	$(foreach t,$(TESTS),$(MAKE) test-case TEST_CASE=$t;)

# install
.PHONY: install install-notest
install: test
install-notest:
install install-notest:
	$(MAKE) install-bin install-lib install-inc install-mod

.PHONY: install-bin
ifdef BIN_TARGETS
install-bin: $(BIN_TARGETS)
	$(INSTALL) -d $(BIN_PREFIX)
	$(INSTALL) -m755 $^ $(BIN_PREFIX)
else
install-bin:
endif

.PHONY: install-lib
ifdef LIB_TARGETS
install-lib: $(LIB_TARGETS)
	$(INSTALL) -d $(LIB_PREFIX)
	$(INSTALL) -m644 $^ $(LIB_PREFIX)
else
install-lib:
endif

.PHONY: install-inc
ifdef INC_TARGETS
install-inc: $(INC_TARGETS)
	$(INSTALL) -d $(INC_PREFIX)
	$(INSTALL) -m644 $^ $(INC_PREFIX)
else
install-inc:
endif

.PHONY: install-mod
ifdef MOD_TARGETS
install-mod: $(MOD_TARGETS)
	$(INSTALL) -d $(MOD_PREFIX)
	$(INSTALL) -m644 $^ $(MOD_PREFIX)
else
install-mod:
endif

# uninstall
.PHONY: uninstall
uninstall: uninstall-bin uninstall-lib uninstall-inc uninstall-mod

.PHONY: uninstall-bin
ifdef BIN_TARGETS
uninstall-bin:
	cd $(BIN_PREFIX) && rm -f $(BIN_TARGETS)
else
uninstall-bin:
endif

.PHONY: uninstall-lib
ifdef LIB_TARGETS
uninstall-lib:
	cd $(LIB_PREFIX) && rm -f $(LIB_TARGETS)
else
uninstall-lib:
endif

.PHONY: uninstall-inc
ifdef INC_TARGETS
uninstall-inc:
	cd $(INC_PREFIX) && rm -f $(INC_TARGETS)
else
uninstall-inc:
endif

.PHONY: uninstall-mod
ifdef MOD_TARGETS
uninstall-mod:
	cd $(MOD_PREFIX) && rm -f $(MOD_TARGETS)
else
uninstall-mod:
endif

# clean
.PHONY: clean
clean:
	$(RM) $(BIN_TARGETS) $(LIB_TARGETS) $(INC_TARGETS) $(OBJS) $(CLEAN_EXTRA) *~
