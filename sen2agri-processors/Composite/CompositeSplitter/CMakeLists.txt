otb_create_application(
  NAME           CompositeSplitter
  SOURCES        CompositeSplitter.cpp
  LINK_LIBRARIES MACCSMetadata ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
