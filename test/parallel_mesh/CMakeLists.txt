
add_executable(pmesh_test test_p_mesh.cpp)

target_link_libraries(pmesh_test 
   PRIVATE
   meshtoolslib
   fparser
   ${MESHTOOLS_DEP_LIBS}
   )



