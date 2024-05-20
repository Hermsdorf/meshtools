add_executable(test_io_tri3 test_io_tri3.cpp)
target_link_libraries(test_io_tri3
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)

add_executable(test_io_quad4 test_io_quad4.cpp)
target_link_libraries(test_io_quad4
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)