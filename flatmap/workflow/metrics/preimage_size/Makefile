include ../../common_header.mk

output/$(METRICS_PREIMAGE_SIZE_FILE): input/$(NEAREST_VERTICES_FILE) input/$(FLAT_MESH_FILE)
	$(AWK) 'NR == 2 { nv = $$1 } NR > 2 && NR <= nv + 2 { print "v" NR - 3,$$1,$$2 }' $(word 2,$^) \
		| $(AWK) 'BEGIN { while(getline < ARGV[2] > 0) { pos[$$1] = $$2 OFS $$3 } ARGC-- }  \
		    { cnt[$$1]++ } END { for(k in cnt) { print pos[k],cnt[k] } }' $< - \
		> $@

# input links
input/$(FLAT_MESH_FILE): SRC := $(STAGE_I_ROOT)/04_flatten_mesh/output/$(FLAT_MESH_FILE)
input/$(NEAREST_VERTICES_FILE): SRC := $(STAGE_III_ROOT)/01_nearest/output/$(NEAREST_VERTICES_FILE)

# auxiliary commands
view: view-heatmap

view-heatmap:
	echo -e "\
set palette gray negative\n\
set size square\n\
set xr [0:1]\n\
set yr [0:1]\n\
plot 'output/$(METRICS_PREIMAGE_SIZE_FILE)' u 1:2:3 w d lc pal" > .tmp.gp
	$(GNUPLOT) -p .tmp.gp -
	$(RM) .tmp.gp

include ../../common_footer.mk
