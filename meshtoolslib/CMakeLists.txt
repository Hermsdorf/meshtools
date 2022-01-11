

file(GLOB source_files_lib
         "include/*.h"
         "src/*.cpp"
    )

add_library(meshtoolslib SHARED ${source_files_lib})

if(MPI_ENABLE)
   find_package(MPI)
   if(MPI_FOUND)
	   include_directories(meshtoolslib ${MPI_C_INCLUDE_PATH})
      set(USE_MPI 1)
   endif()
   
endif()

if(CATALYST_ENABLE)
   find_package(ParaView CONFIG COMPONENTS Catalyst PythonCatalyst)
   if (${ParaView_FOUND})
      MESSAGE(STATUS "Found ParaView Catalyst.")
      set(ADDITIONAL_ITEMS ParaView::Catalyst ParaView::PythonCatalyst VTK::CommonDataModel)
      set(USE_CATALYST 1)
   else (${ParaView_FOUND})
      list(FILTER source_files_lib EXCLUDE REGEX "include/FEAdaptor.h")
      list(FILTER source_files_lib EXCLUDE REGEX "src/FEAdaptor.cpp")
   endif (${ParaView_FOUND})
else()
   list(FILTER source_files_lib EXCLUDE REGEX "include/FEAdaptor.h")
   list(FILTER source_files_lib EXCLUDE REGEX "src/FEAdaptor.cpp")
endif()

configure_file(meshtools_config.h.in ${CMAKE_SOURCE_DIR}/meshtoolslib/include/meshtools_config.h @ONLY)





