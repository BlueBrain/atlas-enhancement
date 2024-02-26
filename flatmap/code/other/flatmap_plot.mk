# This makefile is meant to be included by flatmap.mk

# Required utilities
GNUPLOT ?= gnuplot
UNIQ ?= uniq
AWK ?= gawk
 
# Internal program
FLATPATH_FLATDOTS := $(FLATPATH_ROOT)/flatdots.py

.PHONY: flatmap-plot
flatmap-plot: flatmap.png layers_map.png regions_map.png flatmap_histogram.png flatmap_cake.png

PLOT_PREREQS := flatdots_discrete.txt regions.txt FLATMAP_LAYERS_SPLIT.DONE
flatdots_discrete.txt: flatmap.nrrd
	$(LOAD_VOXCELL)
	$(PYTHON3) $(FLATPATH_FLATDOTS) $< $(ATLAS_HIERARCHY) $(ATLAS_REGIONS) \
        regions.txt $@ $(PARENT_REGION)

LAYER_BEST ?= 5
LAYERS_IX ?= 1 2 3 4 5 6
LAYERS_NAME ?= Layer_1 Layer_2 Layer_3 Layer_4 Layer_5 Layer_6
FLATMAP_LAYERS_SPLIT.DONE: $(PLOT_PREREQS)
	awk '{ print $$0 > "flatmap_Layer" $$4 ".txt" }' $<
	touch $@

labels.txt: flatmap_Layer$(LAYER_BEST).txt regions.txt
	$(AWK) '{ r[$$3] = $$3; n[$$3]++; x[$$3] += $$1; y[$$3] += $$2 } \
		END { for(reg in r) print r[reg],x[reg]/n[reg],y[reg]/n[reg] }' $< \
		| $(SORT) -k1g \
		| $(JOIN) -12 $(word 2,$^) - \
		| $(AWK) '{ print $$3,$$4,$$1,$$2 }' \
		> $@

HIST_PIXELS ?= 100
flatdots_discrete_counts.txt: flatmap.txt
	$(AWK) -v npix=$(HIST_PIXELS) '\
		BEGIN { d = 1.0 / npix; } \
		{ x = int($$4/d); y = int($$5/d); h[x,y]++ } \
		END { for(i=0;i<npix;i++) for(j=0;j<npix;j++) { if(h[i,j] != "") print i,j,h[i,j]; else print i,j,0; } }' $< \
		| awk '{ print $$3,$$1,$$2 }' > $@

#flatdots_discrete_counts.txt: flatdots_discrete.txt
#    $(AWK) '{ print $$1,$$2 }' $< | $(SORT) -gk1,1 -gk2,2 | $(UNIQ) -c > $@

PLOT_SCALE ?= 6# inches per subplot
PLOT_NCOLORS ?= words(regions)
PLOT_POINT ?= p pt 7 ps 0.5

## Color palette alternatives

# Color Brewer Categorical
#PLOT_NCOLORS ?= 11
#PLOT_PALETTE ?= (0 '\#a6cee3',1 '\#1f78b4',2 '\#b2df8a',3 '\#33a02c',4 '\#fb9a99',5 '\#e31a1c',6 '\#fdbf6f',7 '\#ff7f00',8 '\#cab2d6',9 '\#6a3d9a',10 '\#ffff99')

