cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0057 NEW)


project(meshtools)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)
set(CMAKE_CXX_FLAGS "-O3")
#set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
#set(CMAKE_C_COMPILER tau_cc.sh)
#set(CMAKE_CXX_COMPILER tau_cxx.sh)

file(GLOB source_files
            "*.h"
            "*.cpp"
            "src/*.cpp"
            "include/*.h"
    )


# Finding Depedencies
# Looking for Metis Library

find_package(MPI)
find_package(METIS REQUIRED)

find_package(ParaView CONFIG COMPONENTS Catalyst PythonCatalyst)

include_directories(${METIS_INCLUDE_DIRS})
include_directories(include)

add_executable(meshtools ${source_files})

target_link_libraries(meshtools 
PRIVATE
ParaView::Catalyst 
ParaView::PythonCatalyst 
VTK::CommonDataModel 
MPI::MPI_C
${METIS_LIBRARIES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
