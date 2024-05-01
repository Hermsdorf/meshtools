add_executable(test_io_1 test_io_1.cpp)
target_link_libraries(test_io_1
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)