cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)

project(meshtools)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-g -fopenmp -O2")

find_package(METIS REQUIRED)


file(GLOB source_files
            "*.h"
            "*.cpp"
            "src/*.cpp"
            "include/*.h"
            "test/finite_element_kernel/*.cpp"
    )

option(MPI_ENABLE "Enable MPI" OFF)
option(OpenMP_ENABLE "Enable OpenMP" OFF)
option(CATALYST_ENABLE "Enable Paraview Catalyst" OFF)

if(MPI_ENABLE)
   find_package(MPI)
   if(MPI_FOUND)
      target_link_libraries(meshtools PUBLIC MPI::C)
   endif()
   set(USE_MPI 1)
endif()

if(CATALYST_ENABLE)
   find_package(ParaView CONFIG COMPONENTS Catalyst PythonCatalyst)
   if (${ParaView_FOUND})
      MESSAGE(STATUS "Found ParaView Catalyst.")
      set(ADDITIONAL_ITEMS ParaView::Catalyst ParaView::PythonCatalyst VTK::CommonDataModel)
      set(USE_CATALYST 1)
   else (${ParaView_FOUND})
      list(FILTER source_files EXCLUDE REGEX "include/FEAdaptor.h")
      list(FILTER source_files EXCLUDE REGEX "src/FEAdaptor.cpp")
   endif (${ParaView_FOUND})
else()
   list(FILTER source_files EXCLUDE REGEX "include/FEAdaptor.h")
   list(FILTER source_files EXCLUDE REGEX "src/FEAdaptor.cpp")
endif()

configure_file(meshtools_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/meshtools_config.h @ONLY)

# Finding Depedencies
# Looking for Metis Library

include_directories(${METIS_INCLUDE_DIRS})
include_directories(include)
include_directories(test/finite_element_kernel)

add_executable(meshtools ${source_files})

if(OpenMP_ENABLE)
   find_package(OpenMP)
   if(OpenMP_CXX_FOUND)
      target_link_libraries(meshtools PUBLIC OpenMP::OpenMP_CXX)
   endif()
endif()

target_link_libraries(meshtools 
PRIVATE
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
