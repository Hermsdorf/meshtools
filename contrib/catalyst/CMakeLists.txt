
if (CATALYST_LEGACY_ENABLE)
  add_library(CatalystAdaptor
    FEAdaptor.cxx
    FEAdaptor.h)
  target_link_libraries(CatalystAdaptor
    INTERFACE
      VTK::PythonUsed
    PRIVATE
      ParaView::PythonCatalyst
      VTK::CommonDataModel)
endif ()

