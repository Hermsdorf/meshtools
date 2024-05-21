
add_executable(poisson poisson_2d.cpp)

# add_executable(conv2d convection_diffusion_2d.cpp)
# add_executable(transport transport.cpp)
# add_executable(benchmark_rotation_pulse benchmark_rotation_pulse.cpp)
# add_executable(benchmark_rotation_coin benchmark_rotation_coin.cpp)
# add_executable(benchmark_disk_stretching benchmark_disk_stretching.cpp)
# add_executable(benchmark_sphere_stretching benchmark_sphere_stretching.cpp)
# add_executable(normal_vectors normal_vectors.cpp)
# add_executable(surface_integral surface_integral.cpp)
# add_executable(poisson3d poisson_3d.cpp)


target_link_libraries(poisson 
   PRIVATE
   meshtoolslib
   fparser
   ${MESHTOOLS_DEP_LIBS}
   )


# target_link_libraries(conv2d 
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(transport 
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(benchmark_rotation_pulse 
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(benchmark_rotation_coin
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(benchmark_disk_stretching
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(normal_vectors
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(surface_integral
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(benchmark_sphere_stretching
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

# target_link_libraries(poisson3d
# PRIVATE
# meshtoolslib
# fparser
# ${MESHTOOLS_DEP_LIBS}
# )

