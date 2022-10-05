
add_executable(poisson poisson_2d.cpp)
add_executable(conv2d convection_diffusion_2d.cpp)
add_executable(transport transport.cpp)
if(MPI_FOUND)
include_directories(poisson ${MPI_C_INCLUDE_PATH})
target_link_libraries(poisson PUBLIC MPI::MPI_C MPI::MPI_CXX)

include_directories(conv2d ${MPI_C_INCLUDE_PATH})
target_link_libraries(conv2d PUBLIC MPI::MPI_C MPI::MPI_CXX)

include_directories(transport ${MPI_C_INCLUDE_PATH})
target_link_libraries(transport PUBLIC MPI::MPI_C MPI::MPI_CXX)

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


target_link_libraries(conv2d 
PRIVATE
meshtoolslib
femsolverlib
fparser
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
${PETSc_LINK_LIBRARIES}
)

target_link_libraries(transport 
PRIVATE
meshtoolslib
femsolverlib
fparser
${ADDITIONAL_ITEMS}
${METIS_LIBRARIES}
${PETSc_LINK_LIBRARIES}
)
