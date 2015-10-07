otb_create_application(
  NAME           CreateS2AnglesRaster
  SOURCES        CreateS2AnglesRaster.cpp
  LINK_LIBRARIES MACCSMetadata ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
