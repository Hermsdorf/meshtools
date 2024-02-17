
get_property(CATALYST_ENABLE GLOBAL PROPERTY CATALYST_ENABLE)
if(CATALYST_ENABLE)
  add_executable(benchmark_catalyst test_catalyst.cpp)
  add_executable(test_simple test_catalyst_basic.cpp)

  target_link_libraries(benchmark_catalyst
  PRIVATE
  meshtoolslib
  fparser
  ${MESHTOOLS_DEP_LIBS}
  )

  target_link_libraries(test_simple
  PRIVATE
  meshtoolslib
  fparser
  ${MESHTOOLS_DEP_LIBS}
  )
endif()