# BlueObelisk Elements (from Paraview)
#PLOT_NCOLORS ?= 119
#PLOT_PALETTE ?= (0 '\#117fb2',1 '\#ffffff',2 '\#d8ffff',3 '\#cc7fff',4 '\#c1ff00',5 '\#ffb5b5',6 '\#7f7f7f',7 '\#0c0cff',8 '\#ff0c0c',9 '\#b2ffff',10 '\#b2e2f4',11 '\#aa5bf2',12 '\#89ff00',13 '\#bfa5a5',14 '\#7f9999',15 '\#ff7f00',16 '\#ffff30',17 '\#1eef1e',18 '\#7fd1e2',19 '\#8e3fd3',20 '\#3dff00',21 '\#e5e5e5',22 '\#bfc1c6',23 '\#a5a5aa',24 '\#8999c6',25 '\#9b7ac6',26 '\#7f7ac6',27 '\#707ac6',28 '\#5b7ac1',29 '\#ff7a60',30 '\#7c7faf',31 '\#c18e8e',32 '\#668e8e',33 '\#bc7fe2',34 '\#ffa000',35 '\#a52828',36 '\#5bb7d1',37 '\#702daf',38 '\#00ff00',39 '\#93ffff',40 '\#93e0e0',41 '\#72c1c9',42 '\#54b5b5',43 '\#3a9e9e',44 '\#238e8e',45 '\#0a7c8c',46 '\#006884',47 '\#e0e0ff',48 '\#ffd88e',49 '\#a57572',50 '\#667f7f',51 '\#9e63b5',52 '\#d37a00',53 '\#930093',54 '\#429eaf',55 '\#56168e',56 '\#00c900',57 '\#70d3ff',58 '\#ffffc6',59 '\#d8ffc6',60 '\#c6ffc6',61 '\#a3ffc6',62 '\#8effc6',63 '\#60ffc6',64 '\#44ffc6',65 '\#30ffc6',66 '\#1effc6',67 '\#00ff9b',68 '\#00e575',69 '\#00d351',70 '\#00bf38',71 '\#00aa23',72 '\#4cc1ff',73 '\#4ca5ff',74 '\#2193d6',75 '\#267caa',76 '\#266696',77 '\#165487',78 '\#f4edd1',79 '\#ccd11e',80 '\#b5b5c1',81 '\#a5544c',82 '\#565960',83 '\#9e4fb5',84 '\#aa5b00',85 '\#754f44',86 '\#428296',87 '\#420066',88 '\#007c00',89 '\#70aaf9',90 '\#00baff',91 '\#00a0ff',92 '\#008eff',93 '\#007fff',94 '\#006bff',95 '\#545bf2',96 '\#775be2',97 '\#894fe2',98 '\#a035d3',99 '\#b21ed3',100 '\#b21eba',101 '\#b20ca5',102 '\#bc0c87',103 '\#c60066',104 '\#cc0059',105 '\#d1004f',106 '\#d80044',107 '\#e00038',108 '\#e5002d',109 '\#e80026',110 '\#ea0023',111 '\#ed0021',112 '\#ef001e',113 '\#f2001c',114 '\#f40019',115 '\#f70016',116 '\#f90014',117 '\#fc0011',118 '\#ff000f')
PLOT_PALETTE ?= (0 '\#44ffc6',1 '\#b5b5c1',2 '\#667f7f',3 '\#c60066',4 '\#d37a00',5 '\#00d351',6 '\#ffd88e',7 '\#a3ffc6',8 '\#e00038',9 '\#006bff',10 '\#8999c6',11 '\#ffffc6',12 '\#8e3fd3',13 '\#00baff',14 '\#c1ff00',15 '\#007fff',16 '\#bc0c87',17 '\#0a7c8c',18 '\#cc0059',19 '\#ffff30',20 '\#ffffff',21 '\#00aa23',22 '\#f40019',23 '\#70d3ff',24 '\#006884',25 '\#b20ca5',26 '\#7c7faf',27 '\#1eef1e',28 '\#30ffc6',29 '\#a5a5aa',30 '\#9e4fb5',31 '\#ff0c0c',32 '\#5b7ac1',33 '\#00ff00',34 '\#429eaf',35 '\#56168e',36 '\#00bf38',37 '\#7f9999',38 '\#00e575',39 '\#00ff9b',40 '\#117fb2',41 '\#f4edd1',42 '\#cc7fff',43 '\#54b5b5',44 '\#e5002d',45 '\#93e0e0',46 '\#b2e2f4',47 '\#c6ffc6',48 '\#a52828',49 '\#e0e0ff',50 '\#5bb7d1',51 '\#00a0ff',52 '\#d8ffc6',53 '\#b21ed3',54 '\#545bf2',55 '\#70aaf9',56 '\#00c900',57 '\#7f7f7f',58 '\#428296',59 '\#165487',60 '\#b21eba',61 '\#238e8e',62 '\#565960',63 '\#d1004f',64 '\#702daf',65 '\#aa5bf2',66 '\#c18e8e',67 '\#f70016',68 '\#007c00',69 '\#f2001c',70 '\#267caa',71 '\#ed0021',72 '\#ffb5b5',73 '\#bfc1c6',74 '\#4cc1ff',75 '\#0c0cff',76 '\#60ffc6',77 '\#668e8e',78 '\#7f7ac6',79 '\#ea0023',80 '\#266696',81 '\#9b7ac6',82 '\#d8ffff',83 '\#420066',84 '\#8effc6',85 '\#d80044',86 '\#fc0011',87 '\#707ac6',88 '\#3dff00',89 '\#b2ffff',90 '\#ff7f00',91 '\#7fd1e2',92 '\#894fe2',93 '\#bfa5a5',94 '\#89ff00',95 '\#a57572',96 '\#9e63b5',97 '\#ff000f',98 '\#aa5b00',99 '\#e5e5e5',100 '\#ff7a60',101 '\#93ffff',102 '\#2193d6',103 '\#3a9e9e',104 '\#930093',105 '\#ccd11e',106 '\#72c1c9',107 '\#008eff',108 '\#ef001e',109 '\#754f44',110 '\#a035d3',111 '\#4ca5ff',112 '\#1effc6',113 '\#f90014',114 '\#a5544c',115 '\#775be2',116 '\#bc7fe2',117 '\#ffa000',118 '\#e80026')

