

   add_executable(meshtools meshtools_app.cpp)
   add_executable(femtools femtools_app.cpp)


   if(PETSc_FOUND)
       include_directories(${PETSc_INCLUDE_DIRS})
   endif()


   if(MPI_ENABLE)
      find_package(MPI)
      if(MPI_FOUND)
         include_directories(meshtools ${MPI_C_INCLUDE_PATH})
         target_link_libraries(meshtools PUBLIC MPI::MPI_C MPI::MPI_CXX)

         include_directories(femtools ${MPI_C_INCLUDE_PATH})
         target_link_libraries(femtools PUBLIC MPI::MPI_C MPI::MPI_CXX)
      endif()
      set(USE_MPI 1)
   endif()

   if(CATALYST_ENABLE)
      find_package(ParaView CONFIG COMPONENTS Catalyst PythonCatalyst)
      if (${ParaView_FOUND})
         MESSAGE(STATUS "Found ParaView Catalyst.")
         set(ADDITIONAL_ITEMS ParaView::Catalyst ParaView::PythonCatalyst VTK::CommonDataModel)
         set(USE_CATALYST 1)
      else (${ParaView_FOUND})
         list(FILTER source_files EXCLUDE REGEX "include/FEAdaptor.h")
         list(FILTER source_files EXCLUDE REGEX "src/FEAdaptor.cpp")
      endif (${ParaView_FOUND})
   else()
      list(FILTER source_files EXCLUDE REGEX "include/FEAdaptor.h")
      list(FILTER source_files EXCLUDE REGEX "src/FEAdaptor.cpp")
   endif()

   

   if(OpenMP_ENABLE)
      find_package(OpenMP)
      if(OpenMP_CXX_FOUND)
         target_link_libraries(meshtools PUBLIC OpenMP::OpenMP_CXX)
         target_link_libraries(femtools PUBLIC OpenMP::OpenMP_CXX)
      endif()
   endif()

   target_link_libraries(meshtools 
   PRIVATE
   meshtoolslib
   ${ADDITIONAL_ITEMS}
   ${METIS_LIBRARIES}
   ${PETSc_LINK_LIBRARIES}
   )

   target_link_libraries(femtools 
   PRIVATE
   meshtoolslib
   ${ADDITIONAL_ITEMS}
   ${METIS_LIBRARIES}
   ${PETSc_LINK_LIBRARIES}
   )
