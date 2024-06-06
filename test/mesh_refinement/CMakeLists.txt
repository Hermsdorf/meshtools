add_executable(test_refine test_refine.cpp)
target_link_libraries(test_refine
PRIVATE
meshtoolslib
fparser
${MESHTOOLS_DEP_LIBS}
)