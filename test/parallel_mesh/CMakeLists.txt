project (pmesh_test)
cmake_minimum_required(VERSION 3.10)


add_executable(pmesh_test test_p_mesh.cpp)

if(PETSc_FOUND)
      include_directories(${PETSc_INCLUDE_DIRS})
endif()


if(MPI_ENABLE)
      find_package(MPI)
      if(MPI_FOUND)
         include_directories(pmesh_test ${MPI_C_INCLUDE_PATH})
         target_link_libraries(pmesh_test PUBLIC MPI::MPI_C MPI::MPI_CXX)
      endif()
endif()

target_link_libraries(pmesh_test 
PRIVATE
meshtoolslib
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
${PETSc_LINK_LIBRARIES}
)
