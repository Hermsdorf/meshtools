cmake_minimum_required(VERSION 3.0.0)

project(meshtools)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

file(GLOB source_files
            "*.h"
            "*.cpp"
            "src/*.cpp"
            "include/*.h"
    )


add_executable(meshtools ${source_files})
include_directories(include)

# Finding Depedencies
# Looking for Metis Library

find_package(METIS REQUIRED)
include_directories(${METIS_INCLUDE_DIRS})

find_package(MPI)
find_package(ParaView CONFIG REQUIRED)
INCLUDE(${PARAVIEW_USE_FILE})

set(LIBS ${LIB} "${METIS_LIBRARIES}")
target_link_libraries(meshtools ${LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
