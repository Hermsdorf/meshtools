cmake_minimum_required(VERSION 3.20.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)
project(meshtools)
include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

#set(CMAKE_CXX_COMPILER g++)
# set(CMAKE_CXX_FLAGS "-g -fopenmp -O3")

option(MESHTOOLS_BUILD_APP  "Build meshtools application" ON)
option(MPI_ENABLE       "Enable MPI" ON)
option(OpenMP_ENABLE    "Enable OpenMP" ON)
option(CATALYST_ENABLE  "Enable Paraview Catalyst" OFF)
option(DEBUG_ENABLE  "Enable Paraview Catalyst" OFF)



if(DEBUG_ENABLE)
  set(DEBUG_ 1)
endif()

find_package(METIS REQUIRED)
find_package(HDF5)

include(FindPkgConfig)

if(MPI_ENABLE)
  find_package(MPI)
  if(MPI_FOUND)
     include_directories(${MPI_C_INCLUDE_PATH})
     set(MPI_LIBS MPI::MPI_C MPI::MPI_CXX)
     set(MPI_ENABLE 1)
  endif()
endif()

if(DEFINED ENV{PETSC_DIR})
  set (ENV{PKG_CONFIG_PATH} "$ENV{PETSC_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  pkg_search_module(PETSc REQUIRED PETSc>3.10)
  if(PETSc_FOUND)
     message(STATUS "PETSc ${PETSc_VERSION} was found!")
     message(STATUS "PETSc_INCLUDE: ${PETSc_INCLUDE_DIRS} ")
     message(STATUS "PETSc_LIBRARIES: ${PETSc_LINK_LIBRARIES} ")
     SET(PETSC_ENABLE 1)
  endif()
else()
  message(STATUS "PETSc was not founded! Please define PETSc_DIR var!!")
endif()

if(PETSc_FOUND)
include_directories(${PETSc_INCLUDE_DIRS})
set(PETSC_LIBS ${PETSc_LINK_LIBRARIES})
endif()

if(HDF5_FOUND)
include_directories(${HDF5_INCLUDE_DIR})
set(HDF5_LIBS ${HDF5_C_LIBRARIES})
#SET(HDF5_ENABLE 1)
endif()

SET(MESHTOOLS_INCLUDE_DIR ${PETSc_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR} ${MPI_C_INCLUDE_PATH} ${METIS_INCLUDE_DIR})
SET(MESHTOOLS_LIBS ${PETSc_LINK_LIBRARIES} ${MPI_LIBS} ${HDF5_LIBS} ${METIS_LIBRARIES})

include_directories(${METIS_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/meshtoolslib/include)
include_directories(${CMAKE_SOURCE_DIR}/femsolverlib/include)
include_directories(${CMAKE_SOURCE_DIR}/contrib/fparser)

# Compilando bibliotecas de terceitos
add_subdirectory(${CMAKE_SOURCE_DIR}/contrib/fparser)

# Compilar as bibliotecas locais
add_subdirectory(meshtoolslib)
add_subdirectory(femsolverlib)
# add_subdirectory(app)
add_subdirectory(test)

if(MESHTOOLS_BUILD_APP)
 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
add_test(Help ./app/meshtools -h)
add_test(Read:Ascii  ./app/meshtools -m    ../test/io/ascii/quadrado.msh)
add_test(Read:Binary ./app/meshtools -m    ../test/io/binary/quadrado.msh)
add_test(Reordering:RCM ./app/meshtools -m ../test/io/binary/quadrado.msh -r rcm)
add_test(Reordering:ND ./app/meshtools -m  ../test/io/binary/quadrado.msh -r nd)

endif()


include(CPack)
