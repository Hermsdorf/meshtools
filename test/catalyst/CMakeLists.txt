
add_executable(benchmark_catalyst_disk test_catalyst_disk.cpp)
add_executable(benchmark_catalyst_sphere test_catalyst_sphere.cpp)
add_executable(test_catalyst_simple test_catalyst_basic.cpp)

target_link_libraries(benchmark_catalyst_disk
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)

target_link_libraries(benchmark_catalyst_sphere
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)

target_link_libraries(test_catalyst_simple
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)
