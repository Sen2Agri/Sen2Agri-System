project(Composite)

otb_create_application(
  NAME           ComputeNDVI
  SOURCES        ComputeNDVI.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
