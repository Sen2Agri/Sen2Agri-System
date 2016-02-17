otb_create_application(
  NAME           DataSmoothing
  SOURCES        DataSmoothing.cpp DataSmoothing.hxx
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_DataSmoothing DESTINATION usr/lib/otb/applications/)