# D3 category20
#PLOT_NCOLORS ?= 20
#PLOT_PALETTE ?= (0 '\#1f77b4',1 '\#aec7e8',2 '\#ff7f0e',3 '\#ffbb78',4 '\#2ca02c',5 '\#98df8a',6 '\#d62728',7 '\#ff9896',8 '\#9467bd',9 '\#c5b0d5',10 '\#8c564b',11 '\#c49c94',12 '\#e377c2',13 '\#f7b6d2',14 '\#7f7f7f',15 '\#c7c7c7',16 '\#bcbd22',17 '\#dbdb8d',18 '\#17becf',19 '\#9edae5')

layers_map.png: LAYOUT_ROWS := 2
layers_map.png: LAYOUT_COLS := 3
layers_map.png: REGIONS = $(shell cut -d' ' -f1 regions.txt | tr '\n' ' ')
layers_map.png: PLOT_SIZE ?= ($(LAYOUT_COLS) * $(PLOT_SCALE))in,($(LAYOUT_ROWS) * $(PLOT_SCALE))in
layers_map.png: $(PLOT_PREREQS)
	echo "\
		regions = '$(REGIONS)'; \
		coloring = '$(PLOT_COLORING)'; \
		xmax = $(HIST_PIXELS) - 1; \
		ymax = $(HIST_PIXELS) - 1; \
		set term pngcairo noenh size $(PLOT_SIZE) background '#eeeeee'; \
		set out '$@'; \
		set palette maxcolors $(PLOT_NCOLORS); \
		set palette defined $(PLOT_PALETTE); \
		set size square; \
		unset key; \
		unset colorbox; \
		set cbr [ 0 : $(PLOT_NCOLORS) ]; \
		set xr [ 0 : xmax ]; \
		set yr [ 0 : ymax ]; \
		set multiplot layout 2,3; \
		layers_ix = '$(LAYERS_IX)'; \
		layers_name = '$(LAYERS_NAME)'; \
		do for [i in layers_ix] { \
			set title word(layers_name,int(i)); \
			plot \"flatmap_Layer\".i.\".txt\" u 1:2:(int(word(coloring,int(\$$3 + 1)))) w p pt 7 ps 0.2 lc pal; \
		} \
		" | $(GNUPLOT)

