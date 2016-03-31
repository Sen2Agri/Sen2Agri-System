otb_create_application(
  NAME           SampleSelection
  SOURCES        SampleSelection.cpp
  LINK_LIBRARIES ${OTB_LIBRARIES})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

install(TARGETS otbapp_SampleSelection DESTINATION usr/lib/otb/applications/)
