
add_executable(poisson poisson_2d.cpp)
if(MPI_FOUND)
include_directories(poisson ${MPI_C_INCLUDE_PATH})
target_link_libraries(poisson PUBLIC MPI::MPI_C MPI::MPI_CXX)
endif()

target_link_libraries(poisson 
   PRIVATE
   meshtoolslib
   femsolverlib
   fparser
   ${ADDITIONAL_ITEMS}
   ${METIS_LIBRARIES}
   ${PETSc_LINK_LIBRARIES}
   )
