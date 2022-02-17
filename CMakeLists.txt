cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)
project(meshtools)
include(CTest)
enable_testing()



set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-g -fopenmp -O3")

option(MESHTOOLS_BUILD_APP  "Build meshtools application" ON)
option(MPI_ENABLE       "Enable MPI" ON)
option(OpenMP_ENABLE    "Enable OpenMP" ON)
option(CATALYST_ENABLE  "Enable Paraview Catalyst" OFF)

find_package(METIS REQUIRED)


include(FindPkgConfig)

if(DEFINED ENV{PETSC_DIR})
  set (ENV{PKG_CONFIG_PATH} "$ENV{PETSC_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  pkg_search_module(PETSc REQUIRED PETSc>3.10)
  if(PETSc_FOUND)
     message(STATUS "PETSc ${PETSc_VERSION} was found!")
     message(STATUS "PETSc_INCLUDE: ${PETSc_INCLUDE_DIRS} ")
     message(STATUS "PETSc_LIBRARIES: ${PETSc_LINK_LIBRARIES} ")
  endif()
else()
  message(STATUS "PETSc was not founded! Please define PETSc_DIR var!!")
endif()

#include_directories(${PETSC_INCLUDES})
#add_definitions(${PETSC_DEFINITIONS})
include_directories(${METIS_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/meshtoolslib/include)

add_subdirectory(meshtoolslib)

if(MESHTOOLS_BUILD_APP)
 
add_subdirectory(app)
add_subdirectory(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
add_test(Help ./app/meshtools -h)
add_test(Read:Ascii  ./app/meshtools -m    ../test/io/ascii/quadrado.msh)
add_test(Read:Binary ./app/meshtools -m    ../test/io/binary/quadrado.msh)
add_test(Reordering:RCM ./app/meshtools -m ../test/io/binary/quadrado.msh -r rcm)
add_test(Reordering:ND ./app/meshtools -m  ../test/io/binary/quadrado.msh -r nd)

endif()


include(CPack)
