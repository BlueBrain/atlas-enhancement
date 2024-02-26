-include config.mk

ifndef CGAL_ROOT
$(error Please set CGAL_ROOT)
endif

.PHONY: compile compile-mkl

compile: | build/
	cd build && cmake .. \
		-DCMAKE_BUILD_TYPE=Release\
		-DCGAL_DIR=$(CGAL_ROOT)\
		$(CMAKE_EXTRA_FLAGS)
	cd build && make

ifdef MKLROOT
compile-mkl: | build_mkl/
	cd build_mkl && cmake ../mkl \
		-DCMAKE_BUILD_TYPE=Release \
		-DCGAL_DIR=$(CGAL_ROOT) \
		-DMKL_DIR=$(MKLROOT)/lib/cmake/mkl \
		-DMKL_INTERFACE_FULL=intel_lp64 \
		$(CMAKE_EXTRA_FLAGS)
	cd build_mkl && make
else
compile-mkl:
	$(error Please set MKLROOT)
endif

%/:
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r build

clean-mkl:
	$(RM) -r build_mkl
