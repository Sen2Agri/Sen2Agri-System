otb_create_application(
  NAME           WeightAOT
  SOURCES        weightonaot.cpp weightaotapp.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
