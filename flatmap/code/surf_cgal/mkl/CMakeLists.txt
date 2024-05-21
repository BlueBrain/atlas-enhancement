# Created by the script cgal_create_CMakeLists
# This is the CMake script for compiling a set of CGAL applications.

cmake_minimum_required(VERSION 3.1...3.15)

project( surf_cgal )


# CGAL and its components
find_package( CGAL QUIET COMPONENTS  )

if ( NOT CGAL_FOUND )

  message(STATUS "This project requires the CGAL library, and will not be compiled.")
  return()

endif()


# Boost and its components
find_package( Boost REQUIRED )

if ( NOT Boost_FOUND )

  message(STATUS "This project requires the Boost library, and will not be compiled.")
  return()

endif()

create_single_source_cgal_program( "Flatten_Authalic_Iterative_convex.cxx" )
create_single_source_cgal_program( "Flatten_Authalic_Iterative_parametric.cxx" )
create_single_source_cgal_program( "Flatten_Authalic_Iterative.cxx" )


# Eigen3 and its components
find_package(Eigen3 3.1.0) #(requires 3.1.0 or greater)
include(CGAL_Eigen3_support)
if(TARGET CGAL::Eigen3_support)

    target_link_libraries(Flatten_Authalic_Iterative_convex PUBLIC CGAL::Eigen3_support)
    target_link_libraries(Flatten_Authalic_Iterative_parametric PUBLIC CGAL::Eigen3_support)
    target_link_libraries(Flatten_Authalic_Iterative PUBLIC CGAL::Eigen3_support)

else()

  message(STATUS "NOTICE: This project requires Eigen 3.1 (or greater), and will not be compiled.")
  return()

endif()


# Intel TBB and its components
find_package(TBB)
include(CGAL_TBB_support)
if(TARGET CGAL::TBB_support)

    message(STATUS "NOTICE: Intel TBB will be used for this project.")
    target_link_libraries(Flatten_Authalic_Iterative_convex PUBLIC CGAL::TBB_support)
    target_link_libraries(Flatten_Authalic_Iterative_parametric PUBLIC CGAL::TBB_support)
    target_link_libraries(Flatten_Authalic_Iterative PUBLIC CGAL::TBB_support)

else()

    message(STATUS "NOTICE: Intel TBB will not be used for this project.")

endif()


# OpenMP for Eigen
find_package(OpenMP)
if(OpenMP_CXX_FOUND)

    target_link_libraries(Flatten_Authalic_Iterative_convex PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(Flatten_Authalic_Iterative_parametric PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(Flatten_Authalic_Iterative PUBLIC OpenMP::OpenMP_CXX)

endif()

# Intel MKL for Eigen
find_package(MKL CONFIG REQUIRED)
if(1)

    target_compile_options(Flatten_Authalic_Iterative_convex PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_COMPILE_OPTIONS>)
    target_include_directories(Flatten_Authalic_Iterative_convex PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(Flatten_Authalic_Iterative_convex PUBLIC $<LINK_ONLY:MKL::MKL>)

    target_compile_options(Flatten_Authalic_Iterative_parametric PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_COMPILE_OPTIONS>)
    target_include_directories(Flatten_Authalic_Iterative_parametric PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(Flatten_Authalic_Iterative_parametric PUBLIC $<LINK_ONLY:MKL::MKL>)

    target_compile_options(Flatten_Authalic_Iterative PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_COMPILE_OPTIONS>)
    target_include_directories(Flatten_Authalic_Iterative PUBLIC $<TARGET_PROPERTY:MKL::MKL,INTERFACE_INCLUDE_DIRECTORIES>)
    target_link_libraries(Flatten_Authalic_Iterative PUBLIC $<LINK_ONLY:MKL::MKL>)

endif()
