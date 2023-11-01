
add_executable(test_coloring test_coloring.cpp)
target_link_libraries(test_coloring 
   PRIVATE
   meshtoolslib
   fparser
   ${MESHTOOLS_DEP_LIBS}
   )
