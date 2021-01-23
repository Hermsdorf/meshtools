cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)

project(meshtools)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
#set(CMAKE_C_COMPILER tau_cc.sh)
#set(CMAKE_CXX_COMPILER tau_cxx.sh)
set (CMAKE_CXX_FLAGS "-fopenmp -g")

#find_package(MPI)
find_package(METIS REQUIRED)
find_package(ParaView CONFIG COMPONENTS Catalyst PythonCatalyst)

file(GLOB source_files
            "*.h"
            "*.cpp"
            "src/*.cpp"
            "include/*.h"
    )

if (${ParaView_FOUND})
   MESSAGE(STATUS "Found ParaView Catalyst.")
   set(ADDITIONAL_ITEMS ParaView::Catalyst ParaView::PythonCatalyst VTK::CommonDataModel)
   add_definitions(-DPARAVIEWCAT_FOUND=1)
else (${ParaView_FOUND})
   list(FILTER source_files EXCLUDE REGEX "include/FEAdaptor.h")
   list(FILTER source_files EXCLUDE REGEX "src/FEAdaptor.cpp")
endif (${ParaView_FOUND})

# Finding Depedencies
# Looking for Metis Library

include_directories(${METIS_INCLUDE_DIRS})
include_directories(include)

add_executable(meshtools ${source_files})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(meshtools PUBLIC OpenMP::OpenMP_CXX)
endif()

target_link_libraries(meshtools 
PRIVATE
#MPI::MPI_C
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)