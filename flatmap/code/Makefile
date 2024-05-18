ALL_BIN := surf_cgal/build/Reconstruct_PCAlpha\
		   flatpath/flatpath

all: $(ALL_BIN)

surf_cgal/build/Reconstruct_PCAlpha:
	cd surf_cgal && $(MAKE) compile

surf_scal/build_mkl/FlattenAuthalic_Iterative:
	cd surf_cgal && $(MAKE) compile-mkl

flatpath/flatpath:
	cd flatpath && $(MAKE) flatpath
