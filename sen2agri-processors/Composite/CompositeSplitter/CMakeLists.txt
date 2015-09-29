otb_create_application(
  NAME           CompositeSplitter
  SOURCES        CompositeSplitter.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
