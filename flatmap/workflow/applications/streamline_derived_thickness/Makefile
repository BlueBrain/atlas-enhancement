include ../../common_header.mk

output/$(STREAMLINE_DERIVED_THICKNESS_FILE): input/$(RELATIVE_DEPTH_FILE) output/height_notnan.txt
	$(SETVOXELS_BIN) $< $(word 2,$^) $@

output/height_notnan.txt: output/height.txt
	$(AWK) '$$4 != "nan"' $< > $@

ifdef FLATPATH_TIMESTEP
FLATPATH_FLAGS += -h $(FLATPATH_TIMESTEP)
endif

ifdef FLATPATH_MAXITER
FLATPATH_FLAGS += --maxiter $(FLATPATH_MAXITER)
endif

ifdef FLATPATH_EPSILON_ZERO
FLATPATH_FLAGS += --epsilon-zero $(FLATPATH_EPSILON_ZERO)
endif

ifdef FLATPATH_EPSILON_ONE
FLATPATH_FLAGS += --epsilon-one $(FLATPATH_EPSILON_ONE)
endif

ifdef FLATPATH_NSTEP_PATH
FLATPATH_FLAGS += --nstep-path $(FLATPATH_NSTEP_PATH)
endif

output/height.txt: BLOCKSIZE := $(FLATPATH_BLOCKSIZE)
output/height.txt: NVOX = $(shell $(HEAD) -1 $(word 5,$^))
output/height.txt: NBLOCK = $(shell $(EXPR) $(NVOX) / $(BLOCKSIZE) + \( $(NVOX) % $(BLOCKSIZE) \> 0 \))

ifeq (1,$(PARALLELIZE))
NJOB ?= 0
output/height.txt: $(CONVERTED_INPUTS) output/$(DIMENSIONS_FILE)
	$(SEQ) 1 $(NBLOCK) \
		| $(AWK) '{ print ($$1 - 1) * $(BLOCKSIZE),$$1 * $(BLOCKSIZE) }' \
		| $(PARALLEL) --lb --halt soon,fail=1 -C' ' -j$(NJOB) \
			$(SRUN) $(MAKE) streamline-block BLOCK_FIRST={1} BLOCK_LAST={2} BLOCK_OUTPUT=$@.{1} \
		&& $(CAT) $@.* > $@
	$(RM) $@.*
else
output/height.txt: $(CONVERTED_INPUTS) output/$(DIMENSIONS_FILE)
	for i in `$(SEQ) 1 $(NBLOCK)`; do
		first=`$(EXPR) \( $$i - 1 \) \* $(BLOCKSIZE)`
		last=`$(EXPR) $$i \* $(BLOCKSIZE)`
		$(MAKE) streamline-block BLOCK_FIRST=$$first BLOCK_LAST=$$last BLOCK_OUTPUT=$@.$$i
	done \
		&& $(CAT) $@.* > $@
	$(RM) $@.*
endif

.PHONY: streamline-block
.DELETE_ON_ERROR: $(BLOCK_OUTPUT)
streamline-block: $(CONVERTED_INPUTS) output/$(DIMENSIONS_FILE)
	dim=(`$(SED) '2q;d' $(word 5,$^)`) # atlas dimensions
	vox=(`$(SED) '3q;d' $(word 5,$^)`) # voxel size
	off=(`$(SED) '4q;d' $(word 5,$^)`) # offset
	$(FLATPATH_BIN) \
		-l $(word 1,$^) -x $(word 2,$^) -y $(word 3,$^) -z $(word 4,$^) \
		-i $${dim[0]} -j $${dim[1]} -k $${dim[2]} \
		-u $${vox[0]} -v $${vox[1]} -w $${vox[2]} \
		-a $${off[0]} -b $${off[1]} -c $${off[2]} \
		--first $(BLOCK_FIRST) --last $(BLOCK_LAST) \
		--height \
		$(FLATPATH_FLAGS) \
		$(FLATPATH_EXTRA) \
		-o $(BLOCK_OUTPUT)

# auxiliary files
$(CONVERTED_INPUTS): output/%.bin.gz: $(USER_DATA_ROOT)/%.nrrd
	$(NRRD_TO_BINGZ_BIN) $< output/$*

output/$(DIMENSIONS_FILE): input/$(RELATIVE_DEPTH_FILE)
	$(GET_DIMENSIONS_BIN) $< $@

# auxiliary targets
view: view-nrrd

include ../../common_footer.mk
