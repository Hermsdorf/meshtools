add_subdirectory(parallel_mesh)
add_subdirectory(finite_element)

set(TEST_MESH_DIR "${CMAKE_SOURCE_DIR}/test/finite_element/msh/")
configure_file(test_config.h.in ${CMAKE_SOURCE_DIR}/include/test_config.h @ONLY)