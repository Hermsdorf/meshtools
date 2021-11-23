cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)

project(meshtools)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-g -fopenmp -O3")

option(MPI_ENABLE       "Enable MPI" ON)
option(OpenMP_ENABLE    "Enable OpenMP" ON)
option(CATALYST_ENABLE  "Enable Paraview Catalyst" OFF)
add_subdirectory(lib)

file(GLOB source_files_exec "main.cpp")

add_executable(meshtools ${source_files_exec})


if(MPI_ENABLE)
   find_package(MPI)
   if(MPI_FOUND)
	   include_directories(meshtools ${MPI_C_INCLUDE_PATH})
	   target_link_libraries(meshtools PUBLIC MPI::MPI_C MPI::MPI_CXX)
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

#configure_file(meshtools_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/meshtools_config.h @ONLY)

# Finding Depedencies
# Looking for Metis Library

include_directories(${METIS_INCLUDE_DIRS})
include_directories(lib/include)

if(OpenMP_ENABLE)
   find_package(OpenMP)
   if(OpenMP_CXX_FOUND)
      target_link_libraries(meshtools PUBLIC OpenMP::OpenMP_CXX)
   endif()
endif()

target_link_libraries(meshtools 
PRIVATE
meshtoolslib
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
