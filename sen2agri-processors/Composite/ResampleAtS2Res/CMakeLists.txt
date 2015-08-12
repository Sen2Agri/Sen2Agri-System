otb_create_application(
  NAME           ResampleAtS2Res
  SOURCES        ResampleAtS2Res.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES} MACCSMetadata)

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