regions_map.png: LAYOUT_ROWS ?= ceil(words(regions)/4.0)
regions_map.png: LAYOUT_COLS ?= 4
regions_map.png: REGIONS = $(shell cut -d' ' -f1 regions.txt | tr '\n' ' ')
regions_map.png: PLOT_SIZE ?= ($(LAYOUT_COLS) * $(PLOT_SCALE))in,($(LAYOUT_ROWS) * $(PLOT_SCALE))in
regions_map.png: $(PLOT_PREREQS)
	echo "\
		regions = '$(REGIONS)'; \
		coloring = '$(PLOT_COLORING)'; \
		xmax = $(HIST_PIXELS) - 1; \
		ymax = $(HIST_PIXELS) - 1; \
		set term pngcairo size $(PLOT_SIZE) background '#eeeeee'; \
		set out '$@'; \
		set palette maxcolors $(PLOT_NCOLORS); \
		set palette defined $(PLOT_PALETTE); \
		set size square; \
		unset key; \
		unset colorbox; \
		set cbr [ 0 : $(PLOT_NCOLORS) ]; \
		set xr [ 0 : xmax ]; \
		set yr [ 0 : ymax ]; \
		set multiplot layout $(LAYOUT_ROWS),$(LAYOUT_COLS); \
		do for [i = 0:words(regions)-1] { \
			set title word(regions,i+1); \
			plot \"< $(AWK) '\$$3 == \".i.\"' $<\" u 1:2:(int(word(coloring,int(\$$3 + 1)))) w p pt 7 ps 0.2 lc pal; \
		} \
		" | $(GNUPLOT)

# get coloring with manual graph adjacency list of flat regions + networkx
# g = nx.read_adjlist('flatmap/results/isocortex/adjlist.txt')
# d = {int(k):v for k,v in nx.equitable_color(g, 10).items()}
# list(dict(sorted(d.items())).values())
flatmap-png: flatmap_Layer$(LAYER_BEST).png
flatmap_Layer$(LAYER_BEST).png flatmap_labels_Layer$(LAYER_BEST).png: REGIONS = $(shell cut -d' ' -f1 regions.txt | tr '\n' ' ')
flatmap_Layer$(LAYER_BEST).png: $(PLOT_PREREQS)
	echo "\
		regions = '$(REGIONS)'; \
		coloring = '$(PLOT_COLORING)'; \
		xmax = $(HIST_PIXELS) - 1; \
		ymax = $(HIST_PIXELS) - 1; \
		set term pngcairo size xmax + 1,ymax + 1 background '#eeeeee'; \
		set out '$@'; \
		set palette maxcolors $(PLOT_NCOLORS); \
		set palette defined $(PLOT_PALETTE); \
		set size square; \
		set margins at screen 0,at screen 1,at screen 0,at screen 1; \
		unset xlab; unset ylab; \
		unset border; \
		unset tics; \
		unset key; \
		unset colorbox; \
		set cbr [ 0 : $(PLOT_NCOLORS) ]; \
		set xr [ 0 : xmax ]; \
		set yr [ 0 : ymax ]; \
		plot 'flatmap_Layer$(LAYER_BEST).txt' u 1:2:(int(word(coloring,int(\$$3 + 1)))) w $(PLOT_POINT) lc pal; \
		" | $(GNUPLOT)

flatmap_labels_Layer$(LAYER_BEST).png: labels.txt $(PLOT_PREREQS)
	echo "\
		regions = '$(REGIONS)'; \
		coloring = '$(PLOT_COLORING)'; \
		xmax = $(HIST_PIXELS) - 1; \
		ymax = $(HIST_PIXELS) - 1; \
		set term pngcairo size xmax + 1,ymax + 1 background '#eeeeee'; \
		set out '$@'; \
		set palette maxcolors $(PLOT_NCOLORS); \
		set palette defined $(PLOT_PALETTE); \
		set size square; \
		set margins at screen 0,at screen 1,at screen 0,at screen 1; \
		unset xlab; unset ylab; \
		unset border; \
		unset tics; \
		unset key; \
		unset colorbox; \
		set cbr [ 0 : words(regions) - 1 ]; \
		set xr [ 0 : xmax ]; \
		set yr [ 0 : ymax ]; \
		plot 'flatmap_Layer$(LAYER_BEST).txt' u 1:2:(int(word(coloring,int(\$$3 + 1)))) w $(PLOT_POINT) lc pal; \
		     '$<' u 1:2:3 w labels font 'Terminus,8'; \
		" | $(GNUPLOT)


