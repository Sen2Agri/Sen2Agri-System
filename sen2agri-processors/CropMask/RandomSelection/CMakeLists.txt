otb_create_application(
  NAME           RandomSelection
  SOURCES        RandomSelection.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
