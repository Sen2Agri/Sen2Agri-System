otb_create_application(
  NAME           CompositeSplitter2
  SOURCES        CompositeSplitter2.cpp
  LINK_LIBRARIES MACCSMetadata MetadataHelper ${OTB_LIBRARIES})
  
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
