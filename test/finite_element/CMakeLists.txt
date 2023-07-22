
add_executable(poisson poisson_2d.cpp)
add_executable(conv2d convection_diffusion_2d.cpp)
add_executable(transport transport.cpp)
add_executable(benchmark_rotation_pulse benchmark_rotation_pulse.cpp)
add_executable(benchmark_rotation_coin benchmark_rotation_coin.cpp)
add_executable(benchmark_disk_stretching benchmark_disk_stretching.cpp)
add_executable(benchmark_sphere_stretching benchmark_sphere_stretching.cpp)
add_executable(normal_vectors normal_vectors.cpp)
add_executable(surface_integral surface_integral.cpp)
add_executable(poisson3d poisson_3d.cpp)


include_directories(poisson   ${MESHTOOLS_INCLUDE_DIR})
include_directories(conv2d    ${MESHTOOLS_INCLUDE_DIR})
include_directories(transport ${MESHTOOLS_INCLUDE_DIR})
include_directories(benchmark_rotation_pulse ${MESHTOOLS_INCLUDE_DIR})
include_directories(benchmark_disk_stretching ${MESHTOOLS_INCLUDE_DIR})
include_directories(benchmark_sphere_stretching ${MESHTOOLS_INCLUDE_DIR})
include_directories(normal_vectors ${MESHTOOLS_INCLUDE_DIR})
include_directories(surface_integral ${MESHTOOLS_INCLUDE_DIR})
include_directories(poisson3d ${MESHTOOLS_INCLUDE_DIR})

# if(MPI_FOUND)
# include_directories(poisson ${MPI_C_INCLUDE_PATH})
# target_link_libraries(poisson PUBLIC MPI::MPI_C MPI::MPI_CXX)

# include_directories(conv2d ${MPI_C_INCLUDE_PATH})
# target_link_libraries(conv2d PUBLIC MPI::MPI_C MPI::MPI_CXX)

# include_directories(transport ${MPI_C_INCLUDE_PATH})
# target_link_libraries(transport PUBLIC MPI::MPI_C MPI::MPI_CXX)

# endif()

target_link_libraries(poisson 
   PRIVATE
   meshtoolslib
   femsolverlib
   fparser
   ${MESHTOOLS_LIBS}
   )


target_link_libraries(conv2d 
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(transport 
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(benchmark_rotation_pulse 
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(benchmark_rotation_coin
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(benchmark_disk_stretching
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(normal_vectors
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(surface_integral
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(benchmark_sphere_stretching
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

target_link_libraries(poisson3d
PRIVATE
meshtoolslib
femsolverlib
fparser
${MESHTOOLS_LIBS}
)

