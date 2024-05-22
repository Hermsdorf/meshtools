cmake_minimum_required(VERSION 3.20.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 17)
project(meshtools)
include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)


option(MESHTOOLS_BUILD_APP  "Build meshtools application" ON)
option(MPI_ENABLE           "Enable MPI" ON)
option(OpenMP_ENABLE        "Enable OpenMP" ON)
option(CATALYST_ENABLE      "Enable Paraview Catalyst" OFF)
if(CATALYST_ENABLE)
 option(CATALYST_LEGACY_ENABLE "Enable Paraview Catalyst Legacy" OFF)
endif()
option(DEBUG_ENABLE         "Enable Debug Mode" ON)
option(DEBUG_OUTPUT_ENABLE  "Enable Write Debug at File" OFF)

set(CMAKE_CXX_FLAGS "-fPIC")


if(DEBUG_ENABLE)
  add_definitions(-DNDEBUG)  
endif()

if(DEBUG_OUTPUT_ENABLE) 
  add_definitions(-DDEBUG_OUTPUT)
endif()

find_package(METIS REQUIRED)
find_package(HDF5)

if(CATALYST_ENABLE)
  if(CATALYST_LEGACY_ENABLE)
    find_package(ParaView REQUIRED)
    if (NOT TARGET ParaView::PythonCatalyst)
      message(STATUS
        "Skipping example: ${PROJECT_NAME} requires ParaView to be built "
        "with Catalyst and Python support enabled. Please rebuild ParaView (or "
        "point to a different build of ParaView) with PARAVIEW_USE_PYTHON set "
        "to TRUE")
      return ()
    endif()

    if (NOT PARAVIEW_USE_MPI)
      message(STATUS
        "Skipping example: ${PROJECT_NAME} requires ParaView to be built "
        "with MPI support enabled. Please rebuild ParaView (or point to a "
        "different build of ParaView) with PARAVIEW_USE_MPI set to TRUE")
      return ()
    endif ()
    SET(CATALYST_LEGACY_ENABLE 1)
  else()  
    find_package(catalyst 2.0 REQUIRED PATHS "$catalist_DIR")
    if(catalyst_FOUND)
        message(STATUS "Catalyst was found!")
        SET(CATALYST_ENABLE 1)
        if(NOT DEFINED Paraview_Dir)
          message(FATAL_ERROR "Paraview_Dir is not defined!")
        endif()
        set(PARAVIEW_IMPL_DIR "${Paraview_Dir}/lib/catalyst")
        set(CATALYST_LIBS catalyst::catalyst)
    endif()
  endif()
endif()

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
  #set (ENV{PKG_CONFIG_PATH} "/opt/lib/petsc/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  pkg_search_module(PETSc REQUIRED PETSc>3.10)
  if(PETSc_FOUND)
     message(STATUS "PETSc ${PETSc_VERSION} was found!")
     message(STATUS "PETSc_INCLUDE: ${PETSc_INCLUDE_DIRS} ")
     message(STATUS "PETSc_LIBRARIES: ${PETSc_LINK_LIBRARIES} ")
     SET(PETSC_ENABLE 1)
  endif()
else()
  message(STATUS "PETSc was not founded! Please define PETSC_DIR var!!")
endif()

if(PETSc_FOUND)
  include_directories(${PETSc_INCLUDE_DIRS})
  set(PETSC_LIBS ${PETSc_LINK_LIBRARIES})
endif()

if(HDF5_FOUND)
  include_directories(${HDF5_INCLUDE_DIR})
  set(HDF5_LIBS ${HDF5_C_LIBRARIES})
  SET(HDF5_ENABLE 1)
endif()

SET(MESHTOOLS_DEP_INCLUDE_DIR ${PETSc_INCLUDE_DIRS} ${HDF5_INCLUDE_DIR} ${MPI_C_INCLUDE_PATH} ${METIS_INCLUDE_DIR})
SET(MESHTOOLS_DEP_LIBS ${PETSc_LINK_LIBRARIES} ${MPI_LIBS} ${HDF5_LIBS} ${METIS_LIBRARIES} ${CATALYST_LIBS})

SET(MESHTOOLS_SOURCE_DIR ${CMAKE_SOURCE_DIR})

include_directories(${MESHTOOLS_DEP_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include/base)
include_directories(${CMAKE_SOURCE_DIR}/include/mesh)
include_directories(${CMAKE_SOURCE_DIR}/include/io)
include_directories(${CMAKE_SOURCE_DIR}/include/numerics)
include_directories(${CMAKE_SOURCE_DIR}/include/fem)


include_directories(${CMAKE_SOURCE_DIR}/contrib/fparser)


configure_file(${CMAKE_SOURCE_DIR}/config/meshtools_config.h.in ${CMAKE_SOURCE_DIR}/include/base/meshtools_config.h @ONLY)

# Compilando bibliotecas de terceitos
add_subdirectory(${CMAKE_SOURCE_DIR}/contrib/fparser)
if(CATALYST_LEGACY_ENABLE)
  add_subdirectory(${CMAKE_SOURCE_DIR}/contrib/catalyst)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/contrib/pugixml-1.14)



file(GLOB base_files
         "include/base/*.h"
         "include/base/*.hpp"
         "src/base/*.cpp"
    )

file(GLOB mesh_files
         "include/mesh/*.h"
         "include/mesh/*.hpp"
         "src/mesh/*.cpp"
    )

file(GLOB io_files
    "include/io/*.h"
    "include/io/*.hpp"
    "src/io/*.cpp"
)

file(GLOB numerics_files
    "include/numerics/*.h"
    "include/numerics/*.hpp"
    "src/numerics/*.cpp"
)

file(GLOB fem_files
    "include/fem/*.h"
    "include/fem/*.hpp"
    "src/fem/*.cpp"
)
    
add_library(meshtoolslib SHARED ${base_files} ${mesh_files} ${io_files} ${numerics_files} ${fem_files})

target_link_libraries(meshtoolslib fparser ${MESHTOOLS_DEP_LIBS})

add_subdirectory(test)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})



include(CPack)