#flatmap_histogram.png: XMAX = $(shell $(AWK) '{ print $$2 }' $< | $(SORT) -g | tail -n1)
#flatmap_histogram.png: YMAX = $(shell $(AWK) '{ print $$3 }' $< | $(SORT) -g | tail -n1)
ifeq (1,$(LOG_CB))
cbmax = log(1 + $1)
cbval = log(1 + $1)
else  # LOG_CB == 0
cbmax = $1
cbval = $1
endif # LOG_CB
flatmap_histogram.png: flatdots_discrete_counts.txt
	#$(AWK) '\
	#    BEGIN { for(i=0;i<=$(XMAX);i++) for(j=0;j<=$(YMAX);j++) n[i][j] = 0; } \
	#    { n[$$2][$$3] = $$1 } \
	#    END { for(i=0;i<$(XMAX);i++) for(j=0;j<=$(YMAX);j++) print i,j,n[i][j] } \
	#    ' $< \
	#    | $(AWK) 'BEGIN { i = -1 } { if($$1 != i) print ""; print; i = $$1 }' > $@.tmp
	$(AWK) '{ print $$2,$$3,$$1 }' $< > $@.tmp
	echo "\
		set term pngcairo size 20in,10in; \
		set out '$@'; \
		set cblab 'Voxel incidence count'; \
		stats '$<' u 2:3 nooutput; \
		xmax = STATS_max_x; \
		ymax = STATS_max_y; \
		stats '$<' u 1 nooutput; \
		cmax = STATS_max; \
		set xr [0 : xmax + 1] noext; \
		set yr [0 : ymax + 1] noext; \
		set cbr [0 : $(call cbmax,cmax)] noext; \
		load '$(FLATPATH_ROOT)/viridis.pal'; \
		set multiplot layout 1,2; \
		set size square; \
		unset key; \
		plot '$@.tmp' u (\$$1 + 0.5):(\$$2 + 0.5):($(call cbval,\$$3)) w image; \
		set size nosquare; \
		set ytics nomirror; \
		set y2tics nomirror; \
		set key; \
        set xr [0:*]; \
        set yr [0:*]; \
		set y2r [0:1]; \
		plot '$<' u 1:(1.0) w histeps tit 'Histogram' smooth fnormal, \
		     '$<' u 1:(1.0) w l tit 'Cumulative' smooth cnormal axes x1y2; \
		unset multiplot; \
		" | gnuplot
	$(RM) $@.tmp

PLOT_VIEW ?= 0,0
XRANGE ?= [*:*]
YRANGE ?= [*:*]
PLOT_NCOLS ?= 10

flatmap_collage.png: flatmap.txt
	echo "\
		set term pngcairo size 25in,10in; \
		set out '$@'; \
		stats '$<' u 1:2 nooutput; \
		xmax = STATS_max_x; \
		ymax = STATS_max_y; \
		unset key; \
		set view $(PLOT_VIEW); \
		set view noequal; \
		set xr [0 : xmax]; \
		set yr [0 : ymax]; \
		set hidden3d; \
		set multiplot layout ceil($(PLOT_NCOLS)/5.0),5; \
		do for [i = 0:$(PLOT_NCOLS) - 1] { \
			tit = sprintf('%d <= x < %d',i,i+1); \
			set title tit; \
			splot '< awk \"{ print \\\$$1,\\\$$2,\\\$$3,int(\\\$$4 * ($(PLOT_NCOLS) - 1E-6)),int(\\\$$5 * ($(PLOT_NCOLS) - 1E-6)) }\" $< | awk \"\\\$$4 == '.i.'{ print }\" | shuf -n 30000' \
			      u 1:2:3:5 w p pt 7 ps 0.3 lc var; \
		}; \
		unset multiplot; \
		" | gnuplot

flatmap_cake.png: flatmap.txt
	echo "\
		set term pngcairo size 25in,10in; \
		set out '$@'; \
		unset key; \
		set view $(PLOT_VIEW); \
		set view noequal; \
		set xr $(XRANGE); \
		set yr $(YRANGE); \
		set hidden3d; \
		splot '< awk \"BEGIN { f = $(PLOT_NCOLS) - 1E-6 } { ix = int(\\\$$4 * f); iy = int(\\\$$5 * f); print \\\$$1,\\\$$2,\\\$$3,ix,iy,(ix + 3 * iy) % 8 }\" $<' \
			  u 1:2:3:6 w p pt 7 ps 0.5 lc var; \
		" | gnuplot

